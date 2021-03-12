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

int main()
{
    int width;
    int height;
    int comp;
    unsigned char* data = stbi_load("Textures/BlueNoiseRes.png", &width, &height, &comp, 4);

    float initialTemperature = 1.f;
    float minTemperature = 0.0001f;
    float reductionFactor = 0.9f;
    unsigned char* dataOut = new unsigned char[width * height * 4];

    srand(time(NULL));

    std::random_device rd;

    std::mt19937 random(rd());

    int bytesPerLine = width * 4;
    int bytesPerLine2 = width * 2;

    float T = initialTemperature;
    while (T > minTemperature)
    {
        std::uniform_int_distribution<int> dist(-6, 6);
        std::uniform_real_distribution<float> fDist(0.f, 1.f);
        int offset = 0;
        for (size_t j = 0; j < height; j++)
        {
            std::cerr << "\rScanlines remaining: " << (height - j) << ' ' << std::flush;
            for (size_t i = 0; i < width; i++)
            {
                int randomNumX = dist(random);
                int randomNumY = dist(random);
    
                int locX = i + randomNumX;
                int locY = j + randomNumY;
                if (CheckBounds(locX, width) && CheckBounds(locY, height))
                {
                    auto pixel = data + locY * bytesPerLine + locX * 4;
                    auto outLoc = dataOut + j * bytesPerLine + i * 4;
    
                    {
                        outLoc[0] = (i);
                        outLoc[1] = (j);
                        outLoc[2] = 0;
                        outLoc[3] = 255;
                    }
    
                    float r2X, r2Y;
    
                    GenerateR2Sequence(1, r2X, r2Y);

                    //r2X = r2X * 2.f - 1.f;
                    //r2Y = r2Y * 2.f - 1.f;
    
                    auto ir2X = static_cast<int>(r2X * width);
                    auto ir2Y = static_cast<int>(r2Y * height);
    
                    int newJ = ((j + ir2Y) % height);
                    int newI = ((i + ir2X) % width);
    
                    auto modifiedPixel = data +  newJ* bytesPerLine + newI * 4;
    
                    if (((pixel[0] - modifiedPixel[0]) / T) > fDist(random))
                    {
                        outLoc[0] = (locX/width)*255.99;
                        outLoc[1] = (locY/height)*255.99;
                        outLoc[2] = 0;
                        outLoc[3] = 255;

                        pixel[0] = (i/width)*255.99;
                        pixel[1] = (j/height)*255.99;
                        pixel[2] = 0;
                        pixel[3] = 255;
    
                    }
                }
    
                
            }
        }
    
        T *= reductionFactor;
    }


    //std::uniform_int_distribution<int> dist(-6, 6);
    //std::uniform_real_distribution<float> fDist(0.f, 1.f);
    //int offset = 0;
    //for (size_t j = 0; j < height; j++)
    //{
    //    std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
    //    for (size_t i = 0; i < width; i++)
    //    {
    //
    //        float T = initialTemperature;
    //        while (T > minTemperature)
    //        {
    //
    //            int randomNumX = dist(random);
    //            int randomNumY = dist(random);
    //            int locX = i + randomNumX;
    //            int locY = j + randomNumY;
    //            if (CheckBounds(locX, width) && CheckBounds(locY, height))
    //            {
    //                auto pixel = data +  locY* bytesPerLine + locX * 4;
    //                auto outLoc = dataOut + j * bytesPerLine + i * 4;
    //
    //                {
    //                    outLoc[0] = (i);
    //                    outLoc[1] = (j);
    //                    outLoc[2] = 0;
    //                    outLoc[3] = 255;
    //                }
    //
    //                float r2X, r2Y;
    //
    //                GenerateR2Sequence(1, r2X, r2Y);
    //
    //                auto ir2X = static_cast<int>(r2X * width);
    //                auto ir2Y = static_cast<int>(r2Y * height);
    //
    //                auto modifiedPixel = data + ((j+ir2Y)%height) * bytesPerLine + ((i+ir2X)%width) * 4;
    //
    //                if (((pixel[0] - modifiedPixel[0]) / T) > fDist(random))
    //                {
    //                    outLoc[0] = locX;
    //                    outLoc[1] = locY;
    //                    outLoc[2] = 0;
    //                    outLoc[3] = 255;
    //
    //                    pixel[0] = i;
    //                    pixel[1] = j;
    //                    pixel[2] = 0;
    //                    pixel[3] = 255;
    //
    //                }
    //            }
    //
    //            T *= reductionFactor;
    //        }
    //    }
    //}


    stbi_write_png("Textures/Retarget.png", width, height, 4, dataOut, 0);

    delete[] dataOut;


}