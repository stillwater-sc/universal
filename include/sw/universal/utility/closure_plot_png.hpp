#pragma once
// closure_plot_png.hpp: PNG generation for closure plots
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <vector>
#include <string>
#include <map>
#include <filesystem>
#include <cmath>
#include <iostream>
#include <execution>
#include <algorithm>
#include <universal/utility/png_encoder.hpp>
#include <universal/utility/error.hpp>

namespace sw { namespace universal {

// Closure result types for color coding
enum class ClosureResult {
    EXACT,
    APPROXIMATION,
	OVERFLOW_,     // avoid a name clash in MSVC++
	UNDERFLOW_,    // avoid a name clash in MSVC++
    NAN_NAR,
    SATURATE
};

std::ostream& operator<<(std::ostream& os, ClosureResult result) {
    switch (result) {
        case ClosureResult::EXACT:         os << "EXACT"; break;
        case ClosureResult::APPROXIMATION: os << "APPROXIMATION"; break;
        case ClosureResult::OVERFLOW_:     os << "OVERFLOW"; break;
        case ClosureResult::UNDERFLOW_:    os << "UNDERFLOW"; break;
        case ClosureResult::NAN_NAR:       os << "NAN_NAR"; break;
        case ClosureResult::SATURATE:      os << "SATURATE"; break;
        default:                           os << "UNKNOWN"; break;
    }
    return os;
}

// Mapping modes for closure plots
enum class MappingMode {
    ENCODING_DIRECT,     // Original: direct encoding, direct coordinates
    VALUE_CENTERED       // Mathematical: value-based encoding, zero-centered coordinates
};

// Structure to hold closure plot data for a single operation
template<typename NumberType>
struct ClosureData {
    std::vector<std::vector<ClosureResult>> results;
    std::vector<std::vector<uint8_t>> errorLevels;  // 0-255 instead of double
    uint32_t size;

    ClosureData(uint32_t operandCount) : size(operandCount) {
        results.resize(size, std::vector<ClosureResult>(size));
        errorLevels.resize(size, std::vector<uint8_t>(size, 0));
    }
};

// PNG Closure Plot Generator
template<typename NumberType>
class ClosurePlotPNG {
private:
    static const unsigned NBITS = NumberType::nbits;
    static const unsigned NR_ENCODINGS = (1u << NBITS);

    // Sampling configuration to limit output size
    // Use 2500x2500 to avoid power-of-2 aliasing with floating-point structure
    // Only sample when nbits >= 12 (4096 encodings)
    static constexpr unsigned MAX_PLOT_SIZE = 2500;
    static constexpr unsigned SAMPLING_THRESHOLD = 4096;  // 2^12
    static constexpr bool needsSampling = (NR_ENCODINGS >= SAMPLING_THRESHOLD);
    static constexpr unsigned defaultPlotSize = needsSampling ? MAX_PLOT_SIZE : NR_ENCODINGS;

    MappingMode mappingMode = MappingMode::ENCODING_DIRECT;  // Default to original behavior
    bool enableSampling = true;  // Enable sampling by default for large configurations
    unsigned plotSize;
    double sampleStride;
    mutable std::vector<unsigned> valueBasedEncodingMap;    // Cached value-to-encoding mapping

    // Helper function to map pixel coordinate to encoding based on mapping mode
    unsigned getEncodingForPixel(unsigned pixelCoord) const {
        // Apply sampling: map pixel coordinate to actual encoding
        // Use non-integer stride to avoid aliasing with power-of-2 structure
        unsigned sampledCoord = static_cast<unsigned>(pixelCoord * sampleStride);

        // Clamp to valid encoding range to prevent out-of-bounds access
        if (sampledCoord >= NR_ENCODINGS) {
            sampledCoord = NR_ENCODINGS - 1;
        }

        if (mappingMode == MappingMode::ENCODING_DIRECT) {
            // Original behavior: direct pixel to encoding mapping
            return sampledCoord;
        } else {
            // VALUE_CENTERED: use value-based encoding
            return getValueBasedEncoding(sampledCoord);
        }
    }

    // Create value-based encoding map (one-time setup, cached)
    void initializeValueBasedEncodingMap() const {
        if (!valueBasedEncodingMap.empty()) return;

        std::vector<std::pair<double, unsigned>> valueEncodingPairs;
        valueEncodingPairs.reserve(NR_ENCODINGS);

        // Sample all encodings and their actual values
        for (unsigned encoding = 0; encoding < NR_ENCODINGS; ++encoding) {
            NumberType temp;
            temp.setbits(encoding);
            double value = double(temp);
            valueEncodingPairs.emplace_back(value, encoding);
        }

        // Sort by actual numerical value: maxneg → ... → zero → ... → maxpos
        std::sort(valueEncodingPairs.begin(), valueEncodingPairs.end(),
                 [](const auto& a, const auto& b) {
                     // Handle NaN/NaR values - put them at the end
                     if (std::isnan(a.first) && std::isnan(b.first)) return false;
                     if (std::isnan(a.first)) return false; // NaN goes to end
                     if (std::isnan(b.first)) return true;
                     return a.first < b.first;
                 });

        // Extract encodings in value order
        valueBasedEncodingMap.reserve(NR_ENCODINGS);
        for (const auto& pair : valueEncodingPairs) {
            valueBasedEncodingMap.push_back(pair.second);
        }
    }

    const std::vector<unsigned>& getValueBasedEncodingMap() const {
        return valueBasedEncodingMap;
    }

    // Get encoding for mathematical value-based ordering
    unsigned getValueBasedEncoding(unsigned pixelCoord) const {
        const auto& encodingMap = getValueBasedEncodingMap();
        return encodingMap[pixelCoord];
    }

    // Helper function to get color for a closure result
    RGB getResultColor(ClosureResult result, double errorLevel = 0.0) const {
        switch (result) {
            case ClosureResult::EXACT:
                return ClosureColor::EXACT;
            case ClosureResult::APPROXIMATION:
                return ClosureColor::approximation(errorLevel);
            case ClosureResult::OVERFLOW_:
                return ClosureColor::OVERFLOW_;
            case ClosureResult::UNDERFLOW_:
                return ClosureColor::UNDERFLOW_;
            case ClosureResult::NAN_NAR:
                return ClosureColor::NAN_NAR;
            case ClosureResult::SATURATE:
                return ClosureColor::SATURATE;
            default:
                return ClosureColor::BACKGROUND;
        }
    }

    // Classify arithmetic result
    template<bool MapSaturation = false>
    ClosureResult classifyResult(NumberType /* va */, NumberType /* vb */, NumberType result,
                               double targetValue, double& normalizedError) const {
        normalizedError = 0.0;

        // Handle special values using uniform ADL interface
        if (sw::universal::isnan(result)) {
            return ClosureResult::NAN_NAR;
        }
        else if (sw::universal::isinf(result)) {
            return ClosureResult::OVERFLOW_;
        }
        else if (sw::universal::iszero(result)) {
            if (targetValue == 0.0) {
                return ClosureResult::EXACT;
            } else {
                return ClosureResult::UNDERFLOW_;
            }
		}
        else if (!sw::universal::isnormal(result) && result != NumberType(0)) {
            return ClosureResult::UNDERFLOW_;
        }

        double resultValue = double(result);
        if (targetValue == resultValue) {
            return ClosureResult::EXACT;
        }

        // For non-exact results, check for basic overflow/underflow conditions
        NumberType maxpos(sw::universal::SpecificValue::maxpos);
        NumberType maxneg(sw::universal::SpecificValue::maxneg);
        NumberType minpos(sw::universal::SpecificValue::minpos);
        NumberType minneg(sw::universal::SpecificValue::minneg);
        double dmaxpos = double(maxpos);
        double dmaxneg = double(maxneg);
        double dminpos = double(minpos);
        double dminneg = double(minneg);

        // Simple overflow/underflow detection
        if ((targetValue > dmaxpos) || (targetValue < dmaxneg)) {
            if constexpr (MapSaturation) {
                if (result == maxpos || result == maxneg) {
                    return ClosureResult::SATURATE;
                }
                else {
                    return ClosureResult::OVERFLOW_;
                }
            }
            else {
                return ClosureResult::OVERFLOW_;
			}
        }
        if ((targetValue > dminneg) && (targetValue < dminpos)) {
            if constexpr (MapSaturation) {
                if (result == minneg || result == minpos) {
                    return ClosureResult::SATURATE;
                }
                else {
                    return ClosureResult::UNDERFLOW_;
                }
            }
            else {
				return ClosureResult::UNDERFLOW_;
			}
        }

        // Calculate normalized relative log error for approximations
        double relativeLogError = LogRelativeError(resultValue, targetValue);
        normalizedError = MinMaxLogNormalization(relativeLogError, double(maxpos), double(minpos));

        return ClosureResult::APPROXIMATION;
    }

public:
    // Constructor
    ClosurePlotPNG() {
        updateSamplingConfiguration();
    }

    // Configuration methods
    void setMappingMode(MappingMode mode) { mappingMode = mode; }
    MappingMode getMappingMode() const { return mappingMode; }

    // Enable or disable sampling for validation purposes
    // When disabled, generates full enumeration (useful for validation)
    void setSamplingEnabled(bool enabled) {
        enableSampling = enabled;
        updateSamplingConfiguration();
    }
    bool isSamplingEnabled() const { return enableSampling; }

private:
    void updateSamplingConfiguration() {
        if (needsSampling && enableSampling) {
            // Use sampling
            plotSize = MAX_PLOT_SIZE;
            sampleStride = static_cast<double>(NR_ENCODINGS) / MAX_PLOT_SIZE;
        } else {
            // Full enumeration
            plotSize = NR_ENCODINGS;
            sampleStride = 1.0;
        }
    }

public:

    // debug 
    ClosureResult cr(NumberType va, NumberType vb, NumberType result,
        double targetValue, double& normalizedError) const {
        return classifyResult(va, vb, result, targetValue, normalizedError);
    }

    // Generate closure data for a specific operation
    template<char Op>
    ClosureData<NumberType> generateClosureData(ClosureData<NumberType>& data) const {

        // Initialize value-based encoding map if using VALUE_CENTERED mode
        // This must be done before parallel execution to avoid race conditions
        if (mappingMode == MappingMode::VALUE_CENTERED) {
            initializeValueBasedEncodingMap();
        }

        // Determine whether to use parallel execution based on plot size
        bool useParallel = (plotSize > 256);

        if (useParallel) {
            // Parallel execution for large plot spaces (> 2^8)
            std::vector<unsigned> rowIndices(plotSize);
            std::iota(rowIndices.begin(), rowIndices.end(), 0u);

            std::for_each(std::execution::par_unseq, rowIndices.begin(), rowIndices.end(),
                [this, &data](unsigned i) {
                    NumberType va{0}, vb{0}, vc{0};
                    unsigned y_pixel, y_encoding;

                    if (mappingMode == MappingMode::ENCODING_DIRECT) {
                        // direct mapping, no coordinate transformation
                        y_pixel = i;
                    } else {
                        // VALUE_CENTERED: flip Y for mathematical orientation (positive up)
                        y_pixel = plotSize - 1 - i;
                    }

                    y_encoding = getEncodingForPixel(y_pixel);
                    va.setbits(y_encoding);

                    for (unsigned j = 0; j < plotSize; ++j) {
                        unsigned x_encoding = getEncodingForPixel(j);
                        vb.setbits(x_encoding);

                        // Perform operation
                        if constexpr (Op == '+') {
                            vc = va + vb;
                        } else if constexpr (Op == '-') {
                            vc = va - vb;
                        } else if constexpr (Op == '*') {
                            vc = va * vb;
                        } else if constexpr (Op == '/') {
                            vc = va / vb;
                        }

                        // Calculate target value using double precision
                        double targetValue;
                        double dva = double(va);
                        double dvb = double(vb);
                        if constexpr (Op == '+') {
                            targetValue = dva + dvb;
                        } else if constexpr (Op == '-') {
                            targetValue = dva - dvb;
                        } else if constexpr (Op == '*') {
                            targetValue = dva * dvb;
                        } else if constexpr (Op == '/') {
                            targetValue = dva / dvb;
                        }

                        // Classify result
                        double normalizedError;
                        ClosureResult closureResult = classifyResult(va, vb, vc, targetValue, normalizedError);

                        data.results[i][j] = closureResult;
                        data.errorLevels[i][j] = static_cast<uint8_t>(std::min(255.0, normalizedError * 255.0));
                    }
                }
            );
            std::cout << "\ndata set complete (parallel execution)" << std::endl;
        } else {
            // Sequential execution for small plot spaces (<= 2^8)
            NumberType va{0}, vb{0}, vc{0};

            for (unsigned i = 0; i < plotSize; ++i) {
                unsigned y_pixel, y_encoding;

                if (mappingMode == MappingMode::ENCODING_DIRECT) {
                    // direct mapping, no coordinate transformation
                    y_pixel = i;
                } else {
                    // VALUE_CENTERED: flip Y for mathematical orientation (positive up)
                    y_pixel = plotSize - 1 - i;
                }

                y_encoding = getEncodingForPixel(y_pixel);
                va.setbits(y_encoding);

                for (unsigned j = 0; j < plotSize; ++j) {
                    unsigned x_encoding = getEncodingForPixel(j);
                    vb.setbits(x_encoding);

                    // Perform operation
                    if constexpr (Op == '+') {
                        vc = va + vb;
                    } else if constexpr (Op == '-') {
                        vc = va - vb;
                    } else if constexpr (Op == '*') {
                        vc = va * vb;
                    } else if constexpr (Op == '/') {
                        vc = va / vb;
                    }

                    // Calculate target value using double precision
                    double targetValue;
                    double dva = double(va);
                    double dvb = double(vb);
                    if constexpr (Op == '+') {
                        targetValue = dva + dvb;
                    } else if constexpr (Op == '-') {
                        targetValue = dva - dvb;
                    } else if constexpr (Op == '*') {
                        targetValue = dva * dvb;
                    } else if constexpr (Op == '/') {
                        targetValue = dva / dvb;
                    }

                    // Classify result
                    double normalizedError;
                    ClosureResult closureResult = classifyResult(va, vb, vc, targetValue, normalizedError);

                    data.results[i][j] = closureResult;
                    data.errorLevels[i][j] = static_cast<uint8_t>(std::min(255.0, normalizedError * 255.0));
                }
                if (i > 0 && (i % 1024) == 0) std::cout << '.';
            }
            std::cout << "\ndata set complete" << std::endl;
        }

        return data;
    }

    // Generate PNG from closure data
    bool generatePNG(const ClosureData<NumberType>& data,
                    const std::string& filename,
                    const std::string& /* title */ = "") const {
        PNGEncoder encoder(data.size, data.size);

        if (encoder.isStreamingMode()) {
            // For large images, use streaming generation
            return generateStreamingPNG(data, filename);
        } else {
            // For small images, use the traditional approach
            for (uint32_t i = 0; i < data.size; ++i) {
                for (uint32_t j = 0; j < data.size; ++j) {
                    ClosureResult result = data.results[i][j];
                    double errorLevel = data.errorLevels[i][j] / 255.0;
                    RGB color = getResultColor(result, errorLevel);
                    encoder.setPixel(i, j, color);
                }
            }
            return encoder.savePNG(filename);
        }
    }

    // Generate all four operation closure plots
    bool generateAllOperations(const std::string& systemName,
        const std::string& outputDir) const {
        // Ensure output directory exists
        std::filesystem::create_directories(outputDir);

        std::map<char, std::string> operations = {
            {'+', "add"}, {'-', "sub"}, {'*', "mul"}, {'/', "div"}
        };

        ClosureData<NumberType> data(plotSize);
        bool allSuccess = true;

        // Report sampling configuration
        if (needsSampling && enableSampling) {
            std::cout << "Sampling " << NR_ENCODINGS << "x" << NR_ENCODINGS
                      << " encoding space to " << plotSize << "x" << plotSize
                      << " (sample stride: " << sampleStride << ")" << std::endl;
        } else if (needsSampling && !enableSampling) {
            std::cout << "Full enumeration: " << NR_ENCODINGS << "x" << NR_ENCODINGS
                      << " encoding space (validation mode)" << std::endl;
        }

        for (const auto& opPair : operations) {
            char op = opPair.first;
            const std::string& name = opPair.second;
            std::string filename = outputDir + "/" + systemName + "_" + name + ".png";

            bool success = false;
            if (op == '+') {
                generateClosureData<'+'>(data);
                success = generatePNG(data, filename, systemName + " Addition");
            }
            else if (op == '-') {
                generateClosureData<'-'>(data);
                success = generatePNG(data, filename, systemName + " Subtraction");
            }
            else if (op == '*') {
                generateClosureData<'*'>(data);
                success = generatePNG(data, filename, systemName + " Multiplication");
            }
            else if (op == '/') {
                generateClosureData<'/'>(data);
                success = generatePNG(data, filename, systemName + " Division");
            }

            if (!success) {
                allSuccess = false;
                std::cerr << "Failed to generate " << filename << std::endl;
            }
            else {
                std::cout << "Generated " << filename << std::endl;
            }
        }

        return allSuccess;
    }

private:
    // Generate PNG using streaming approach to avoid memory issues
    bool generateStreamingPNG(const ClosureData<NumberType>& data,
                             const std::string& filename) const {

        // For very large images, output a simple PPM format instead of PNG
        // PPM is much simpler to generate without memory buffers
        std::string ppmFilename = filename;
        size_t lastDot = ppmFilename.find_last_of('.');
        if (lastDot != std::string::npos) {
            ppmFilename = ppmFilename.substr(0, lastDot) + ".ppm";
        } else {
            ppmFilename += ".ppm";
        }

        std::ofstream file(ppmFilename, std::ios::binary);
        if (!file) return false;

        // Write PPM header
        file << "P6\n" << data.size << " " << data.size << "\n255\n";

        // Write pixel data row by row
        for (uint32_t i = 0; i < data.size; ++i) {
            for (uint32_t j = 0; j < data.size; ++j) {
                ClosureResult result = data.results[i][j];
                double errorLevel = data.errorLevels[i][j] / 255.0;
                RGB color = getResultColor(result, errorLevel);
                file.put(static_cast<char>(color.r));
                file.put(static_cast<char>(color.g));
                file.put(static_cast<char>(color.b));
            }
        }

        file.close();
        std::cout << "Generated " << ppmFilename << " (PPM format for large images)" << std::endl;
        return true;
    }

};

// Convenience function to generate closure plots for any number system
template<typename NumberType>
bool generateClosurePlotsPNG(const std::string& systemName,
                            const std::string& outputDir = "closure_plots",
                            MappingMode mode = MappingMode::VALUE_CENTERED,   // ENCODING_DIRECT vs VALUE_CENTERED
                            bool enableSampling = true) {                     // Enable sampling by default
    ClosurePlotPNG<NumberType> generator;
    generator.setMappingMode(mode);
    generator.setSamplingEnabled(enableSampling);
    return generator.generateAllOperations(systemName, outputDir);
}

}} // namespace sw::universal