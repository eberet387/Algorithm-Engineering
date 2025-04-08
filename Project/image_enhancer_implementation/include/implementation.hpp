#ifndef IMPLEMENTATION_HPP
#define IMPLEMENTATION_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <cmath>
#include <algorithm>
#include <omp.h>

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

// function to load a PPM image
unsigned char* load_ppm(const char* filename, int& width, int& height, int& channels) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file " << filename << std::endl;
        return nullptr;
    }

    std::string line;
    file >> line; // read the PPM format identifier (P3)

    if (line != "P3") {
        std::cerr << "Error: Unsupported PPM format (only P3 supported)." << std::endl;
        return nullptr;
    }

    // skip comments
    while (file.peek() == '#') {
        std::getline(file, line);
    }

    // read dimensions
    file >> width >> height;

    int max_val;
    file >> max_val;
    if (max_val != 255) {
        std::cerr << "Error: Unsupported max value in PPM (only 255 supported)." << std::endl;
        return nullptr;
    }

    channels = 3; // P3 format has 3 channels (RGB)

    // allocate memory for the image data
    unsigned char* data = static_cast<unsigned char*>(
        ALIGNED_ALLOC(64, width * height * channels * sizeof(unsigned char))
    );

    // read and verify pixel data
    for (int i = 0; i < width * height * channels; ++i) {
        int value;
        file >> value;
        if (value < 0 || value > 255) {
            std::cerr << "Error: Invalid pixel value in PPM file." << std::endl;
            ALIGNED_FREE(data);
            return nullptr;
        }
        data[i] = static_cast<unsigned char>(value);
    }
    file.close();
    return data;
}

// function to write data to a ppm P3 file
int writePPM(const char* filename, int width, int height, unsigned char* data) {
    // open the output file
    std::ofstream file(filename);
    if (!file) {
        return 0;
    }

    // write the PPM header
    file << "P3\n";                // P3 format
    file << width << " " << height << "\n"; // dimensions
    file << "255\n";              // max color value

    // write pixel data
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int r = static_cast<unsigned char>(data[i * width + j]);
            int g = static_cast<unsigned char>(data[i * width + j]);
            int b = static_cast<unsigned char>(data[i * width + j]);

            file << r << " " << g << " " << b << "   ";
        }
        file << "\n";
    }

    // close file
    file.close();
    if (!file) {
        std::cerr << "Error: Failed to write to file " << filename << std::endl;
        return 0;
    } else {
        std::cout << "PPM file written successfully: " << filename << std::endl;
    }
    return 1;
}

// function to convert an image to grayscale
unsigned char* toGrayScale(unsigned char* img, int pixelAmount, int channels) {
    unsigned char* imageGrayScale = static_cast<unsigned char*>(
        ALIGNED_ALLOC(64, pixelAmount * sizeof(unsigned char))
    );

    #pragma omp parallel for simd schedule(simd:static) aligned(img, imageGrayScale:64)
    for (int i = 0; i < pixelAmount; ++i) {
        int pixel_index = i * channels;
        unsigned char grayscale = (img[pixel_index] + img[pixel_index + 1] + img[pixel_index + 2]) / 3;
        imageGrayScale[i] = grayscale;
    }
    return imageGrayScale;
}

// function to enhance the readability based on the new calculated grayscale threshold
unsigned char* toEnhanced(unsigned char* image_greyscale, int pixelAmount, int channels, float* threshold) {
    unsigned char* enhanced_img = static_cast<unsigned char*>(
        ALIGNED_ALLOC(64, pixelAmount * sizeof(unsigned char))
    );

    #pragma omp parallel for simd schedule(simd:static) aligned(image_greyscale, enhanced_img, threshold:64)
    for (int i = 0; i < pixelAmount; ++i) {
        enhanced_img[i] = image_greyscale[i] < threshold[i] ? 0 : 255;
    }
    return enhanced_img;
}

// function to calculate the average grayscale value of the image
float calculateMean(const unsigned char* img, int pixelAmount) {
    long long sum = 0;

    #pragma omp parallel for simd reduction(+:sum) schedule(simd:static) aligned(img:64)
    for (int i = 0; i < pixelAmount; ++i) {
        sum += img[i];
    }

    return static_cast<float>(sum) / pixelAmount;
}

// function to calculte the deviation of each pixel
float* calculateDeviation(const unsigned char* img, int pixelAmount, float mean) {
    float* deviation = static_cast<float*>(
        ALIGNED_ALLOC(64, pixelAmount * sizeof(float))
    );

    #pragma omp parallel for simd schedule(simd:static) aligned(img, deviation:64)
    for (int i = 0; i < pixelAmount; ++i) {
        deviation[i] = std::abs(img[i] - mean);
    }

    return deviation;
}

// function to calculate the standard deviation (average squared deviation)
float calculateStandardDeviation(const unsigned char* img, int pixelAmount, float mean, float* deviation) {
    long long sum_squared_dev = 0.0;

    #pragma omp parallel for simd reduction(+:sum_squared_dev) schedule(simd:static) aligned(deviation:64)
    for (int i = 0; i < pixelAmount; ++i) {
        sum_squared_dev += deviation[i] * deviation[i];
    }
    
    return static_cast<float>(sum_squared_dev) / pixelAmount;
}

// simple function to check for the minimum deviation value
float checkMinDeviation(const unsigned char* img, int pixelAmount, float mean, float* deviation) {
    float min_deviation = 255;

    #pragma omp parallel for simd reduction(min:min_deviation) schedule(simd:static) aligned(deviation:64)
    for (int i = 0; i < pixelAmount; ++i) {
        min_deviation = std::min(min_deviation, deviation[i]);
    }

    return min_deviation;
}

// simple function to check for the maximum deviation value
float checkMaxDeviation(const unsigned char* img, int pixelAmount, float mean, float* deviation) {
    float max_deviation = 0;

    #pragma omp parallel for simd reduction(max:max_deviation) schedule(simd:static) aligned(deviation:64)
    for (int i = 0; i < pixelAmount; ++i) {
        max_deviation = std::max(max_deviation, deviation[i]);
    }

    return max_deviation;
}

// function to calculate the start and end points of each pixels window
int* calculateWindowBorder(int width, int height, int windowSize) {
    int* windowBorder = static_cast<int*>(
        ALIGNED_ALLOC(64, width * height * 5 * sizeof(int))
    );

    int halfWindowSize = windowSize / 2;
    int remainder = windowSize - halfWindowSize;
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int cur_pixel = (i * width + j) * 5;
            windowBorder[cur_pixel] = std::max(0, i - halfWindowSize); // starting Y position
            windowBorder[cur_pixel + 1] = std::max(0, j - halfWindowSize); // starting X position
            windowBorder[cur_pixel + 2] = std::min(i + remainder - 1, height - 1); // ending Y position
            windowBorder[cur_pixel + 3] = std::min(j + remainder - 1, width - 1); // starting X position
            windowBorder[cur_pixel + 4] = (windowBorder[cur_pixel + 3] - windowBorder[cur_pixel + 1] + 1) * 
                                         (windowBorder[cur_pixel + 2] - windowBorder[cur_pixel] + 1); // amount of pixels within the scope
        }
    }

    return windowBorder;
}

// function to calculate the average grayscale value within window of pixels arround the target pixel
float* calculateWindowMean(const unsigned char* img, int width, int height, int windowSize, int* windowBorder) {
    float* windowMean = static_cast<float*>(
        ALIGNED_ALLOC(64, width * height * sizeof(float))
    );

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int cur_pixel = (i * width + j) * 5;
            int startY = windowBorder[cur_pixel];
            int startX = windowBorder[cur_pixel + 1];
            int endY = windowBorder[cur_pixel + 2];
            int endX = windowBorder[cur_pixel + 3];
            int count = windowBorder[cur_pixel + 4];

            long long sum = 0;
            
            for (int y = startY; y <= endY; ++y) {
                for (int x = startX; x <= endX; ++x) {
                    sum += img[y * width + x];
                }
            }
            windowMean[i * width + j] = static_cast<float>(sum) / count;
        }
    }
    
    return windowMean;
}


// Function to apply fast spatial averaging
float* fastSpatialAveraging(const unsigned char* img, int width, int height, int windowSize) {
    float* windowMean = static_cast<float*>(
        ALIGNED_ALLOC(64, width * height * sizeof(float))
    );

    int* average = static_cast<int*>(
        ALIGNED_ALLOC(64, 255 * width * height * sizeof(int))
    );


    int col_sum[height + windowSize - 2]; // Array to store column sums

    int k = 0;  // Counter to keep track of elements in the window
    int j = 0;  // Average value index

    // First pass: Compute average values for the sliding window
    for (int i = 0; i < 255 * windowSize * windowSize; ++i) {
        if (k == windowSize * windowSize) {
            k = 0;
            j = j + 1;
        }
        average[i] = j;
        k = k + 1;
    }

    // Second pass: Perform sliding window summation and apply averages
    for (int j = 0; j < height + windowSize - 2; ++j) {
        col_sum[j] = 0;
        for (int i = 0; i < windowSize/2; ++i) {
            col_sum[j] += img[(i)* width + j - windowSize/2];
        }
    }

    int sum = 0;
    for (int i = 0; i < windowSize; ++i) {
        sum += col_sum[i];
    }

    // Calculate the first column's average
    windowMean[0] = average[sum];

    // Calculate the rest of the columns for the first row
    for (int col = 1; col < height - 1; ++col) {
        sum = sum - col_sum[col - 1] + col_sum[col + windowSize - 1];
        windowMean[col] = average[sum];
    }

    // For the rest of the rows
    for (int row = 1; row < width - 1; ++row) {
        for (int j = 0; j < height + windowSize - 2; ++j) {
            col_sum[j] = col_sum[j] - img[(row - 1) * width + j] + img[(row + windowSize - 1) * width + j];
        }

        sum = 0;
        for (int i = 0; i < windowSize; ++i) {
            sum += col_sum[i];
        }

        // First column of the current row
        windowMean[row * width] = average[sum];

        // Rest of the columns of the current row
        for (int col = 1; col < height - 1; ++col) {
            sum = sum - col_sum[col - 1] + col_sum[col + windowSize - 1];
            sum = std::max(0, sum);
            windowMean[row * width + col] = average[sum];
        }
    }

    return windowMean;
}

float* calculateWindowSquaredDiff(int width, int height, float* windowMean, float* deviation) {
    float* squaredDiff = static_cast<float*>(
        ALIGNED_ALLOC(64, width * height * sizeof(float))
    );

    #pragma omp parallel for collapse(2) schedule(simd: static)
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            squaredDiff[i * width + j] = (deviation[i * width + j] - windowMean[i * width + j]) * (deviation[i * width + j] - windowMean[i * width + j]);
        }
    }

    return squaredDiff;
}

// Function to apply fast window standard deviation
float* fastWindowStandardDeviation(int width, int height, int windowSize, float* squaredDiff) {
    float* windowMean = static_cast<float*>(
        ALIGNED_ALLOC(64, width * height * sizeof(float))
    );

    int* average = static_cast<int*>(
        ALIGNED_ALLOC(64, 255 * width * height * sizeof(int))
    );


    int col_sum[height + windowSize - 2]; // Array to store column sums

    int k = 0;  // Counter to keep track of elements in the window
    int j = 0;  // Average value index

    // First pass: Compute average values for the sliding window
    for (int i = 0; i < 255 * windowSize * windowSize; ++i) {
        if (k == windowSize * windowSize) {
            k = 0;
            j = j + 1;
        }
        average[i] = j;
        k = k + 1;
    }

    // Second pass: Perform sliding window summation and apply averages
    for (int j = 0; j < height + windowSize - 2; ++j) {
        col_sum[j] = 0;
        for (int i = 0; i < windowSize/2; ++i) {
            col_sum[j] += squaredDiff[(i)* width + j - windowSize/2];
        }
    }

    int sum = 0;
    for (int i = 0; i < windowSize; ++i) {
        sum += col_sum[i];
    }

    // Calculate the first column's average
    windowMean[0] = std::sqrt(average[sum]);

    // Calculate the rest of the columns for the first row
    for (int col = 1; col < height - 1; ++col) {
        sum = sum - col_sum[col - 1] + col_sum[col + windowSize - 1];
        windowMean[col] = std::sqrt(average[sum]);
    }

    // For the rest of the rows
    for (int row = 1; row < width - 1; ++row) {
        for (int j = 0; j < height + windowSize - 2; ++j) {
            col_sum[j] = col_sum[j] - squaredDiff[(row - 1) * width + j] + squaredDiff[(row + windowSize - 1) * width + j];
        }

        sum = 0;
        for (int i = 0; i < windowSize; ++i) {
            sum += col_sum[i];
        }

        // First column of the current row
        sum = std::max(0, sum);
        windowMean[row * width] = std::sqrt(average[sum]);

        // Rest of the columns of the current row
        for (int col = 1; col < height - 1; ++col) {
            sum = sum - col_sum[col - 1] + col_sum[col + windowSize - 1];
            sum = std::max(0, sum);
            windowMean[row * width + col] = std::sqrt(average[sum]);
        }
    }

    return windowMean;
}

// function to calculate the standard deviation within window of pixels arround the target pixel
float* calculateWindowStandardDeviation(const unsigned char* img, int width, int height, int windowSize, 
                                       float* squaredDiff, int* windowBorder) {
    float* windowStandardDev = static_cast<float*>(
        ALIGNED_ALLOC(64, width * height * sizeof(float))
    );

    #pragma omp parallel for collapse(2) schedule(simd: static)
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int cur_pixel = (i * width + j) * 5;
            int startY = windowBorder[cur_pixel];
            int startX = windowBorder[cur_pixel + 1];
            int endY = windowBorder[cur_pixel + 2];
            int endX = windowBorder[cur_pixel + 3];
            int count = windowBorder[cur_pixel + 4];

            float sum_squared_diff = 0.0f;

            for (int y = startY; y <= endY; ++y) {
                for (int x = startX; x <= endX; ++x) {
                    sum_squared_diff += squaredDiff[y * width + x];
                }
            }
            windowStandardDev[i * width + j] = std::sqrt(sum_squared_diff / count);
        }
    }
    
    return windowStandardDev;
}

// function to calculate the adaptive deviation of each pixel
float* calculateAdaptiveDeviation(float* windowStandardDev, int pixelAmount, float minDev, float maxDev) {
    float* adaptiveDev = static_cast<float*>(
        ALIGNED_ALLOC(64, pixelAmount * sizeof(float))
    );

    float devDifference = maxDev - minDev;

    #pragma omp parallel for simd safelen(16) aligned(windowStandardDev, adaptiveDev: 64)
    for (int i = 0; i < pixelAmount; ++i) {
        adaptiveDev[i] = (windowStandardDev[i] - minDev) * devDifference;
    }
    return adaptiveDev;
}

// function to calculate the grayscale threshold
float* calculateThreshold(float mean, int pixelAmount, float* meanWindow, 
                          float* adaptiveDeviation, float* windowStandardDeviation, float noiseMultiplier) {
    float* threshold = static_cast<float*>(
        ALIGNED_ALLOC(64, pixelAmount * sizeof(float))
    );

    #pragma omp parallel for simd safelen(16) aligned(meanWindow, adaptiveDeviation, windowStandardDeviation, threshold: 64)
    for (int i = 0; i < pixelAmount; ++i) {
        threshold[i] = (meanWindow[i] - (meanWindow[i] * meanWindow[i] - windowStandardDeviation[i]) / 
                       ((mean + windowStandardDeviation[i]) * (adaptiveDeviation[i] + windowStandardDeviation[i]))) * noiseMultiplier;
    }
    return threshold;
}

#endif // IMPLEMENTATION_HPP
