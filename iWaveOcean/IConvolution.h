#pragma once

template<int radius>
class IConvolution
{
public:
    virtual void Convolve(float* in, float* out, int rows, int cols) = 0;
};

