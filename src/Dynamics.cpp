#define _USE_MATH_DEFINES 

#include "Dynamics.h"
#include <cmath>
#include <triobj.h>
#include <units.h>
#include "GaussianConvolution.h"
#include "VerticalDerivativeConvolution.h"
#include "Ambient.h"

void Clear(float* arr, int size, float val)
{
    for (int i = 0; i < size; i++)
    {
        arr[i] = val;
    }
}

TriObject* GetTriObjectFromNode(INode *node, int &deleteIt, TimeValue t)
{
    deleteIt = FALSE;
    Object *obj = node->EvalWorldState(t).obj;

    if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
    {
        TriObject *tri = (TriObject *) obj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID, 0));
        // Note that the TriObject should only be deleted
        // if the pointer to it is not equal to the object
        // pointer that called ConvertToType()
        if (obj != tri) deleteIt = TRUE;
        return tri;
    }
    else
    {
        return NULL;
    }
}

Dynamics::Dynamics(int startFrame, float width, float length, int widthSegs, int lengthSegs, float heightScale, float dt, float alpha, float sigma, float wakePower, INode* parentNode, INode** collisionNodes, int numCollisionNodes, Grid* ambient)
    : frame_num(startFrame), vertices_x(widthSegs + 1), vertices_y(lengthSegs + 1), vertices_total((widthSegs + 1) * (lengthSegs + 1)), width(width), length(length), height_scale(heightScale),
    dt(dt), alpha(alpha), gravity(9.8 * dt * dt), sigma(sigma), wake_exp(wakePower),
    parent_node(parentNode), collision_nodes(collisionNodes), collision_nodes_count(numCollisionNodes),
    ambient(ambient)
{
    assert(parentNode != NULL);
    assert(collisionNodes != NULL);

    gaussianConvolution = new GaussianConvolution(sigma);
    verticalDerivConvolution = new VerticalDerivativeConvolution<P>();

    obstruction_raw = new float[vertices_total];
    obstruction = new float[vertices_total];
    source = new float[vertices_total];
    height = new float[vertices_total];
    previous_height = new float[vertices_total];
    vertical_derivative = new float[vertices_total];

    Clear(obstruction, vertices_total, 1.0);
    Clear(source, vertices_total, 0.0);
    Clear(height, vertices_total, 0.0);
    Clear(previous_height, vertices_total, 0.0);
    Clear(vertical_derivative, vertices_total, 0.0);
}

Dynamics::~Dynamics(void)
{
    delete gaussianConvolution;
    delete verticalDerivConvolution;

    delete [] obstruction_raw;
    delete [] obstruction;
    delete [] source;
    delete [] height;
    delete [] previous_height;
    delete [] vertical_derivative;
}

void Dynamics::UpdateObstructions()
{
    TimeValue t = frame_num * GetTicksPerFrame();

    Clear(obstruction_raw, vertices_total, 1.0);

    float halfWidth = width / 2.0;
    float halfLength = length / 2.0;
    int faces_x = vertices_x - 1;
    int faces_y = vertices_y - 1;

    // Calculate obstructions first.
    Matrix3 objToWorld = parent_node->GetObjectTM(t);
    Point3 dirUp = Point3(0.0, 0.0, 1.0);
    Point3 dirDown = Point3(0.0, 0.0, -1.0);

    // Loop through nodes.
    for (int k = 0; k < collision_nodes_count; k++)
    {
        INode* currNode = collision_nodes[k];

        Matrix3 worldToObj = currNode->GetObjectTM(t);
        worldToObj.Invert();
        Matrix3 objToObj = objToWorld * worldToObj;

        int deleteIt;
        TriObject *triObject = GetTriObjectFromNode(currNode, deleteIt, t);

        // Use the TriObject if available.
        if (!triObject) continue; // Go to next node.

        Box3 boundBox;
        triObject->GetDeformBBox(t, boundBox);

        // Loop through points.
        int vtx = 0;
        for (int i = 0; i < vertices_x; i++)
        {
            for (int j = 0; j < vertices_y; j++)
            {
                if (obstruction_raw[vtx] != 0.0) // Only continue if point is not a known obstruction.
                {
                    Point3 pt(i * width / faces_x - halfWidth, j * length / faces_y - halfLength, 0.0f);
                    Point3 ptInObjSpace = objToObj.PointTransform(pt);

                    if (boundBox.Contains(ptInObjSpace))
                    {
                        // Need to convert rays from our coords to the GeomObject's coords.
                        Ray upHigh;
                        upHigh.p = objToObj.PointTransform(pt);
                        upHigh.dir = objToObj.PointTransform(dirUp);

                        Point3 norm;
                        float atUp;
                        bool intersectsUpHigh = triObject->IntersectRay(t, upHigh, atUp, norm) != 0;

                        if (!intersectsUpHigh)
                        {
                            atUp = boundBox.Width().FLength(); // Estimate.
                        }

                        Ray downLow;
                        downLow.p = upHigh.p + atUp * upHigh.dir;
                        downLow.dir = -upHigh.dir;

                        float atDown;
                        bool intersectsDownLow = triObject->IntersectRay(t, downLow, atDown, norm) != 0;

                        if (intersectsDownLow && atDown < atUp)
                        {
                            obstruction_raw[vtx] = 0.0;
                        }
                    }
                }

                vtx++;
            }
        }

        // Delete it when done...
        if (deleteIt) triObject->DeleteMe();
    }

    // Perform Gaussian smoothing of obstruction boundaries.
    gaussianConvolution->Convolve(obstruction_raw, obstruction, vertices_x, vertices_y);

    // Calculate sources.
    for (int i = 0; i < vertices_total; i++)
    {
        // Make all collision objects create wakes (act as sources).
        // Paper suggests source = 1 - obstruction, but source = 1 - obstruction^2 gives slightly stronger waves.
        // Thus, wake_exp=1.0 is the natural/minimum, wake_exp=2.0+ gives slightly exaggerated wakes.
        source[i] = 1.0 - pow(obstruction[i], wake_exp);
    }
}

void Dynamics::PropagateWaves()
{
    // Apply obstruction; prevents waves from crossing obstructions.
    for (int i = 0; i < vertices_total; i++)
    {
        height[i] *= obstruction[i];
    }

    verticalDerivConvolution->Convolve(height, vertical_derivative, vertices_x, vertices_y);
    
    float* ambientHeights = ambient == NULL ? NULL : ambient->GetVertexHeights();

    // Actually move the surface waves now!
    float alpha_dt_1 = 2.0 - (alpha * dt);
    float alpha_dt_2 = 1.0 / (1.0 + alpha * dt);
    for (int i = 0; i < vertices_total; i++) {
        float temp = height[i]; // Save previous position before modifying
        height[i] = height[i] * alpha_dt_1 - previous_height[i] - gravity * vertical_derivative[i];
        height[i] *= alpha_dt_2;
        height[i] += source[i];
        height[i] *= obstruction[i];

        if (ambientHeights != NULL)
        {
            height[i] -= ambientHeights[i] * (1.0 - obstruction[i]);
        }

        previous_height[i] = temp;
    }
}

Grid* Dynamics::GetDisplayGrid()
{
    Grid* ret = new Grid(width, length, vertices_x - 1, vertices_y - 1);
    float* vertexHeights = ret->GetVertexHeights();

    float* ambientHeights = ambient == NULL ? NULL : ambient->GetVertexHeights();

    int vtx = 0;
    for (int i = 0; i < vertices_x; i++)
    {
        for (int j = 0; j < vertices_y; j++)
        {
            vertexHeights[vtx] = (height[vtx] * height_scale);
            
            if (ambientHeights != NULL)
            {
                vertexHeights[vtx] += ambientHeights[vtx];
            }

            vtx++;
        }
    }

    return ret;
}

Grid* Dynamics::NextGrid() {
    UpdateObstructions();
    PropagateWaves();
    Grid* render = GetDisplayGrid();
    frame_num++;
    return render;
}
