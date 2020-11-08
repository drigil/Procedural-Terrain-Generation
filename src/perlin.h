#ifndef TINYTRIANGLES_PERLIN_H
#define TINYTRIANGLES_PERLIN_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <limits>


//Function to get Perlin noise map with required dimensions
std::vector<std::vector<float>> generateNoiseMap(int mapWidth, int mapHeight, float scale, int octaves, float persistance, float lacunarity);

//Linear Interpolation
double lerp(double t, double a, double b);

double fade(double t);

double grad(int hash, double x, double y, double z);

double getPerlinNoise(float x, float y);

std::vector<int> getPermutationVector ();

#endif //TINYTRIANGLES_PERLIN_H