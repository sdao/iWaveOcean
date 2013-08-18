#define _USE_MATH_DEFINES
#include <cmath>
#include <fftw3.h>
#include "Ambient.h"

Ambient::Ambient(float width, float length, int verticesX, int verticesY, unsigned long rngSeed, float phaseDuration) : Grid(width, length, verticesX - 1, verticesY - 1)
{
    // Static parameters.
    M = verticesX;
    N = verticesY;
    T = phaseDuration;

    omega_0 = 2. * M_PI / T;
    seed = rngSeed;
    engine = std::tr1::mt19937();

    h_tildes_in = new complex[M * N];
    h_tildes_out = new complex[M * N];
}

Ambient::~Ambient()
{   
    delete [] h_tildes_in;
    delete [] h_tildes_out;
}

float Ambient::omega(Point3 k)
{
    return floor(sqrt(GRAVITY * k.FLength()) / omega_0) * omega_0;
}

float Ambient::P_h(Point3 k)
{
    float k_length = k.FLength();

    if (k_length < DBL_EPSILON) {
        return 0.; // Avoid divison by zero error.
    }

    Point3 k_hat = k.FNormalize();

    float nomin = exp(-1. / pow(k_length * P_h__L, 2));
    float denom = pow(k_length, 4);
    float scale = exp(-pow(k_length, 2) * P_h__l_2);

    return nomin / denom * pow(DotProd(k_hat, w_hat), 2) * scale; // Let A = 1.0; A is not a useful user-facing attribute.
}

complex Ambient::h_tilde_0(Point3 k)
{
    return complex(dist(engine), dist(engine)) * (float)sqrt(P_h(k) / 2.);
}

complex Ambient::h_tilde(Point3 k)
{
    complex h_tilde_0_k = h_tilde_0(k);
    complex h_tilde_0_k_star = h_tilde_0(-k);

    float omega_k_t = omega(k) * t;

    float cos_omega_k_t = cos(omega_k_t);
    float sin_omega_k_t = sin(omega_k_t);

    complex c0(cos_omega_k_t, sin_omega_k_t);
    complex c1(cos_omega_k_t, -sin_omega_k_t);

    return h_tilde_0_k * c0 + h_tilde_0_k_star * c1;
}

void Ambient::heights(float time, float speed, Point3 angle, float scaleX, float scaleY, float waveSizeLimit, float* heights, float heightScale)
{
    // Animatable parameters.
    V = speed;
    w_hat = angle;
    t = time;
    Lx = scaleX;
    Ly = scaleY;
    l = waveSizeLimit;

    // Precalculate known constants.
    P_h__L = pow(V, 2) / GRAVITY;
    P_h__l_2 = pow(l, 2);

    engine.seed(seed);

    // Actual simulation begins here.
    for (int m = 0; m < M; m++) {
        for (int n = 0; n < N; n++) {
            int index = m * N + n;

            int m_ = m - M / 2;  // m coord offsetted.
            int n_ = n - N / 2; // n coord offsetted.

            Point3 k(2. * M_PI * n_ / Lx, 2. * M_PI * m_ / Ly, 0.);

            complex h_tilde_k = h_tilde(k);
            h_tildes_in[index] = h_tilde_k;
        }
    }

    fftwf_plan p_h = fftwf_plan_dft_1d(M*N, reinterpret_cast<fftwf_complex*>(h_tildes_in), reinterpret_cast<fftwf_complex*>(h_tildes_out), FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_execute(p_h);
    fftwf_destroy_plan(p_h);

    float signs[2] = { -1., 1. };

    for (int m = 0; m < M; m++) {
        for (int n = 0; n < N; n++) {
            int index = m * N + n;
            int sign = signs[(m + n) & 1]; // Sign-flip all of the odd coefficients.

            float m_ = m - M / 2.0f;  // m coord offsetted.
            float n_ = n - N / 2.0f;  // n coord offsetted.

            heights[index] = real(h_tildes_out[index]) * sign * heightScale;
        }
    }
}

void Ambient::Simulate(float time, float speed, float direction, float scale, float waveSizeLimit, float desiredMaxHeight)
{
    Point3 angle(cos(direction), sin(direction), 0.0f);
    float scaleX = scale;
    float scaleY = scale / (_width / _length);

    heights(0.0f, speed, angle, scaleX, scaleY, waveSizeLimit, _vertices, 1.0f);

    int numVertices = M * N;
    float currMaxHeight = 0.0f;
    for (int i = 0; i < numVertices; i++)
    {
        currMaxHeight = max(currMaxHeight, _vertices[i]);
    }

    heights(time, speed, angle, scaleX, scaleY, waveSizeLimit, _vertices, desiredMaxHeight / currMaxHeight);
}
