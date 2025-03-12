#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <cmath>
#include <cstdlib>
#include <malloc.h>
#include <omp.h>
#include "stb_image.h"
#include "stb_image_write.h"
#include <algorithm>


#if defined(__GNUC__) || defined(__clang__)
#include <mm_malloc.h>
#define ALIGNED_ALLOC(alignment, size) _mm_malloc(size, alignment)
#define ALIGNED_FREE _mm_free
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

    #pragma omp parallel for 
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

    #pragma omp parallel for
    for (int i = 0; i < pixelAmount; ++i) {
        enhanced_img[i] = image_greyscale[i] < threshold[i] ? 0 : 255;
    }
    return enhanced_img;
}

// function to calculate the average grayscale value of the image
float calculateMean(const unsigned char* img, int pixelAmount) {
    long long sum = 0;

    #pragma omp parallel for reduction(+:sum)
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

    #pragma omp parallel for
    for (int i = 0; i < pixelAmount; ++i) {
        deviation[i] = std::abs(img[i] - mean);
    }

    return deviation;
}

// function to calculate the standard deviation (average squared deviation)
float calculateStandardDeviation(const unsigned char* img, int pixelAmount, float mean, float* deviation) {
    long long sum_squared_dev = 0.0;

    #pragma omp parallel for reduction(+:sum_squared_dev)
    for (int i = 0; i < pixelAmount; ++i) {
        sum_squared_dev += deviation[i] * deviation[i];
    }
    
    return static_cast<float>(sum_squared_dev) / pixelAmount;
}

// simple function to check for the minimum deviation value
float checkMinDeviation(const unsigned char* img, int pixelAmount, float mean, float* deviation) {
    float min_deviation = 255;

    #pragma omp parallel for reduction(min:min_deviation)
    for (int i = 0; i < pixelAmount; ++i) {
        min_deviation = std::min(min_deviation, deviation[i]);
    }

    return min_deviation;
}

// simple function to check for the maximum deviation value
float checkMaxDeviation(const unsigned char* img, int pixelAmount, float mean, float* deviation) {
    float max_deviation = 0;

    #pragma omp parallel for reduction(max:max_deviation)
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

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int cur_pixel = (i * width + j) * 5;
            windowBorder[cur_pixel] = std::max(0, i - halfWindowSize);
            windowBorder[cur_pixel + 1] = std::max(0, j - halfWindowSize);
            windowBorder[cur_pixel + 2] = std::min(i + halfWindowSize, height - 1);
            windowBorder[cur_pixel + 3] = std::min(j + halfWindowSize, width - 1);
            windowBorder[cur_pixel + 4] = (windowBorder[cur_pixel + 3] - windowBorder[cur_pixel + 1] + 1) * 
                                         (windowBorder[cur_pixel + 2] - windowBorder[cur_pixel] + 1);
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

// function to calculate the standard deviation within window of pixels arround the target pixel
float* calculateWindowStandardDeviation(const unsigned char* img, int width, int height, int windowSize, 
                                       float* windowMean, float* deviation, int* windowBorder) {
    float* windowStandardDev = static_cast<float*>(
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

            float sum_squared_diff = 0.0f;

            for (int y = startY; y <= endY; ++y) {
                for (int x = startX; x <= endX; ++x) {
                    float diff = deviation[y * width + x] - windowMean[y * width + x];
                    sum_squared_diff += diff * diff;
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

    #pragma omp parallel for 
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

    #pragma omp parallel for
    for (int i = 0; i < pixelAmount; ++i) {
        threshold[i] = (meanWindow[i] - (meanWindow[i] * meanWindow[i] - windowStandardDeviation[i]) / 
                       ((mean + windowStandardDeviation[i]) * (adaptiveDeviation[i] + windowStandardDeviation[i]))) * noiseMultiplier;
    }
    return threshold;
}

// main function
int main(int argc, char** argv) {
    // handle the case where nothing is specified / provided
    if (argc <= 1) {
        std::cerr << "Usage: " << argv[0] << " <image_path> {arguments}" <<  std::endl << "or use flag -h instad of an image path for help.";
        return 1;
    }

    // base arguments if not specified
    std::string output = "output/";
    std::string filetype;
    bool verbose = false;
    bool benchmark = false;
    double start, end;
    int windowSize = 20;
    float noiseMultiplier = 0.75;

    // argument logic
    // argument list:
    //  -o="output_path.filetype"
    //  -v -> verbose
    //  -w=windowSize (int)
    //  -b -> benchmark mode
    //  -n=noiseMultiplier (int) 0-100, however interesting results can be achieved if the noiseMultiplier exceeds 100.
    //  -h -> help
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (i == 1 && arg.substr(0, 2) == "-h") {
            printf("This program helps enhance images for printing / other use.\nUsage: %s <image_path> {arguments}\n\nArgument List:\n-v   Verbose Mode\n-b   Benchmark Mode\n-o=\"filepath.filetype\" -> specify the output\n-w=windowSize -> specify the window size of nearest pixels that influence the adaptive thresholds (standard: 20)\n-n=noiseMultiplier -> specify a number between 0 and 100 - lower number means less noise\n");
            return 1;
        }
        if (arg.substr(0, 2) == "-o") {
            output += arg.substr(3); 

            size_t pos = output.rfind('.');
            if (output.rfind('.') == std::string::npos) {
                std::cerr << "Must include output file type" << std::endl;
                return 1;
            } else {
                filetype = output.substr(pos);
            }
        } else if (arg.substr(0, 2) == "-v") {
            verbose = true;
        } else if (arg.substr(0, 2) == "-w") {
            windowSize = std::stoi(arg.substr(3));
        } else if (arg.substr(0, 2) == "-b") {
            benchmark = true;
        } else if (arg.substr(0, 2) == "-n") {
            noiseMultiplier = std::stoi(arg.substr(3));
            noiseMultiplier /= 100;
        }
        
    }

    // handle no output specification
    if (output == "output/") {
        output += "output_enhanced.ppm";
        filetype = ".ppm";
    }

    int width, height, channels;
    unsigned char* img = nullptr;

    // image file loading
    std::string filepath = argv[1];
    if (filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".ppm") {
        img = load_ppm(argv[1], width, height, channels);
    } else {
        img = stbi_load(argv[1], &width, &height, &channels, 0);
    }

    if (img == nullptr) {
        std::cerr << "Error: Could not load the image at " << argv[1] << std::endl;
        return 1;
    }

    

    // start benchmark
    if (benchmark) start = omp_get_wtime();

    // helper
    int pixelAmount = width * height;
    
    // step 1: convert the image to grayscale
    if (verbose) printf("Converting Image to GrayScale...\n");
    unsigned char* imageGrayScale = toGrayScale(img, pixelAmount, channels);

    // step 2: calculate the mean (pixel grayscale average)
    if (verbose) printf("Calculaitng Mean...\n");
    float mean = calculateMean(imageGrayScale, pixelAmount);

    // step 3: calculate the deviation per pixel 
    if (verbose) printf("Calculaitng Deviation per pixel...\n");
    float* deviation = calculateDeviation(imageGrayScale, pixelAmount, mean);

    // step 4: calculate the standard deviation
    if (verbose) printf("Calculaitng Standard Deviation...\n"); 
    float standardDev = calculateStandardDeviation(imageGrayScale, pixelAmount, mean, deviation);
    
    // step 5: calculate the minimum deviation
    if (verbose) printf("Calculaitng Min Deviation...\n");
    float minDev = checkMinDeviation(imageGrayScale, pixelAmount, mean, deviation);
    
    // step 6: calculate the maximum deviation
    if (verbose) printf("Calculaitng Max Deviation...\n");
    float maxDev = checkMaxDeviation(imageGrayScale, pixelAmount, mean, deviation);

    // step 7: calculate the window border of each pixel
    if (verbose) printf("Calculaitng Window Border per pixel...\n");
    int* windowBorder = calculateWindowBorder(width, height, windowSize);

    // step 8: calculate the average grayscale value of all pixels within a window per pixel
    if (verbose) printf("Calculaitng Window Mean per pixel...\n");
    float* windowMean = calculateWindowMean(imageGrayScale, width, height, windowSize, windowBorder);
    
    // step 9: calculate the window's standard deviation per pixel
    if (verbose) printf("Calculaitng Window Standard Deviation per pixel...\n");
    float* windowStandardDev = calculateWindowStandardDeviation(imageGrayScale, width, height, windowSize, windowMean, deviation, windowBorder);

    // step 10: calculate an adaptive deviation per pixel
    if (verbose) printf("Calculaitng Adaptive Deviation per pixel...\n");
    float* adaptiveDev = calculateAdaptiveDeviation(windowStandardDev, pixelAmount, minDev, maxDev);
    
    // step 11: calculate the grayscale value threshold at which a pixel should be fully black / white, influenced by the noiseMultiplier for reducing noise
    if (verbose) printf("Calculaitng Threshold per pixel...\n");
    float* threshold = calculateThreshold(mean, pixelAmount, windowMean, adaptiveDev, windowStandardDev, noiseMultiplier);

    // print some results if verbose
    if (verbose) {
        int halfHeight = height/2;
        int halfWidth = width/2;
        printf("Finished Calculating, Results:\n");
        printf("Width: %d, Height: %d\n", width, height);
        printf("Mean: %f\n", mean);
        printf("Standarddev: %f\n", standardDev);
        printf("minDev: %f\n", minDev);
        printf("maxDev: %f\n", maxDev);
        printf("\nExample pixel-specific values:\n");
        printf("(x,y) (%d,%d) dev: %f\n", halfWidth, halfHeight, deviation[halfHeight * width + halfWidth]);
        printf("(x,y) (%d,%d) window mean: %f\n", halfWidth, halfHeight, windowMean[halfHeight * width + halfWidth]);
        printf("(x,y) (%d,%d) window std. dev: %f\n", halfWidth, halfHeight, windowStandardDev[halfHeight * width + halfWidth]);
        printf("(x,y) (%d,%d) adaptive dev: %f\n", halfWidth, halfHeight, adaptiveDev[halfHeight * width + halfWidth]);
        printf("(x,y) (%d,%d) Threshold: %f\n\n", halfWidth, halfHeight, threshold[halfHeight * width + halfWidth]);
    }

    // step 12: enhance the image by converting pixels to black / white depending on if they hit the grayscale threshold or not 
    unsigned char* enhancedImg = toEnhanced(imageGrayScale, pixelAmount, 1, threshold);

    // end benchmark
    if (benchmark) {
        end = omp_get_wtime();
        printf("Benchmark: %f seconds.\n", end - start);
    }

    // write the enhanced image to a file

    // png:
    if (filetype == ".png") {
        if (stbi_write_png(output.c_str(), width, height, 1, enhancedImg, width) == 0) {
            std::cerr << "Error: Could not write the enhanced image file." << std::endl;
            delete[] enhancedImg;
            if (filepath.substr(filepath.size() - 4) == ".ppm") {
                delete[] img;
            } else {
                stbi_image_free(img);
            }
            return 1;
        }

    // jpg:
    } else if (filetype == ".jpg") {
        if (stbi_write_jpg(output.c_str(), width, height, 1, enhancedImg, width) == 0) {
            std::cerr << "Error: Could not write the enhanced image file." << std::endl;
            delete[] enhancedImg;
            if (filepath.substr(filepath.size() - 4) == ".ppm") {
                delete[] img;
            } else {
                stbi_image_free(img);
            }
            return 1;
        }

    // ppm:
    } else if (filetype == ".ppm") {
        if (writePPM(output.c_str(), width, height, enhancedImg) == 0) {
            std::cerr << "Error: Could not write the enhanced image file." << std::endl;
            delete[] enhancedImg;
            if (filepath.substr(filepath.size() - 4) == ".ppm") {
                delete[] img;
            } else {
                stbi_image_free(img);
            }
            return 1;
        }
    } else {
        std::cerr << "Error: Filetype not supported." << std::endl;
        return 1;
    }

    std::cout << "Enhanced image created successfully: " << output << std::endl;

    // Modified cleanup section
    ALIGNED_FREE(imageGrayScale);
    ALIGNED_FREE(deviation);
    ALIGNED_FREE(windowBorder);
    ALIGNED_FREE(windowMean);
    ALIGNED_FREE(windowStandardDev);
    ALIGNED_FREE(adaptiveDev);
    ALIGNED_FREE(threshold);
    ALIGNED_FREE(enhancedImg);
    
    if (filepath.substr(filepath.size() - 4) == ".ppm") {
        ALIGNED_FREE(img);
    } else {
        stbi_image_free(img);
    }

    return 0;
}