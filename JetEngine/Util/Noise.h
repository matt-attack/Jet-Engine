#ifndef NOISE_HEADER
#define NOISE_HEADER

double easeCurve(double t);

// Return value: -1 ... 1
double noise2d(int x, int y, int seed);
double noise3d(int x, int y, int z, int seed);

double noise2d_gradient(double x, double y, int seed);
double noise3d_gradient(double x, double y, double z, int seed);

double noise2d_perlin(double x, double y, int seed, int octaves, double persistence);
double noise2d_perlin_abs(double x, double y, int seed, int octaves, double persistence);

double noise3d_perlin(double x, double y, double z, int seed, int octaves, double persistence);
double noise3d_perlin_abs(double x, double y, double z, int seed, int octaves, double persistence);

#endif
