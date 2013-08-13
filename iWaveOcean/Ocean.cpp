#define _USE_MATH_DEFINES 

#include "Ocean.h"
#include <math.h>
#include <triobj.h>

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

void GetGaussianKernel(float sigma, float(& arr)[5][5])
{
    float double_sigma_2 = 2.0 * sigma * sigma;
    float sum = 0.0f;

    for (int i = -2; i <= 2; i++)
    {
        int i_2 = i * i;
        for (int j = -2; j <= 2; j++)
        {
            int j_2 = j * j;
            float val = exp(-(i_2 + j_2) / double_sigma_2);
            arr[i + 2][j + 2] = val;
            sum += val;
        }
    }

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            arr[i][j] /= sum;
        }
    }
}

void GetVerticalDerivKernel(float(& arr)[2 * P + 1][2 * P + 1])
{
    float dq = 0.001;
    float sigma = 1.0;

    double G_0 = 0.0; // Norm value.
    for (float q = 0.0; q < 10.0; q += dq)
    {
        // From the paper, we want dq = .001 and calculate q_n for 1 <= n <= 10000.
        // This gives us 0 <= q < 10.
        G_0 += q * q * exp(-sigma * q * q);
    }

    for (int i = -P; i <= P; i++)
    {
        for (int j = -P; j <= P; j++)
        {
            float r = sqrt((float)(i * i + j * j));
            float kern = 0.0;

            for (float q = 0.0; q < 10.0; q += dq)
            {
                kern += q * q * exp(-sigma * q * q) * j0(r * q);
            }

            arr[i + P][j + P] = kern / G_0;
        }
    }
}

Ocean::Ocean(int verticesX, int verticesY, float width, float length, float heightScale, float dt, float alpha, float sigma, INode* parentNode, INode** collisionNodes, int numCollisionNodes)
    : vertices_x(verticesX), vertices_y(verticesY), vertices_total(verticesX * verticesY), width(width), length(length), height_scale(heightScale),
    dt(dt), alpha(alpha), gravity(9.8 * dt * dt), sigma(sigma),
    parent_node(parentNode), collision_nodes(collisionNodes), collision_nodes_count(numCollisionNodes)
{
    float gaussianKernel[5][5];
    GetGaussianKernel(sigma, gaussianKernel);
    gaussianConvolution = new Convolution<2, ExtendEdges>(gaussianKernel);

    float verticalDerivKernel[2 * P + 1][2 * P + 1];
    GetVerticalDerivKernel(verticalDerivKernel);
    verticalDerivConvolution = new Convolution<P, ExtendEdges>(verticalDerivKernel);

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

Ocean::~Ocean(void)
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

void Ocean::PropagateWaves()
{
    // Apply obstruction; prevents waves from crossing obstructions.
    for (int i = 0; i < vertices_total; i++)
    {
        height[i] *= obstruction[i];
    }

    verticalDerivConvolution->Convolve(height, vertical_derivative, vertices_x, vertices_y);

    // Actually move the surface waves now!
    float alpha_dt_1 = 2.0 - (alpha * dt);
    float alpha_dt_2 = 1.0 / (1.0 + alpha * dt);
    for (int i = 0; i < vertices_total; i++) {
        float temp = height[i]; // Save previous position before modifying
        height[i] = height[i] * alpha_dt_1 - previous_height[i] - gravity * vertical_derivative[i];
        height[i] *= alpha_dt_2;
        height[i] += source[i];
        height[i] *= obstruction[i];
        previous_height[i] = temp;
    }
}

Grid* Ocean::GetDisplayGrid()
{
    Grid* ret = new Grid(width, length, vertices_x - 1, vertices_y - 1);
    float* vertexHeights = ret->GetVertexHeights();

    int vtx = 0;
    for (int i = 0; i < vertices_x; i++)
    {
        for (int j = 0; j < vertices_y; j++)
        {
            float vtx_height = 0.5 * (height[vtx] * height_scale + 1.0) * obstruction[vtx];
            vertexHeights[vtx] = vtx_height;
            vtx++;
        }
    }

    return ret;
}

void Ocean::UpdateObstructions(TimeValue t)
{
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

    gaussianConvolution->Convolve(obstruction_raw, obstruction, vertices_x, vertices_y);

    // Calculate sources.
    for (int i = 0; i < vertices_total; i++)
    {
        // Make all collision objects create wakes (act as sources).
        // Paper suggests source = 1 - obstruction, but source = 1 - obstruction^2 gives slightly stronger waves.
        source[i] = 1.0 - (obstruction[i] * obstruction[i]);
    }
}

Grid* Ocean::NextGrid() {
    PropagateWaves();
    return GetDisplayGrid();
}
