#pragma once

enum ConvolutionEdgeBehavior
{
    ExtendEdges,
    WrapEdges,
    ReflectEdges
};

template <int radius, ConvolutionEdgeBehavior behavior>
class Convolution
{
    static const int kernel_width = 2 * radius + 1;
    float _kernel[kernel_width][kernel_width];

public:
    Convolution(float kernel[kernel_width][kernel_width])
    {
        for (int i = 0; i < kernel_width; i++)
        {
            for (int j = 0; j < kernel_width; j++)
            {
                _kernel[i][j] = kernel[i][j];
            }
        }
    }

    ~Convolution(void)
    {
    }

    void Convolve(float* in, float* out, int rows, int cols)
    {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int vtx = i * cols + j;
                float val = 0.0f; // Output value of kernel at specific point.

                for (int kern_x = -radius; kern_x <= radius; kern_x++) {
                    for (int kern_y = -radius; kern_y <= radius; kern_y++) {
                        int other_vtx_x = 0;
                        int other_vtx_y = 0;
                        if (behavior == ExtendEdges)
                        {
                            other_vtx_x = min(max(i + kern_x, 0), rows - 1);
                            other_vtx_y = min(max(j + kern_y, 0), cols - 1);
                        }
                        else if (behavior == WrapEdges)
                        {
                            other_vtx_x = (((i + kern_x) % rows) + rows) % rows;
                            other_vtx_y = (((j + kern_y) % rows) + rows) % rows;
                        }

                        int other_vtx = other_vtx_x * cols + other_vtx_y;
                        val += _kernel[kern_x + radius][kern_y + radius] * in[other_vtx];
                    }
                }

                out[vtx] = val;
            }
        }
    }
};

