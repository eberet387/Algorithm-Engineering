#ifndef IMPLEMENTATION_HPP
#define IMPLEMENTATION_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <cmath>
#include <algorithm>

#if defined(__GNUC__) || defined(__clang__)  
#include <mm_malloc.h>
#define ALIGNED_ALLOC(alignment, size) _mm_malloc(size, alignment)
#define ALIGNED_FREE _mm_free
#elif defined(_MSC_VER)
#include <malloc.h>
#define ALIGNED_ALLOC(alignment, size) _aligned_malloc(size, alignment)
#define ALIGNED_FREE _aligned_free
#else
#include <cstdlib>
#define ALIGNED_ALLOC(alignment, size) std::aligned_alloc(alignment, size)
#define ALIGNED_FREE free
#endif

// Function declarations
unsigned char* load_ppm(const char* filename, int& width, int& height, int& channels);
int writePPM(const char* filename, int width, int height, unsigned char* data);
unsigned char* toGrayScale(unsigned char* img, int pixelAmount, int channels);
unsigned char* toEnhanced(unsigned char* image_greyscale, int pixelAmount, int channels, float* threshold);
float calculateMean(const unsigned char* img, int pixelAmount);
float* calculateDeviation(const unsigned char* img, int pixelAmount, float mean);
float calculateStandardDeviation(const unsigned char* img, int pixelAmount, float mean, float* deviation);
float checkMinDeviation(const unsigned char* img, int pixelAmount, float mean, float* deviation);
float checkMaxDeviation(const unsigned char* img, int pixelAmount, float mean, float* deviation);
int* calculateWindowBorder(int width, int height, int windowSize);
float* calculateWindowMean(const unsigned char* img, int width, int height, int windowSize, int* windowBorder);
float* calculateWindowStandardDeviation(const unsigned char* img, int width, int height, int windowSize, float* windowMean, float* deviation, int* windowBorder);
float* calculateAdaptiveDeviation(float* windowStandardDev, int pixelAmount, float minDev, float maxDev);
float* calculateThreshold(float mean, int pixelAmount, float* meanWindow, float* adaptiveDeviation, float* windowStandardDeviation, float noiseMultiplier);

#endif // IMPLEMENTATION_HPP
