#pragma once
#include "IConvolution.h"

/** @file */
/** The behavior of the convolution kernel when it requires values that extend off the edges of the input. */
enum ConvolutionEdgeBehavior
{
    /** Use the closest values on the input matrix's edge. */
    ExtendEdges,
    /** Use the values on the opposite edge. */
    WrapEdges,
    /** Reflect the index requested across the edge boundary. */
    ReflectEdges
};

/**
Convolution of a specified kernel onto an input matrix, with customizable edge behavior.
\tparam radius the radius of the kernel; the kernel will be of size (2P+1) by (2P+1)
\tparam behavior a ::ConvolutionEdgeBehavior that specifies how the kernel handles boundary conditions
*/
template <int radius, ConvolutionEdgeBehavior behavior>
class Convolution : public IConvolution<radius>
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

    void Convolve(float* in, float* out, int rows, int cols) const
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
                        else if (behavior == ReflectEdges)
                        {
                            // Flip negative values (i.e. -x => x).
                            other_vtx_x = abs(i + kern_x);
                            other_vtx_y = abs(j + kern_y);
                            
                            // Flip all values over the boundary (i.e. BOUND + x => BOUND - x).
                            int diff_x = abs(other_vtx_x - (rows - 1));
                            other_vtx_x = (rows - 1) - diff_x;

                            int diff_y = abs(other_vtx_y - (cols - 1));
                            other_vtx_y = (cols - 1) - diff_y;
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

