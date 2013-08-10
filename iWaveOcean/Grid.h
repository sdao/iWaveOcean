#pragma once

/* Represents a grid mesh that can be deformed. */
class Grid
{
    float _width;
    float _length;
    int _widthSegs;
    int _lengthSegs;
    float* _vertices;

public:
    /*
    Creates a new Grid with the vertices uninitialized. Use the Clear() method to zero out all of the vertex heights.
    \param width the width along the U-axis
    \param length the length along the V-axis
    \param widthSegs the number of faces along the U-axis
    \param lengthSegs the number of faces along the V-axis
    */
    Grid(float width, float length, int widthSegs, int lengthSegs);
    ~Grid(void);

    /* Gets the display width. */
    float GetWidth();

    /* Gets the display height. */
    float GetLength();

    /* Gets the number of faces along the U-axis. */
    int GetWidthSegs();

    /* Gets the number of faces along the V-axis. */
    int GetLengthSegs();

    /* Gets the number of vertices along the U-axis. */
    int GetWidthVertices();

    /* Gets the number of vertices along the V-axis. */
    int GetLengthVertices();

    /* Gets the total number of vertices. */
    int GetTotalVertices();

    /*
    Gets the array of heights corresponding to each UV coordinate on the grid.
    The vertices are arranged U-major; e.g. UV coords (0, 0.1), (0, 0.2), (0, 0.3), ..., (0.1, 0.1), (0.1, 0.2), (0.1, 0.3), ..., (1.0, 1.0).
    */
    float* GetVertexHeights();

    /*
    Zeroes out all of the values on the height map.
    */
    void Clear();

    /*
    Redimensions the grid.
    \param width the width along the U-axis
    \param length the length along the V-axis
    \param widthSegs the new number of faces along the U-axis
    \param lengthSegs the new number of faces along the V-axis
    */
    void Redim(float width, float length, int widthSegs, int lengthSegs);
};

