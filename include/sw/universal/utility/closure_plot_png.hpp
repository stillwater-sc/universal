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

// Structure to hold closure plot data for a single operation
template<typename NumberType>
struct ClosureData {
    std::vector<std::vector<ClosureResult>> results;
    std::vector<std::vector<double>> errorValues;
    std::vector<NumberType> operand1Values;
    std::vector<NumberType> operand2Values;
    uint32_t size;

    ClosureData(uint32_t operandCount) : size(operandCount) {
        results.resize(size, std::vector<ClosureResult>(size));
        errorValues.resize(size, std::vector<double>(size, 0.0));
        operand1Values.resize(size);
        operand2Values.resize(size);
    }
};

// PNG Closure Plot Generator
template<typename NumberType>
class ClosurePlotPNG {
private:
    static const unsigned NBITS = NumberType::nbits;
    static const unsigned NR_ENCODINGS = (1u << NBITS);

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
    ClosureResult classifyResult(NumberType va, NumberType vb, NumberType result,
                               double targetValue, double& normalizedError) const {
        normalizedError = 0.0;

        // Handle special values using uniform ADL interface
        if (sw::universal::isnan(result)) {
            return ClosureResult::NAN_NAR;
        }
        else if (sw::universal::isinf(result)) {
            return ClosureResult::OVERFLOW_;
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
        NumberType minpos(sw::universal::SpecificValue::minpos);
        double dmaxpos = double(maxpos);
        double dminpos = double(minpos);

        // Simple overflow/underflow detection
        if (targetValue > dmaxpos) {
            return ClosureResult::OVERFLOW_;
        }
        if (std::abs(targetValue) > 0.0 && std::abs(targetValue) < dminpos) {
            return ClosureResult::UNDERFLOW_;
        }

        // Calculate normalized relative log error for approximations
        double relativeLogError = LogRelativeError(resultValue, targetValue);
        normalizedError = MinMaxLogNormalization(relativeLogError, double(maxpos), double(minpos));

        return ClosureResult::APPROXIMATION;
    }

public:
    // Generate closure data for a specific operation
    template<char Op>
    ClosureData<NumberType> generateClosureData() const {
        ClosureData<NumberType> data(NR_ENCODINGS);

        NumberType va{0}, vb{0}, vc{0};

        // Populate operand values
        for (unsigned i = 0; i < NR_ENCODINGS; ++i) {
            va.setbits(i);
            data.operand1Values[i] = va;
            data.operand2Values[i] = va; // Same values for both axes
        }

        // Generate closure results
        for (unsigned i = 0; i < NR_ENCODINGS; ++i) {
            va.setbits(i);
            for (unsigned j = 0; j < NR_ENCODINGS; ++j) {
                vb.setbits(j);

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
                data.errorValues[i][j] = normalizedError;
            }
        }

        return data;
    }

    // Generate PNG from closure data
    bool generatePNG(const ClosureData<NumberType>& data,
                    const std::string& filename,
                    const std::string& title = "") const {
        PNGEncoder encoder(data.size, data.size);

        // Fill the image based on closure results
        for (uint32_t i = 0; i < data.size; ++i) {
            for (uint32_t j = 0; j < data.size; ++j) {
                ClosureResult result = data.results[i][j];
                double errorLevel = data.errorValues[i][j];
                RGB color = getResultColor(result, errorLevel);
                encoder.setPixel(i, j, color);
            }
        }

        return encoder.savePNG(filename);
    }

    // Generate all four operation closure plots
    bool generateAllOperations(const std::string& systemName,
                              const std::string& outputDir) const {
        // Ensure output directory exists
        std::filesystem::create_directories(outputDir);

        std::map<char, std::string> operations = {
            {'+', "add"}, {'-', "sub"}, {'*', "mul"}, {'/', "div"}
        };

        bool allSuccess = true;

        for (const auto& opPair : operations) {
            char op = opPair.first;
            const std::string& name = opPair.second;
            std::string filename = outputDir + "/" + systemName + "_" + name + ".png";

            bool success = false;
            if (op == '+') {
                auto data = generateClosureData<'+'>();
                success = generatePNG(data, filename, systemName + " Addition");
            } else if (op == '-') {
                auto data = generateClosureData<'-'>();
                success = generatePNG(data, filename, systemName + " Subtraction");
            } else if (op == '*') {
                auto data = generateClosureData<'*'>();
                success = generatePNG(data, filename, systemName + " Multiplication");
            } else if (op == '/') {
                auto data = generateClosureData<'/'>();
                success = generatePNG(data, filename, systemName + " Division");
            }

            if (!success) {
                allSuccess = false;
                std::cerr << "Failed to generate " << filename << std::endl;
            } else {
                std::cout << "Generated " << filename << std::endl;
            }
        }

        return allSuccess;
    }
};

// Convenience function to generate closure plots for any number system
template<typename NumberType>
bool generateClosurePlotsPNG(const std::string& systemName,
                            const std::string& outputDir = "closure_plots") {
    ClosurePlotPNG<NumberType> generator;
    return generator.generateAllOperations(systemName, outputDir);
}

}} // namespace sw::universal