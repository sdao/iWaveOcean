#include <complex>
#include <random>
#include <point3.h>
#include <iparamb2.h>
#include "Grid.h"

typedef std::complex<float> complex;

/**
A class that simulates ocean waves at a given time using Tessendorf's wave equations and the FFT method.
The equations referenced by the documentation comments are those in "Simulating Ocean Waves", (c) 1999-2001 Jerry Tessendorf (SIGGRAPH Course Notes 2002).
This class assumes the 3ds Max coordinate system, i.e. X- and Y-axes are in the horizontal plane and the Z-axis goes up and down.
*/
class Ambient : public Grid {
    float               GRAVITY;                    /**< The gravity acceleration constant for the input units, e.g. 9.8 m/s^2 in metric or 386.1 in/s^2 in US customary units. */

    std::tr1::mt19937 engine;
    std::tr1::normal_distribution<float> dist;

    float               omega_0;                    /**< Dispersion-sub-naught; calculated using Tessendorf's equation (17). */
    int                 M;                          /**< Resolution of grid vertices along X-axis (16 <= M <= 2048; where M = 2^x for integer x). */
    int                 N;                          /**< Resolution of grid vertices Y-axis (16 <= N <= 2048; where N = 2^y for integer y). */
    float               Lx;                         /**< "Real-life" length of plane along X-axis (in m). */
    float               Ly;                         /**< "Real-life" length of plane along Y-axis (in m). */
    float               l;                          /**< Size limit that waves must surpass to be rendered. */
    float               V;                          /**< Wind speed (in m/s). */
    Point3              w_hat;                      /**< Direction of wind. */
    float               t;                          /**< Time (in s). */
    float               T;                          /**< Time of one phase of simulation. */
    unsigned long       seed;                       /**< Seed for the pseudorandom number generator. */

    // Values precached on initialization.
    float               P_h__L;                     /**< Precached for tessendorf::P_h. Largest possible waves arising from a continuous wind of speed V. */
    float               P_h__l_2;                   /**< Precached for tessendorf::P_h. Square of l (l being the wave size limit). */

    complex*            h_tildes_in;
    complex*            h_tildes_out;

public:
    static const float  GRAVITY_METRIC;             /**< The constant g in m/s^2. */
    static const float  GRAVITY_US;                 /**< Gravity in America. (Just kidding -- it's the constant g in in/s^2.) */

    /**
    Creates a new Tessendorf wave simulation at a specified time, given the specified parameters.

    \param width the width along the X-axis
    \param length the length along the Y-axis
    \param verticesX number of vertices along the X-axis
    \param verticesY number of vertices along the Y-axis
    \param rngSeed seed for the pseudorandom number generator
    \param phaseDuration duration of one phase (in s)
    \param accelerationGravity acceleration due to gravity; e.g. if using metric units, 9.8 m/s^2 and if using US customary, 386.1 in/s^2
    */
    Ambient(float width, float length, int verticesX, int verticesY, unsigned long rngSeed, float phaseDuration, float accelerationGravity);

    ~Ambient();

    /**
    Generates the wave surface and performs Fast Fourier Transforms (FFTs) to calculate the displacement.

    \param time time (in s)
    \param speed wind speed (in distance/s)
    \param direction direction of wind
    \param scale simulated length of plane along X-axis; Y-scale is automatically generated from this value
    \param waveSizeLimit size limit that waves must surpass to be rendered
    \param desiredMaxHeight height of the tallest wave
    */
    void                Simulate(float time, float speed, float direction, float scale, float waveSizeLimit, float desiredMaxHeight);

private:
    /**
    Gets the wave dispersion factor for a given vector k.
    Calculated using Tessendorf's equations (14) and (18) combined.
    */
    float               omega(Point3 k);

    /**
    Gets the value of the Phillips spectrum, which models wind-driven waves, for a given vector k.
    Calculated using Tessendorf's equations (23) and (24) combined.
    */
    float               P_h(Point3 k);

    /**
    Gets the value of h~-sub-naught for a given vector k at the current simulation time.
    Calculated using Tessendorf's equation (25).
    */
    complex             h_tilde_0(Point3 k);

    /**
    Gets the value of h~ for a given vector k at the current simulation time.
    Calculated using Tessendorf's equation (26).
    */
    complex             h_tilde(Point3 k);

    /**
    Generates the wave surface and performs Fast Fourier Transforms (FFTs) to calculate the displacement.
    The main height displacement is based on the Fourier series in Tessendorf's equation (19).
    The horizontal displacement is based on the Fourier series in equation (29).

    \param heightScale a factor by which every height is multiplied
    */
    void                heights(float heightScale);
};
