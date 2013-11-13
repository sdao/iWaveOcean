#pragma once
#include "IConvolution.h"
#include "Convolution.h"

class GaussianConvolution :
    public IConvolution<2>
{
    float _kernel[5][5];
    IConvolution<2>* _convolution;

    static void GetGaussianKernel(float sigma, float(& arr)[5][5])
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

public:
    GaussianConvolution(float sigma) {
        GetGaussianKernel(sigma, _kernel);
        _convolution = new Convolution<2, ExtendEdges>(_kernel);
    }

    void Convolve(float* in, float* out, int rows, int cols) const {
        _convolution->Convolve(in, out, rows, cols);
    }

    ~GaussianConvolution(void) {
        delete _convolution;
    }
};

