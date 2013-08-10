#pragma once

#include <point3.h>

/* Represents a grid mesh that can be deformed. */
class Grid
{
    int _widthSegs;
    int _lengthSegs;
    Point3* _vertices;

public:
    /*
    Creates a new Grid with the vertices uninitialized.
    \param widthSegs the number of faces along the U-axis
    \param lengthSegs the number of faces along the V-axis
    */
    Grid(int widthSegs, int lengthSegs);
    ~Grid(void);

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
    Gets the array of vertices; this is a mapping between the UV space and the XYZ space.
    The length of the array is equal to GetWidthVertices() * GetLengthVertices().
    The vertices are arranged U-major; e.g. UV coords (0, 0.1), (0, 0.2), (0, 0.3), ..., (0.1, 0.1), (0.1, 0.2), (0.1, 0.3), ..., (1.0, 1.0).
    */
    Point3* GetVertices();

    /*
    Replaces the vertices with a mapping of the UV plane to the XY plane.
    \param width the width of the grid in the X-axis
    \param length the length of the grid in the Y-axis
    */
    void MakePlanar(float width, float length);

    /*
    Redimensions the grid.
    \param widthSegs the new number of faces along the U-axis
    \param lengthSegs the new number of faces along the V-axis
    */
    void Redim(int widthSegs, int lengthSegs);
};

