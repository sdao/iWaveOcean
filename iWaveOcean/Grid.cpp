#include "Grid.h"


Grid::Grid(float width, float length, int widthSegs, int lengthSegs)
    : _width(width), _length(length), _widthSegs(widthSegs), _lengthSegs(lengthSegs)
{
    _vertices = new float[(_widthSegs + 1) * (_lengthSegs + 1)];
}

Grid::~Grid(void)
{
    delete [] _vertices;
}

float Grid::GetWidth()
{
    return _width;
}

float Grid::GetLength()
{
    return _length;
}

int Grid::GetWidthSegs()
{
    return _widthSegs;
}

int Grid::GetLengthSegs()
{
    return _lengthSegs;
}

int Grid::GetWidthVertices()
{
    return _widthSegs + 1;
}

int Grid::GetLengthVertices()
{
    return _lengthSegs + 1;
}

int Grid::GetTotalVertices()
{
    return (_widthSegs + 1) * (_lengthSegs + 1);
}

float* Grid::GetVertexHeights()
{
    return _vertices;
}

void Grid::Clear()
{
    int vtx = 0;
    for (int i = 0; i <= _widthSegs; i++)
    {
        for (int j = 0; j <= _lengthSegs; j++)
        {
            _vertices[vtx] = 0.0;
            vtx++;
        }
    }
}

void Grid::Redim(float width, float length, int widthSegs, int lengthSegs)
{
    delete [] _vertices;

    _width = width;
    _length = length;
    _widthSegs = widthSegs;
    _lengthSegs = lengthSegs;

    _vertices = new float[(_widthSegs + 1) * (_lengthSegs + 1)];
}