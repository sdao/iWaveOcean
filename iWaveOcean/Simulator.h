#pragma once

#include <point3.h>

class Simulator
{
public:
    Simulator(void);
    ~Simulator(void);
    static void GetPlane(float width, float length, int widthSegs, int lengthSegs, Point3* outData);
};

