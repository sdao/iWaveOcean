#pragma once
#include "IConvolution.h"
#include "Convolution.h"

template <int radius>
class VerticalDerivativeConvolution : public IConvolution<radius>
{
    float _kernel[2 * radius + 1][2 * radius + 1];
    IConvolution<radius>* _convolution;

    static void GetVerticalDerivKernel(float(& arr)[2 * radius + 1][2 * radius + 1])
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

        for (int i = -radius; i <= radius; i++)
        {
            for (int j = -radius; j <= radius; j++)
            {
                float r = sqrt((float)(i * i + j * j));
                float kern = 0.0;

                for (float q = 0.0; q < 10.0; q += dq)
                {
                    kern += q * q * exp(-sigma * q * q) * j0(r * q);
                }

                arr[i + radius][j + radius] = kern / G_0;
            }
        }
    }

public:
    VerticalDerivativeConvolution(void)
    {
        GetVerticalDerivKernel(_kernel);
        _convolution = new Convolution<radius, ReflectEdges>(_kernel);
    }

    void Convolve(float* in, float* out, int rows, int cols) const
    {
        _convolution->Convolve(in, out, rows, cols);
    }

    ~VerticalDerivativeConvolution(void)
    {
        delete _convolution;
    }
};

