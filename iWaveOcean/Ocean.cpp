#include "Ocean.h"
#include <math.h>

void Clear(float* arr, int size, float val)
{
    for (int i = 0; i < size; i++)
    {
        arr[i] = val;
    }
}

Ocean::Ocean(int verticesX, int verticesY, float width, float length, float heightScale, float dt, float alpha)
    : vertices_x(verticesX), vertices_y(verticesY), width(width), length(length), height_scale(heightScale),
    dt(dt), alpha(alpha), gravity(9.8 * dt * dt)
{
    int numVertices = verticesX * verticesY;

    obstruction = new float[numVertices];
    source = new float[numVertices];
    height = new float[numVertices];
    previous_height = new float[numVertices];
    vertical_derivative = new float[numVertices];

    Clear(obstruction, numVertices, 1.0);
    Clear(source, numVertices, 0.0);
    Clear(height, numVertices, 0.0);
    Clear(previous_height, numVertices, 0.0);
    Clear(vertical_derivative, numVertices, 0.0);

    source[numVertices / 2] = 1.0;

    InitializeKernel();
}

Ocean::~Ocean(void)
{
    delete [] obstruction;
    delete [] source;
    delete [] height;
    delete [] previous_height;
    delete [] vertical_derivative;
}

void Ocean::InitializeKernel()
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

            kernel[i + P][j + P] = kern / G_0;
        }
    }
}

void Ocean::Convolve()
{
    // We're skipping calculating anything within a margin of P.
    for (int i = P; i < vertices_x - P; i++) {
        for (int j = P; j < vertices_y - P; j++) {
            int vtx = i * vertices_y + j;
            float vd = 0.0f; // Local derivative at a point.

            for (int kern_x = -P; kern_x <= P; kern_x++) {
                for (int kern_y = -P; kern_y <= P; kern_y++) {
                    int other_vtx = (i + kern_x) * vertices_y + (j + kern_y);
                    vd += kernel[kern_x + P][kern_y + P] * height[other_vtx];
                }
            }

            vertical_derivative[vtx] = vd;
        }
    }
}

void Ocean::PropagateWaves()
{
    // Apply obstruction; prevents waves from crossing obstructions.
    int numVertices = vertices_x * vertices_y;
    for (int i = 0; i < numVertices; i++)
    {
        height[i] *= obstruction[i];
    }

    Convolve();

    // Actually move the surface waves now!
    float alpha_dt_1 = 2.0 - (alpha * dt);
    float alpha_dt_2 = 1.0 / (1.0 + alpha * dt);
    for (int i = 0; i < numVertices; i++) {
        float temp = height[i]; // Save previous position before modifying
        height[i] = height[i] * alpha_dt_1 - previous_height[i] - gravity * vertical_derivative[i];
        height[i] *= alpha_dt_2;
        height[i] += source[i];
        height[i] *= obstruction[i];
        previous_height[i] = temp;

        // TODO: figure out source values, i.e. source should "ripple" between 0.0...1.0.
        source[i] = 0;
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

Grid* Ocean::NextGrid() {
    PropagateWaves();
    return GetDisplayGrid();
}
