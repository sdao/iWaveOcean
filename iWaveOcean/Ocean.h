#pragma once
#include "Grid.h"

#define P 6

class Ocean
{
    float dt;       /* Time between each frame of the simulation, i.e. 24 fps => dt = 0.4 */
    float alpha;    /* Wave damping factor. */
    float gravity;  /* 9.8 m/s^2 * dt * dt */

    int vertices_x;
    int vertices_y;

    float width;
    float length;
    float height_scale;

    float *obstruction;             /* Water obstruction(s). 1.0 = no obstruction, 0.0 = total obstruction. */
    float *source;                  /* Water source(s). 0.0 = no source. */
    float *height;                  /* Height map of waves. 0.0 = no height. */
    float *previous_height;         /* Height map of waves at previous time. */
    float *vertical_derivative;     /* Used for calculating convolution. */

    /* Convolution kernel. Must have dimensions (2P+1) * (2P+1); recommended P = 6, therefore 13 * 13. */
    float kernel[2*P+1][2*P+1];
    
    /* Initializes the values in the convolution kernel. Only needs to be called once upon initialization. */
    void InitializeKernel();

    /* Helper function for PropagateWaves(). DO NOT CALL DIRECTLY. */
    void Convolve();

    /* Propagates the waves in the simulation one step (one dt). */
    void PropagateWaves();

    /* Gets the Grid representation of the ocean waves at the current simulation time. */
    Grid* GetDisplayGrid();
public:

    /*
    Constructs a new Ocean object.
    \param verticesX the number of vertices along the width; must be >= 2P+1; otherwise failure will occur while simulating
    \param verticesY the number of vertices along the length; must be >= 2P+1
    \param width the width of the plane
    \param length the length of the plane
    \param heightScale a factor that scales the height of the waves
    \param dt the difference in time between frames (e.g. for 24 fps, a normal dt is 1/24)
    \param alpha the wave damping factor
    */
    Ocean(int verticesX, int verticesY, float width, float length, float heightScale, float dt, float alpha);
    ~Ocean(void);

    /* Advances the simulation one step and returns the resultant grid. The Grid returned needs to be destroyed once it is unused. */
    Grid* NextGrid();
};

