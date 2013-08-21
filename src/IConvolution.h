#pragma once

/** Interface for a class that performs an arbitrary convolution on an input matrix. */
template<int radius>
class IConvolution
{
public:
    virtual ~IConvolution() {}

    /**
    Performs a convolution on an input matrix and stores it in an output matrix.
    \param in the input matrix
    \param out the output matrix (must be same dimensions as the input matrix)
    \param rows the number of rows in each matrix
    \param cols the number of columns in each matrix
    */
    virtual void Convolve(float* in, float* out, int rows, int cols) const = 0;
};

