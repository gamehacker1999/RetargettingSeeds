// PrecomputeRetargetLUT.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include<math.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <random>

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

//http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
void GenerateR2Sequence(int n, float& x, float& y)
{
    float g = 1.32471795724474602596;

    float a1 = 1.0 / g;

    float a2 = 1.0 / (g * g);

    float num1 = std::fmodf((0.5 + a1 * n) , 1);

    float num2 = std::fmodf((0.5 + a2 * n) ,1);

    x = num1;
    y = num2;
}

bool CheckBounds(int x, int max)
{
    if (x >= 0 && x < max)
    {
        return true;
    }

    return false;
}


float Energy(unsigned char* data, int dimension)
{
    float x;
    float y;

    GenerateR2Sequence(1, x, y);

    int xOffset = static_cast<int>(x * dimension);
    int yOffset = static_cast<int>(y * dimension);
    
    int bytesPerLine = dimension * 4;

    float energy = 0.0f;
    for (size_t j = 0; j < dimension; j++)
    {
        for (size_t i = 0; i < dimension; i++)
        {
            auto pixel = data + j * bytesPerLine + i * 4;

            int newX = (i + xOffset) % dimension;
            int newY = (j + yOffset) % dimension;

            auto movedPixel = data + newY * bytesPerLine + newX * 4;

            float distanceSquared = (movedPixel - pixel)* (movedPixel - pixel);
            energy += distanceSquared;

        }
    }

    return energy;
}

float map(float input, float input_start, float input_end, float output_start, float output_end)
{
    double slope = 1.0 * (output_end - output_start) / (input_end - input_start);
    float output = output_start + slope * (input - input_start);

    return output;
}

float PixelDistance(float ax, float ay, float bx, float by)
{
    float dist = ((ax - bx) * (ax - bx)) + ((ay - by) * (ay - by));
    return dist;
}

int main()
{
    int width;
    int height;
    int comp;
    unsigned char* data = stbi_load("Textures/BlueNoise512.png", &width, &height, &comp, 4);

    float initialTemperature = 1.f;
    float reductionFactor = 0.0001f;
    float swapCountFactor = 0.01f;

    int64_t numSwaps = int64_t(swapCountFactor * double(512 * 512)); // how many swaps per trial
    float coolingRate = 1.0f / (reductionFactor * double(32 * 32*32*32)); // how much the temperature cools each iteration

    float* dataOut = new float[width * height * 4];
    unsigned char* swapData = new unsigned char[width * height * 4];

    std::random_device rd;

    std::mt19937 random(rd());

    int bytesPerLine = width * 4;
    int bytesPerLine2 = width * 2;

    float T = initialTemperature;

    float energy = Energy(data, 512);

    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    std::uniform_int_distribution<int> iDist(0, 512);

    for (size_t j = 0; j < 512; j++)
    {
        for (size_t i = 0; i < 512; i++)
        {
            auto pixel = dataOut + j * bytesPerLine + i * 4;

            float xVal = map(i, 0, 511, 0, 255);
            float yVal = map(j, 0, 511, 0, 255);

            pixel[0] = xVal;
            pixel[1] = yVal;
            pixel[2] = 0;
            pixel[3] = 1.f;
        }
    }


    for (int64_t swapIndex = 0; swapIndex < numSwaps; ++swapIndex)
    {

        std::cerr << "\rSwaps Left " << numSwaps - swapIndex <<' ' <<std::flush;

        T = std::max(T - coolingRate, 0.0f);

        swapData = data;

        uint32_t indexAx = iDist(random);
        uint32_t indexAy = iDist(random);

        uint32_t indexBx = iDist(random);
        uint32_t indexBy = iDist(random);

        auto indexA = indexAy * bytesPerLine + indexAx * 4;
        auto indexB = indexBy * bytesPerLine + indexBx * 4;

        std::swap(swapData[indexA], swapData[indexB]);

        float swapEnergy = Energy(swapData, 512);

        float rng01 = dist(random);
        if (((swapEnergy < energy) && PixelDistance(indexAx, indexAy,indexBx, indexBy)<=36) || rng01 < T)
        {
            data = swapData;
            energy = swapEnergy;

            std::swap(dataOut[indexA], dataOut[indexB]);

        }

    }

    stbi_write_png("Textures/Retarget1.png", width, height, 4, dataOut, 0);

    delete[] dataOut;
    delete[] swapData;

}