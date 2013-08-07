#include "Grid.h"


Grid::Grid(int widthSegs, int lengthSegs)
{
    _widthSegs = widthSegs;
    _lengthSegs = lengthSegs;

    _vertices = new Point3[(_widthSegs + 1) * (_lengthSegs + 1)];
}

Grid::~Grid(void)
{
    delete [] _vertices;
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

Point3* Grid::GetVertices()
{
    return _vertices;
}

void Grid::MakePlanar(float width, float length)
{
    float halfWidth = width / 2.0f;
    float halfLength = length / 2.0f;

    int vtx = 0;
    for (int i = 0; i <= _widthSegs; i++)
    {
        for (int j = 0; j <= _lengthSegs; j++)
        {
            _vertices[vtx] = Point3(i * width / _widthSegs - halfWidth, j * length / _lengthSegs - halfLength, 0.0f);
            vtx++;
        }
    }
}

void Grid::Redim(int widthSegs, int lengthSegs)
{
    delete [] _vertices;

    _widthSegs = widthSegs;
    _lengthSegs = lengthSegs;

    _vertices = new Point3[(_widthSegs + 1) * (_lengthSegs + 1)];
}