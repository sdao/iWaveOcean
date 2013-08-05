#include "Simulator.h"


Simulator::Simulator(void)
{
}


Simulator::~Simulator(void)
{
}


void Simulator::GetPlane(float width, float length, int widthSegs, int lengthSegs, Point3* outData)
{
    float halfWidth = width / 2.0f;
    float halfLength = length / 2.0f;

    int vtx = 0;
    for (int i = 0; i <= widthSegs; i++)
    {
        for (int j = 0; j <= widthSegs; j++)
        {
            outData[vtx] = Point3(i * width / widthSegs - halfWidth, j * length / lengthSegs - halfLength, 0.0f);
            vtx++;
        }
    }
}