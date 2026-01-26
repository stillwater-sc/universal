// ulp_visual_demo.cpp: demonstrate visual impact of removing ULP bits from image pixels
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This demo loads an RGB image and progressively removes the least significant bits
// from each color channel, saving the results to show that lower bits often have
// no perceptible visual impact - a key insight for mixed-precision image processing.

#define STB_IMAGE_IMPLEMENTATION
#include <image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <image/stb_image_write.h>

`#include` <iostream>
`#include` <iomanip>
`#include` <string>
`#include` <vector>
`#include` <cstring>
`#include` <cmath>
`#include` <limits>
`#include` <filesystem>
// Calculate PSNR (Peak Signal-to-Noise Ratio) between two images
double calculatePSNR(const unsigned char* original, const unsigned char* modified, int width, int height, int channels) {
    double mse = 0.0;
    size_t totalPixels = static_cast<size_t>(width) * height * channels;

    for (size_t i = 0; i < totalPixels; ++i) {
        double diff = static_cast<double>(original[i]) - static_cast<double>(modified[i]);
        mse += diff * diff;
    }
    mse /= totalPixels;

    if (mse == 0.0) return std::numeric_limits<double>::infinity(); // Identical images

    double maxPixelValue = 255.0;
    return 10.0 * std::log10((maxPixelValue * maxPixelValue) / mse);
}

// Calculate percentage of pixels that changed
double calculateChangedPixels(const unsigned char* original, const unsigned char* modified, int width, int height, int channels) {
    size_t totalPixels = static_cast<size_t>(width) * height * channels;
    size_t changedCount = 0;

    for (size_t i = 0; i < totalPixels; ++i) {
        if (original[i] != modified[i]) ++changedCount;
    }

    return 100.0 * changedCount / totalPixels;
}

// Strip n least significant bits from each pixel
void stripLSBs(const unsigned char* input, unsigned char* output, int width, int height, int channels, int bitsToStrip) {
    size_t totalPixels = static_cast<size_t>(width) * height * channels;
    unsigned char mask = static_cast<unsigned char>(0xFF << bitsToStrip);

    for (size_t i = 0; i < totalPixels; ++i) {
        output[i] = input[i] & mask;
    }
}

// Get output filename with bit count
std::string getOutputFilename(const std::string& inputPath, int bitsStripped) {
    size_t lastDot = inputPath.find_last_of('.');
    size_t lastSlash = inputPath.find_last_of("/\\");

    std::string basename;
    if (lastSlash != std::string::npos) {
        basename = inputPath.substr(lastSlash + 1);
    } else {
        basename = inputPath;
    }

    lastDot = basename.find_last_of('.');
    std::string name = (lastDot != std::string::npos) ? basename.substr(0, lastDot) : basename;

    return name + "_" + std::to_string(8 - bitsStripped) + "bit.png";
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <input_image> [output_directory]\n\n";
    std::cout << "Demonstrates the visual impact of reducing precision in image pixels.\n";
    std::cout << "Loads an RGB image and creates versions with progressively fewer bits\n";
    std::cout << "of precision per channel (8-bit down to 1-bit).\n\n";
    std::cout << "Arguments:\n";
    std::cout << "  input_image      Path to input image (PNG, JPEG, BMP, etc.)\n";
    std::cout << "  output_directory Optional: directory for output images (default: current)\n\n";
    std::cout << "Output:\n";
    std::cout << "  Creates <name>_8bit.png through <name>_1bit.png showing the effect\n";
    std::cout << "  of keeping only N bits of precision per color channel.\n\n";
    std::cout << "Example:\n";
    std::cout << "  " << programName << " photo.jpg results/\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputDir = (argc > 2) ? argv[2] : ".";

    // Ensure output directory ends with separator
    if (!outputDir.empty() && outputDir.back() != '/' && outputDir.back() != '\\') {
        outputDir += '/';
    }

    // Create output directory if it doesn't exist
    if (outputDir != "./" && outputDir != ".") {
        std::filesystem::path outPath(outputDir);
        if (!std::filesystem::exists(outPath)) {
            std::error_code ec;
            if (!std::filesystem::create_directories(outPath, ec)) {
                std::cerr << "Error: Could not create output directory '" << outputDir << "'\n";
                if (ec) std::cerr << "Reason: " << ec.message() << "\n";
                return 1;
            }
            std::cout << "Created output directory: " << outputDir << "\n\n";
        }
    }

    // Load the input image
    int width, height, channels;
    unsigned char* originalImage = stbi_load(inputPath.c_str(), &width, &height, &channels, 3);

    if (!originalImage) {
        std::cerr << "Error: Could not load image '" << inputPath << "'\n";
        std::cerr << "Reason: " << stbi_failure_reason() << "\n";
        return 1;
    }

    // Force 3 channels (RGB)
    channels = 3;

    std::cout << "ULP Visual Precision Demo\n";
    std::cout << "=========================\n\n";
    std::cout << "Input image: " << inputPath << "\n";
    std::cout << "Dimensions:  " << width << " x " << height << " pixels\n";
    std::cout << "Channels:    " << channels << " (RGB)\n";
    std::cout << "Total bytes: " << (width * height * channels) << "\n\n";

    // Allocate buffer for modified image
    std::vector<unsigned char> modifiedImage(width * height * channels);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Bits | Mask     | PSNR (dB)  | Changed (%) | Output File\n";
    std::cout << "-----|----------|------------|-------------|---------------------------\n";

    // Process each bit level (8 bits down to 1 bit)
    for (int bitsKept = 8; bitsKept >= 1; --bitsKept) {
        int bitsStripped = 8 - bitsKept;
        unsigned char mask = static_cast<unsigned char>(0xFF << bitsStripped);

        // Strip the LSBs
        stripLSBs(originalImage, modifiedImage.data(), width, height, channels, bitsStripped);

        // Calculate quality metrics
        double psnr = calculatePSNR(originalImage, modifiedImage.data(), width, height, channels);
        double changedPct = calculateChangedPixels(originalImage, modifiedImage.data(), width, height, channels);

        // Generate output filename and save
        std::string outputFile = outputDir + getOutputFilename(inputPath, bitsStripped);

        int success = stbi_write_png(outputFile.c_str(), width, height, channels, modifiedImage.data(), width * channels);

        std::cout << "  " << bitsKept << "  | 0x" << std::hex << std::setfill('0') << std::setw(2)
                  << static_cast<int>(mask) << std::dec << std::setfill(' ')
                  << "   | ";

        if (std::isinf(psnr)) {
            std::cout << "  Inf      ";
        } else {
            std::cout << std::setw(8) << psnr << "  ";
        }

        std::cout << "| " << std::setw(9) << changedPct << "   | ";

        if (success) {
            std::cout << outputFile << "\n";
        } else {
            std::cout << "FAILED TO WRITE\n";
        }
    }

    std::cout << "\nKey Insights:\n";
    std::cout << "-------------\n";
    std::cout << "- PSNR > 40 dB: Differences typically imperceptible to human vision\n";
    std::cout << "- PSNR 30-40 dB: Minor differences, may be noticeable in careful comparison\n";
    std::cout << "- PSNR < 30 dB: Differences clearly visible\n\n";
    std::cout << "For most natural images:\n";
    std::cout << "- 6-bit precision (2 LSBs stripped): Usually indistinguishable from 8-bit\n";
    std::cout << "- 5-bit precision (3 LSBs stripped): Minor banding may appear in gradients\n";
    std::cout << "- 4-bit precision (4 LSBs stripped): Visible posterization in smooth areas\n\n";
    std::cout << "This demonstrates that the lower 2-3 bits of 8-bit pixel data often\n";
    std::cout << "contain noise or imperceptible detail, validating mixed-precision\n";
    std::cout << "approaches in image processing pipelines.\n";

    // Free the original image
    stbi_image_free(originalImage);

    return 0;
}
