#define _USE_MATH_DEFINES
#include <cmath>
#include <fftw3.h>
#include "Ambient.h"

Ambient::Ambient(float amplitude, float speed, Point3 direction, float time, float phaseDuration, int resX, int resY, float scaleX, float scaleY, float waveSizeLimit, unsigned long rngSeed)
{
    // Parameters.
    A = amplitude;
    V = speed;
    w_hat = direction.FNormalize();
    t = time;
    M = resX + 1; // Math works better with no. vertices, but UI works better with no. faces.
    N = resY + 1;
    Lx = scaleX;
    Ly = scaleY;
    l = waveSizeLimit;
    T = phaseDuration;
    omega_0 = 2. * M_PI / T;
    seed = rngSeed;
    engine = std::tr1::mt19937();

    // Precalculate known constants.
    P_h__L = pow(V, 2) / GRAVITY;
    P_h__l_2 = pow(l, 2);
}

Ambient::~Ambient() {}

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

    return A * nomin / denom * pow(DotProd(k_hat, w_hat), 2) * scale;
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

void Ambient::Simulate(float* heights)
{
    engine.seed(seed);

    complex* h_tildes_in = new complex[M*N];
    complex* h_tildes_out = new complex[M*N];

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

            heights[index] = real(h_tildes_out[index]) * sign;
        }
    }

    delete [] h_tildes_in;
    delete [] h_tildes_out;
}
