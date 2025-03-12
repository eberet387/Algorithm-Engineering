#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include "stb_image.h"
#include "stb_image_write.h"

// Function to load a PPM image
unsigned char* load_ppm(const char* filename, int& width, int& height, int& channels) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file " << filename << std::endl;
        return nullptr;
    }

    std::string line;
    file >> line; // Read the PPM format identifier (P3)

    if (line != "P3") {
        std::cerr << "Error: Unsupported PPM format (only P3 supported)." << std::endl;
        return nullptr;
    }

    // Skip comments
    while (file.peek() == '#') {
        std::getline(file, line);
    }

    // Read dimensions
    file >> width >> height;

    int max_val;
    file >> max_val;
    if (max_val != 255) {
        std::cerr << "Error: Unsupported max value in PPM (only 255 supported)." << std::endl;
        return nullptr;
    }

    channels = 3; // P3 format has 3 channels (RGB)

    // Allocate memory for the image data
    unsigned char* data = new unsigned char[width * height * channels];

    // Read and verify pixel data
    for (int i = 0; i < width * height * channels; ++i) {
        int value;
        file >> value;
        if (value < 0 || value > 255) {
            std::cerr << "Error: Invalid pixel value in PPM file." << std::endl;
            delete[] data;
            return nullptr;
        }
        data[i] = static_cast<unsigned char>(value);
    }
    file.close();
    return data;
}

void writePPM(const char* filename, int width, int height, unsigned char* data) {
    // Open the output file
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error: Cannot open file " << filename << " for writing." << std::endl;
        return;
    }

    // Write the PPM header
    file << "P3\n";                // P3 format
    file << width << " " << height << "\n"; // Image dimensions
    file << "255\n";              // Maximum color value

    // Write the pixel data
    for (int i = 0; i < height; i += 1) {
        for (int j = 0; j < width; j += 1) {
            int r = static_cast<unsigned char>(data[i * width + j]);
            int g = static_cast<unsigned char>(data[i * width + j]);
            int b = static_cast<unsigned char>(data[i * width + j]);

            file << r << " " << g << " " << b << "   ";
        }
        file << "\n";
    }

    // Close the file
    file.close();
    if (!file) {
        std::cerr << "Error: Failed to write to file " << filename << std::endl;
    } else {
        std::cout << "PPM file written successfully: " << filename << std::endl;
    }
}

// Function to convert an image to grayscale
unsigned char* toGrayScale(unsigned char* img, int width, int height, int channels) {
    unsigned char* imageGrayScale = new unsigned char[width * height]; // 1 channel -> grayscale

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int pixel_index = (i * width + j) * channels;

            // Get RGB values
            unsigned char r = img[pixel_index];
            unsigned char g = img[pixel_index + 1];
            unsigned char b = img[pixel_index + 2];

            // Calculate grayscale value (simple average)
            unsigned char grayscale = (r + g + b) / 3;
            imageGrayScale[i * width + j] = grayscale; // Assign to 1-channel array

        }
    }
    return imageGrayScale;
}

unsigned char* toEnhanced(unsigned char* img, unsigned char* image_greyscale, int width, int height, int channels, float** threshold) {

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            img[i + j * width] = image_greyscale[i + j * width] < threshold[i][j] ? 0 : 255;
        }
    }

    return img;
}

float calculateMean(const unsigned char* img, int width, int height) {
    long long sum = 0;
    for (int i = 0; i < width * height; ++i) {
        sum += img[i];
    }

    return (sum / (width * height));
}

float calculateStandardDeviation(const unsigned char* img, int width, int height, float** devs) {
    float mean = calculateMean(img, width, height);
    long long sum_squared_diff = 0.0;

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            devs[i][j] = std::abs(img[i * height + j] - mean);
            sum_squared_diff += devs[i][j] * devs[i][j];
        }
    }
    
    return sum_squared_diff / (width * height);
}

float calculateMinDeviation(const unsigned char* img, int width, int height, float mean, float** devs) {

    int min_deviation = 255; // Maximum possible deviation

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            min_deviation = std::min(min_deviation, (int)devs[i][j]);
        }
    }

    return min_deviation;
}

float calculateMaxDeviation(const unsigned char* img, int width, int height, float mean, float** devs) {

    int max_deviation = 0;   // Minimum possible deviation

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            max_deviation = std::max(max_deviation, (int)devs[i][j]);
        }
    }

    return max_deviation;
}

float** calculateWindowMean(const unsigned char* img, int width, int height, int windowSize, float** windowMean) {
    int halfWindowSize = windowSize/2;
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            long long sum = 0;
            int count = 0;
            int startY = std::max(0, j - halfWindowSize);
            int startX = std::max(0, i - halfWindowSize);
            int endY = std::min(j + halfWindowSize, height);
            int endX = std::min(i + halfWindowSize, width);

            for (int y = startY; y < endY; ++y) {
                for (int x = startX; x < endX; ++x) {
                    sum += img[y * width + x]; 
                    count++;
                }
            }
            windowMean[i][j] = sum / count;    
        }
    }
    
    return windowMean;
}

float** calculateWindowStandardDeviation(const unsigned char* img, int width, int height, int windowSize, float** windowMean, float** windowDev, float** devs) {

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            float sum_squared_diff = 0.0;
            int count = 0;
            int halfWindowSize = windowSize/2;
            int startY = std::max(0, j - halfWindowSize);
            int startX = std::max(0, i - halfWindowSize);
            int endY = std::min(j + halfWindowSize, height);
            int endX = std::min(i + halfWindowSize, width);

            for (int y = startY; y < endY; ++y) {
                for (int x = startX; x < endX; ++x) {
                    float diff = devs[x][y] - windowMean[x][y];
                    sum_squared_diff += diff * diff;
                    ++count;
                }
            }
            windowDev[i][j] = std::sqrt(sum_squared_diff / count);
        }
    }
    
    return windowDev;
}

float** calculateAdaptiveDeviation(float** windowDev, int width, int height, float minDev, float maxDev, float** adaptiveDev) {
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            adaptiveDev[i][j] = (windowDev[i][j] - minDev) * (maxDev - minDev);
        }
    }
    return adaptiveDev;
}

float** calculateThreshold(float mean, int width, int height, float** meanWindow, float** adaptiveDeviation, float** windowDeviation, float** threshold) {
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            threshold[i][j] = meanWindow[i][j] - (meanWindow[i][j] * meanWindow[i][j] - windowDeviation[i][j]) / ((mean + windowDeviation[i][j]) * (adaptiveDeviation[i][j] + windowDeviation[i][j]));
        }
    }
    return threshold;
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return 1;
    }
    std::string output = "output/";
    std::string filetype;
    bool verbose = false;
    int windowSize = 20;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
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
            windowSize = stoi(arg.substr(3));
        }
        
    }

    if (output == "output/") {
        output += "output_enhanced.ppm";
        filetype = ".ppm";
    }

    int width, height, channels;
    unsigned char* img = nullptr;

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

    // Convert image to grayscale
    unsigned char* imageGrayScale = toGrayScale(img, width, height, channels);
    
    // Helpers
    float mean = calculateMean(imageGrayScale, width, height);

    // dynamic 2d array allocation
    float** devs = new float*[width];
    float** windowMean = new float*[width];
    float** windowDev = new float*[width];
    float** adaptiveDev = new float*[width];
    float** threshold = new float*[width];
    for (int i = 0; i < width; i++) {
        devs[i] = new float[height];
        windowMean[i] = new float[height];
        windowDev[i] = new float[height];
        adaptiveDev[i] = new float[height];
        threshold[i] = new float[height];
    }
    
    float standardDev = calculateStandardDeviation(imageGrayScale, width, height, devs);
    float minDev = calculateMinDeviation(imageGrayScale, width, height, mean, devs);
    float maxDev = calculateMaxDeviation(imageGrayScale, width, height, mean, devs);
    windowMean = calculateWindowMean(imageGrayScale, width, height, windowSize, windowMean);
    windowDev = calculateWindowStandardDeviation(imageGrayScale, width, height, windowSize, windowMean, windowDev, devs);
    adaptiveDev = calculateAdaptiveDeviation(windowDev, width, height, minDev, maxDev, adaptiveDev);
    threshold = calculateThreshold(mean, width, height, windowMean, adaptiveDev, windowDev, threshold);

    if (verbose) {
        printf("Width: %d, Height: %d\n", width, height);
        printf("Mean: %f\n", mean);
        printf("Standarddev: %f\n", standardDev);
        printf("minDev: %f\n", minDev);
        printf("maxDev: %f\n", maxDev);
        printf("\nExample pixel-specific values:\n");
        printf("(x,y) (%d,%d) dev: %f\n", width/2, height/2, devs[width/2][height/2]);
        printf("(x,y) (%d,%d) window mean: %f\n", width/2, height/2, windowMean[width/2][height/2]);
        printf("(x,y) (%d,%d) window std. dev: %f\n", width/2, height/2, windowDev[width/2][height/2]);
        printf("(x,y) (%d,%d) adaptive dev: %f\n", width/2, height/2, adaptiveDev[width/2][height/2]);
        printf("(x,y) (%d,%d) Threshold: %f\n\n", width/2, height/2, threshold[width/2][height/2]);
    }
    

    img = toEnhanced(img, imageGrayScale, width, height, 1, threshold);
    
    // Write grayscale image to a file
    // if (stbi_write_png(output.c_str(), width, height, 1, imageGrayScale, width) == 0) {
    //     std::cerr << "Error: Could not write the grayscale image file." << std::endl;
    //     delete[] imageGrayScale;
    //     if (filepath.substr(filepath.size() - 4) == ".ppm") {
    //         delete[] img;
    //     } else {
    //         stbi_image_free(img);
    //     }
    //     return 1;
    // }

    // std::cout << "Grayscale image created successfully: " << output << std::endl;

    // Write grayscale image to a file

    if (filetype == ".png") {
        if (stbi_write_png(output.c_str(), width, height, 1, img, width) == 0) {
            std::cerr << "Error: Could not write the enhanced image file." << std::endl;
            delete[] img;
            if (filepath.substr(filepath.size() - 4) == ".ppm") {
                delete[] img;
            } else {
                stbi_image_free(img);
            }
            return 1;
        }
    } else if (filetype == ".jpg") {
        if (stbi_write_jpg(output.c_str(), width, height, 1, img, width) == 0) {
            std::cerr << "Error: Could not write the enhanced image file." << std::endl;
            delete[] img;
            if (filepath.substr(filepath.size() - 4) == ".ppm") {
                delete[] img;
            } else {
                stbi_image_free(img);
            }
            return 1;
        }
    } else if (filetype == ".ppm") {
        writePPM(output.c_str(), width, height, img);
    }


    std::cout << "Enhanced image created successfully: " << output << std::endl;

    // Free allocated memory
    delete[] imageGrayScale;
    if (filepath.substr(filepath.size() - 4) == ".ppm") {
        delete[] img;
    } else {
        stbi_image_free(img);
    }

    return 0;
}
