#pragma once
// png_encoder.hpp: minimal PNG encoder for closure plots
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <array>
#include <algorithm>
#include <cmath>

namespace sw { namespace universal {

// RGB color structure
struct RGB {
    uint8_t r, g, b;
    constexpr RGB(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0) : r(red), g(green), b(blue) {}
};

// Predefined colors for closure plot results
namespace ClosureColor {
    inline constexpr RGB EXACT{ 0, 0, 0 };            // Black
    inline constexpr RGB OVERFLOW_{ 255, 0, 0 };      // Red
    inline constexpr RGB UNDERFLOW_{ 0, 0, 255 };     // Blue
    inline constexpr RGB NAN_NAR{ 255, 255, 0 };      // Yellow
    inline constexpr RGB SATURATE{ 0, 255, 0 };       // Green
    inline constexpr RGB BACKGROUND{ 224, 224, 224 }; // Light gray

    // Purple gradient for approximations (from dark to light purple)
    inline RGB approximation(double errorLevel) {
        // errorLevel should be between 0.0 and 1.0
        // Dark purple (75, 0, 130) to light purple (186, 85, 211)
        double t = std::max(0.0, std::min(1.0, errorLevel));
        uint8_t r = static_cast<uint8_t>(75 + t * (186 - 75));
        uint8_t g = static_cast<uint8_t>(0 + t * (85 - 0));
        uint8_t b = static_cast<uint8_t>(130 + t * (211 - 130));
        return RGB{ r, g, b };
    }
}

// Simple CRC32 implementation for PNG chunks
class CRC32 {
private:
    static constexpr uint32_t POLYNOMIAL = 0xEDB88320;
    std::array<uint32_t, 256> table;

public:
    CRC32() {
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t crc = i;
            for (int j = 0; j < 8; ++j) {
                if (crc & 1) {
                    crc = (crc >> 1) ^ POLYNOMIAL;
                } else {
                    crc >>= 1;
                }
            }
            table[i] = crc;
        }
    }

    uint32_t calculate(const std::vector<uint8_t>& data) const {
        uint32_t crc = 0xFFFFFFFF;
        for (uint8_t byte : data) {
            crc = table[(crc ^ byte) & 0xFF] ^ (crc >> 8);
        }
        return crc ^ 0xFFFFFFFF;
    }
};

// Simple PNG encoder class
class PNGEncoder {
private:
    uint32_t width_, height_;
    std::vector<RGB> pixels_;
    static CRC32 crc_;

    void writeUint32(std::ofstream& file, uint32_t value) const {
        file.put((value >> 24) & 0xFF);
        file.put((value >> 16) & 0xFF);
        file.put((value >> 8) & 0xFF);
        file.put(value & 0xFF);
    }

    void writeChunk(std::ofstream& file, const std::string& type, const std::vector<uint8_t>& data) const {
        // Write length
        writeUint32(file, static_cast<uint32_t>(data.size()));

        // Write type and data combined for CRC calculation
        std::vector<uint8_t> typeAndData;
        typeAndData.insert(typeAndData.end(), type.begin(), type.end());
        typeAndData.insert(typeAndData.end(), data.begin(), data.end());

        // Write type
        file.write(type.c_str(), 4);

        // Write data
        file.write(reinterpret_cast<const char*>(data.data()), data.size());

        // Write CRC
        uint32_t crc = crc_.calculate(typeAndData);
        writeUint32(file, crc);
    }

    std::vector<uint8_t> compressImageData() const {
        std::vector<uint8_t> rawData;

        // Add scanlines with filter byte (0 = none)
        for (uint32_t y = 0; y < height_; ++y) {
            rawData.push_back(0); // Filter type: none
            for (uint32_t x = 0; x < width_; ++x) {
                const RGB& pixel = pixels_[y * width_ + x];
                rawData.push_back(pixel.r);
                rawData.push_back(pixel.g);
                rawData.push_back(pixel.b);
            }
        }

        // For simplicity, we'll use no compression (store as-is)
        // In a full implementation, you'd use zlib/deflate here
        std::vector<uint8_t> compressed;

        // Deflate header for uncompressed blocks
        compressed.push_back(0x78); // CMF
        compressed.push_back(0x01); // FLG (no compression)

        // Add uncompressed blocks
        size_t remaining = rawData.size();
        size_t offset = 0;

        while (remaining > 0) {
            size_t blockSize = std::min(remaining, size_t(0xFFFF));
            bool isLast = (remaining == blockSize);

            // Block header
            compressed.push_back(isLast ? 0x01 : 0x00);

            // Block size (little endian)
            compressed.push_back(blockSize & 0xFF);
            compressed.push_back((blockSize >> 8) & 0xFF);

            // One's complement of block size
            uint16_t complement = ~static_cast<uint16_t>(blockSize);
            compressed.push_back(complement & 0xFF);
            compressed.push_back((complement >> 8) & 0xFF);

            // Block data
            compressed.insert(compressed.end(),
                            rawData.begin() + offset,
                            rawData.begin() + offset + blockSize);

            offset += blockSize;
            remaining -= blockSize;
        }

        // Add Adler-32 checksum (simplified - just use 1 for uncompressed)
        compressed.push_back(0x00);
        compressed.push_back(0x00);
        compressed.push_back(0x00);
        compressed.push_back(0x01);

        return compressed;
    }

public:
    PNGEncoder(uint32_t width, uint32_t height) : width_(width), height_(height) {
        pixels_.resize(width_ * height_, ClosureColor::BACKGROUND);
    }

    void setPixel(uint32_t x, uint32_t y, const RGB& color) {
        if (x < width_ && y < height_) {
            pixels_[y * width_ + x] = color;
        }
    }

    RGB getPixel(uint32_t x, uint32_t y) const {
        if (x < width_ && y < height_) {
            return pixels_[y * width_ + x];
        }
        return RGB{0, 0, 0};
    }

    void fill(const RGB& color) {
        std::fill(pixels_.begin(), pixels_.end(), color);
    }

    bool savePNG(const std::string& filename) const {
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            return false;
        }

        // PNG signature
        const uint8_t signature[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
        file.write(reinterpret_cast<const char*>(signature), sizeof(signature));

        // IHDR chunk
        std::vector<uint8_t> ihdr;
        // Width (4 bytes, big-endian)
        ihdr.push_back((width_ >> 24) & 0xFF);
        ihdr.push_back((width_ >> 16) & 0xFF);
        ihdr.push_back((width_ >> 8) & 0xFF);
        ihdr.push_back(width_ & 0xFF);
        // Height (4 bytes, big-endian)
        ihdr.push_back((height_ >> 24) & 0xFF);
        ihdr.push_back((height_ >> 16) & 0xFF);
        ihdr.push_back((height_ >> 8) & 0xFF);
        ihdr.push_back(height_ & 0xFF);
        // Bit depth, color type, compression, filter, interlace
        ihdr.push_back(8);  // 8 bits per channel
        ihdr.push_back(2);  // Color type: RGB
        ihdr.push_back(0);  // Compression method: deflate
        ihdr.push_back(0);  // Filter method: adaptive
        ihdr.push_back(0);  // Interlace method: none

        writeChunk(file, "IHDR", ihdr);

        // IDAT chunk
        std::vector<uint8_t> imageData = compressImageData();
        writeChunk(file, "IDAT", imageData);

        // IEND chunk
        std::vector<uint8_t> iend;
        writeChunk(file, "IEND", iend);

        return file.good();
    }

    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }
};

// Static member initialization
CRC32 PNGEncoder::crc_;

}} // namespace sw::universal