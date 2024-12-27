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
