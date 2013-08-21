#include "Grid.h"


Grid::Grid(float width, float length, int widthSegs, int lengthSegs)
    : _width(width), _length(length), _widthSegs(widthSegs), _lengthSegs(lengthSegs), _widthVtex(widthSegs + 1), _lengthVtex(lengthSegs + 1)
{
    _vertices = new float[(_widthSegs + 1) * (_lengthSegs + 1)];
}

Grid::Grid(float width, float length, int widthSegs, int lengthSegs, float* vertexHeights)
    : _width(width), _length(length), _widthSegs(widthSegs), _lengthSegs(lengthSegs), _widthVtex(widthSegs + 1), _lengthVtex(lengthSegs + 1)
{
    _vertices = vertexHeights;
}

Grid::~Grid(void)
{
    delete [] _vertices;
}

float Grid::GetWidth() const
{
    return _width;
}

float Grid::GetLength() const
{
    return _length;
}

int Grid::GetWidthSegs() const
{
    return _widthSegs;
}

int Grid::GetLengthSegs() const
{
    return _lengthSegs;
}

int Grid::GetWidthVertices() const
{
    return _widthVtex;
}

int Grid::GetLengthVertices() const
{
    return _lengthVtex;
}

int Grid::GetTotalVertices() const
{
    return _widthVtex * _lengthVtex;
}

float* Grid::GetVertexHeights() const
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
