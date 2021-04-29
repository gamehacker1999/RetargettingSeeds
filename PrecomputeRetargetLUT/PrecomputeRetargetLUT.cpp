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

void CreateTargetTexture(unsigned char* source, unsigned char* target, int w, int h)
{
    float x;
    float y;

    GenerateR2Sequence(1, x, y);

    int xOffset = static_cast<int>(x * w);
    int yOffset = static_cast<int>(y * h);

    int bytesPerLine = w * 4;

    float energy = 0.0f;
    for (size_t j = 0; j < h; j++)
    {
        for (size_t i = 0; i < w; i++)
        {
            auto pixel = target + j * bytesPerLine + i * 4;

            int newX = (i + xOffset) % w;
            int newY = (j + yOffset) % h;

            auto movedPixel = source + newY * bytesPerLine + newX * 4;

            pixel[0] = movedPixel[0];
            pixel[1] = movedPixel[1];
            pixel[2] = movedPixel[2];
            pixel[3] = movedPixel[3];


        }
    }

}


float Energy(unsigned char* data, unsigned char* target, int wdimension, int hdimension)
{
   
    int bytesPerLine = wdimension * 4;

    float energy = 0.0f;
    for (size_t j = 0; j < hdimension; j++)
    {
        for (size_t i = 0; i < wdimension; i++)
        {
            auto pixel = data + j * bytesPerLine + i * 4;

            auto movedPixel = target + j * bytesPerLine + i * 4;

            float differenceSquared = (movedPixel[0] - pixel[0])* (movedPixel[0] - pixel[0]);
            energy += differenceSquared;

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
    unsigned char* data = stbi_load("Textures/BlueNoise256.png", &width, &height, &comp, 4);

    float initialTemperature = 1.f;
    float reductionFactor = 0.0001f;
    float swapCountFactor = 0.01f;

    uint64_t numSwaps = uint64_t(swapCountFactor * long(width * height*100)); // how many swaps per trial
    float coolingRate = 1.0f / (reductionFactor * double(width * height*100)); // how much the temperature cools each iteration

    float* dataOut = new float[width * height * 4];
    unsigned char* swapData = new unsigned char[width * height * 4];
    unsigned char* targetTexture = new unsigned char[width * height * 4];

    CreateTargetTexture(data, targetTexture, width, height);



    std::random_device rd;

    std::mt19937 random(rd());

    int bytesPerLine = width * 4;

    float T = initialTemperature;

    float energy = Energy(data, targetTexture,width, height);

    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    std::uniform_int_distribution<int> iDistW(0, width-1);
    std::uniform_int_distribution<int> iDistH(0, height-1);
    std::uniform_int_distribution<int> iDist6(-3, 3);

    for (size_t j = 0; j < height; j++)
    {
        for (size_t i = 0; i < width; i++)
        {
            auto pixel = dataOut + j * bytesPerLine + i * 4;

            pixel[0] = float(i);
            pixel[1] = float(j);
            pixel[2] = 0;
            pixel[3] = 255.f;
        }
    }


    for (int64_t swapIndex = 0; swapIndex < numSwaps; ++swapIndex)
    {

        std::cerr << "\rSwaps Left " << numSwaps - swapIndex <<' ' <<std::flush;

        T = std::max(T - coolingRate, 0.0f);

        memcpy(swapData, data, width*height*4*sizeof(unsigned char));

        uint32_t indexAx = iDistW(random);
        uint32_t indexAy = iDistH(random);

        uint32_t indexBx = (std::labs(indexAx + iDist6(random))) % width;
        uint32_t indexBy = (std::labs(indexAy + iDist6(random))) % height;

        auto indexA = indexAy * bytesPerLine + indexAx * 4;
        auto indexB = indexBy * bytesPerLine + indexBx * 4;

        std::swap(swapData[indexA], swapData[indexB]);

        float swapEnergy = Energy(swapData, targetTexture, width, height);

        float rng01 = dist(random);
        if ((swapEnergy < energy) || rng01 < T)
        {
            memcpy(data, swapData, width * height * 4 * sizeof(unsigned char));
            energy = swapEnergy;

            std::swap(dataOut[indexA], dataOut[indexB]);

        }

    }

    stbi_write_png("Textures/Retarget3.png", width, height, 4, dataOut, width*4);

    delete[] dataOut;
    delete[] swapData;
    delete[] targetTexture;

}