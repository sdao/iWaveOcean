#pragma once
#include "Grid.h"
#include <object.h>
#include "IConvolution.h"

#define P 6

class Ocean
{
    INode* parent_node;
    INode** collision_nodes;
    int collision_nodes_count;

    float dt;       /* Time between each frame of the simulation, i.e. 24 fps => dt = 0.4 */
    float alpha;    /* Wave damping factor. */
    float gravity;  /* 9.8 m/s^2 * dt * dt */
    float sigma;    /* Used for strength of Gaussian smoothing of obstructions. */
    int wave_exp;

    int vertices_x;
    int vertices_y;
    int vertices_total;

    float width;
    float length;
    float height_scale;

    IConvolution<2>* gaussianConvolution;
    IConvolution<P>* verticalDerivConvolution;

    float *obstruction_raw;         /* Water obstruction(s) before Gaussian smoothing. */
    float *obstruction;             /* Water obstruction(s). 1.0 = no obstruction, 0.0 = total obstruction. */
    float *source;                  /* Water source(s). 0.0 = no source. */
    float *height;                  /* Height map of waves. 0.0 = no height. */
    float *previous_height;         /* Height map of waves at previous time. */
    float *vertical_derivative;     /* Used for calculating convolution. */

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
    Ocean(int verticesX, int verticesY, float width, float length, float heightScale, float dt, float alpha, float sigma, int wavePower, INode* parentNode, INode** collisionNodes, int numCollisionNodes);
    ~Ocean(void);

    /* Updates the obstructions and sources for the current simulation time. */
    void UpdateObstructions(TimeValue t);

    /* Advances the simulation one step and returns the resultant grid. The Grid returned needs to be destroyed once it is unused. */
    Grid* NextGrid();
};

