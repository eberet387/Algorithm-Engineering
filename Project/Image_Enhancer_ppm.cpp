#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <vector>
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

    return data;
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

double calculateMean(const unsigned char* img, int width, int height) {
    long long sum = 0;
    for (int i = 0; i < width * height; ++i) {
        sum += img[i];
    }

    return (sum / (width * height));
}

double calculateStandardDeviation(const unsigned char* img, int width, int height) {
    double mean = calculateMean(img, width, height);
    double sum_squared_diff = 0.0;

    for (int i = 0; i < width * height; ++i) {
        double diff = img[i] - mean;
        sum_squared_diff += diff * diff;
    }

    return std::sqrt(sum_squared_diff / (width * height));
}

double calculateMinDeviation(const unsigned char* img, int width, int height, double mean) {

    int min_deviation = 255; // Maximum possible deviation

    for (int i = 0; i < width * height; ++i) {
        int deviation = std::abs(img[i] - mean);
        min_deviation = std::min(min_deviation, deviation);
    }

    return min_deviation;
}

double calculateMaxDeviation(const unsigned char* img, int width, int height, double mean) {

    int max_deviation = 0;   // Minimum possible deviation

    for (int i = 0; i < width * height; ++i) {
        int deviation = std::abs(img[i] - mean);
        max_deviation = std::max(max_deviation, deviation);
    }

    return max_deviation;
}

double calculateWindowMean(const unsigned char* img, int width, int height, int x, int y, int windowSize) {
    long long sum = 0;
    int count = 0;
    int halfWindowSize = windowSize/2;

    for (int i = y - halfWindowSize; i < y + halfWindowSize && i < height; ++i) {
        for (int j = x - halfWindowSize; j < x + halfWindowSize && j < width; ++j) {
            sum += img[i * width + j];
            count++;
        }
    }

    return sum / count;
}

double calculateWindowStandardDeviation(const unsigned char* img, int width, int height, int x, int y, int windowSize) {
    double mean = calculateWindowMean(img, width, height, x, y, windowSize);
    double sum_squared_diff = 0.0;
    int count = 0;
    int halfWindowSize = windowSize/2;

    for (int i = y - halfWindowSize; i < y + halfWindowSize && i < height; ++i) {
        for (int j = x - halfWindowSize; j < x + halfWindowSize && j < width; ++j) {
            double diff = img[i * width + j] - mean;
            sum_squared_diff += diff * diff;
            ++count;
        }
    }

    return std::sqrt(sum_squared_diff / count);
}

double calculateAdaptiveDeviation(double windowDev, double minDev, double maxDev) {
    return (windowDev - minDev) * (maxDev - minDev);
}

double calculateThreshold(double mean, double meanWindow, double adaptiveDeviation, double windowDeviation) {
    return meanWindow - (meanWindow * meanWindow - windowDeviation) / ((mean + windowDeviation) * (adaptiveDeviation + windowDeviation));
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return 1;
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
    double mean = calculateMean(imageGrayScale, width, height);
    double standardDev = calculateStandardDeviation(imageGrayScale, width, height);
    double minDev = calculateMinDeviation(imageGrayScale, width, height, mean);
    double maxDev = calculateMaxDeviation(imageGrayScale, width, height, mean);
    int windowSize = 20;
    double testWindowMean = calculateWindowMean(imageGrayScale, width, height, 30, 30, windowSize);
    double testWindowDev = calculateWindowStandardDeviation(imageGrayScale, width, height, 30, 30, windowSize);
    double testAdaptiveDev = calculateAdaptiveDeviation(testWindowDev, minDev, maxDev);
    double testThreshold = calculateThreshold(mean, testWindowMean, testAdaptiveDev, testWindowDev);

    printf("Width: %d, Height: %d\n", width, height);
    printf("Mean: %f\n", mean);
    printf("Standarddev: %f\n", standardDev);
    printf("minDev: %f\n", minDev);
    printf("maxDev: %f\n", maxDev);
    printf("(x,y) (30,30) window mean: %f\n", testWindowMean);
    printf("(x,y) (30,30) window std. dev: %f\n", testWindowDev);
    printf("(x,y) (30,30) adaptive dev: %f\n", testAdaptiveDev);
    printf("(x,y) (30,30) Threshold: %f\n", testThreshold);

    // Write grayscale image to a file
    if (stbi_write_png("output/outputGrayScale.png", width, height, 1, imageGrayScale, width) == 0) {
        std::cerr << "Error: Could not write the grayscale image file." << std::endl;
        delete[] imageGrayScale;
        if (filepath.substr(filepath.size() - 4) == ".ppm") {
            delete[] img;
        } else {
            stbi_image_free(img);
        }
        return 1;
    }

    std::cout << "Grayscale image created successfully: outputGrayScale.png" << std::endl;

    // Free allocated memory
    delete[] imageGrayScale;
    if (filepath.substr(filepath.size() - 4) == ".ppm") {
        delete[] img;
    } else {
        stbi_image_free(img);
    }

    return 0;
}
