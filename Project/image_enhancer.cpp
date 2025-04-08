#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION


#include "stb_image.h"
#include "stb_image_write.h"
#include "implementation.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <cmath>
#include <algorithm>
#include <omp.h>

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
    float noiseMultiplier = 0.9;

    // argument logic
    // argument list:
    //  -o="output_path.filetype"
    //  -v -> verbose
    //  -w=windowSize (int)
    //  -b -> benchmark mode
    //  -n=noiseMultiplier (int) 0-100, however interesting results can be achieved if the noiseMultiplier exceeds 100.
    //  -h -> help
    //  -s -> serial version

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (i == 1 && arg.substr(0, 2) == "-h") {
            printf("This program helps enhance images for printing / other use.\nUsage: <image_path> {arguments}\n\nArgument List:\n-v   Verbose Mode\n-b   Benchmark Mode\n-s  Serial Implementation\n-o=\"filepath.filetype\" -> specify the output\n-w=windowSize -> specify the window size of nearest pixels that influence the adaptive thresholds (standard: 20)\n-n=noiseMultiplier -> specify a number between 0 and 100 - lower number means less noise\n");
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
        } else if (arg.substr(0, 2) == "-s") {
            omp_set_num_threads(1);
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
    //float* windowMean = calculateWindowMean(imageGrayScale, width, height, windowSize, windowBorder);
    float* windowMean = fastSpatialAveraging(imageGrayScale, width, height, windowSize);
    
    // step 9: calculate the window's standard deviation per pixel
    if (verbose) printf("Calculaitng Window Standard Deviation per pixel...\n");
    float* squaredDiff = calculateWindowSquaredDiff(width, height, windowMean, deviation);
    float* windowStandardDev = calculateWindowStandardDeviation(imageGrayScale, width, height, windowSize, squaredDiff, windowBorder);

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