// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// test_closure.cpp: test closure plots for configurations
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/utility/closure_plot_png.hpp>

using namespace sw::universal;

bool generatePlots(const std::string& outputDir, MappingMode mode = MappingMode::VALUE_CENTERED) {
    using namespace sw::universal;
    std::string tag;
    bool success;

    using fixpnt8_6 = fixpnt<8, 6, Saturate, std::uint8_t>;
    using fixpnt8_5 = fixpnt<8, 5, Saturate, std::uint8_t>;
    using fixpnt8_4 = fixpnt<8, 4, Saturate, std::uint8_t>;
    using fixpnt8_3 = fixpnt<8, 3, Saturate, std::uint8_t>;
    using fixpnt8_2 = fixpnt<8, 2, Saturate, std::uint8_t>;

    // Generate closure plots for 8 bit fixpnt
    tag = "fixpnt_" + std::to_string(8) + "_" + std::to_string(6);
    std::cout << "Generating plots for " << type_tag(fixpnt8_6()) << "..." << std::endl;
    success  = generateClosurePlotsPNG<fixpnt8_6>(tag, outputDir, mode);

    tag = "fixpnt_" + std::to_string(8) + "_" + std::to_string(5);
    std::cout << "Generating plots for " << type_tag(fixpnt8_5()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<fixpnt8_5>(tag, outputDir, mode);

    tag = "fixpnt_" + std::to_string(8) + "_" + std::to_string(4);
    std::cout << "Generating plots for " << type_tag(fixpnt8_4()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<fixpnt8_4>(tag, outputDir, mode);

    tag = "fixpnt_" + std::to_string(8) + "_" + std::to_string(3);
    std::cout << "Generating plots for " << type_tag(fixpnt8_3()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<fixpnt8_3>(tag, outputDir, mode);

    tag = "fixpnt_" + std::to_string(8) + "_" + std::to_string(2);
    std::cout << "Generating plots for " << type_tag(fixpnt8_2()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<fixpnt8_2>(tag, outputDir, mode);

    return success;
}

namespace sw::universal {

	std::vector<unsigned> encodingMap; // global cache for value-based encoding map

    // Create value-based encoding map (one-time setup, cached)
	template<typename NumberType>
    void createValueBasedEncodingMap(std::vector<unsigned>& valueBasedEncodingMap) {
        const unsigned NR_ENCODINGS = (1u << NumberType::nbits);

        std::vector<std::pair<double, unsigned>> valueEncodingPairs;

        // Sample all encodings and their actual values
        for (unsigned encoding = 0; encoding < NR_ENCODINGS; ++encoding) {
            NumberType temp;
            temp.setbits(encoding);
            double value = double(temp);
            valueEncodingPairs.emplace_back(value, encoding);
        }

        // Sort by actual numerical value: maxneg -> ... -> zero -> ... -> maxpos
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

    // Get encoding for mathematical value-based ordering
    unsigned getValueBasedEncoding(unsigned pixelCoord) {
        return encodingMap[pixelCoord];
    }

    // Helper function to map pixel coordinate to encoding based on mapping mode
    unsigned getEncodingForPixel(unsigned pixelCoord, MappingMode mappingMode) {
        if (mappingMode == MappingMode::ENCODING_DIRECT) {
            // direct pixel to encoding mapping
            return pixelCoord;
        }
        else {
            // VALUE_CENTERED: use value-based encoding
            return getValueBasedEncoding(pixelCoord);
        }
    }

    // Classify arithmetic result
	template<typename NumberType>
    ClosureResult classifyResult(NumberType va, NumberType vb, NumberType result, double targetValue, double& normalizedError) {
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
        NumberType maxneg(sw::universal::SpecificValue::maxneg);
        NumberType minpos(sw::universal::SpecificValue::minpos);
        NumberType minneg(sw::universal::SpecificValue::minneg);
        double dmaxpos = double(maxpos);
        double dmaxneg = double(maxneg);
        double dminpos = double(minpos);
        double dminneg = double(minneg);

        // Simple overflow/underflow detection
        if ( (targetValue > dmaxpos) || (targetValue < dmaxneg) ) {
            if (result == maxpos || result == maxneg) {
                return ClosureResult::SATURATE;
			}
            else {
                return ClosureResult::OVERFLOW_;
            }
        }
        if (targetValue > dminneg && targetValue < dminpos) {
            if (result == minpos || result == minneg) {
                return ClosureResult::SATURATE;
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

    // Structure to hold closure plot data for a single operation
    template<typename NumberType>
    struct ResultData {
        std::vector<std::vector<NumberType>> results;
        std::vector<std::vector<double>> errorLevels;
        uint32_t size;

        ResultData(uint32_t operandCount) : size(operandCount) {
            results.resize(size, std::vector<NumberType>(size));
            errorLevels.resize(size, std::vector<double>(size, 0));
        }
    };

    template<typename NumberType, char Op>
    void generateResultsTable(ResultData<NumberType>& data, MappingMode mappingMode = MappingMode::VALUE_CENTERED) {
        NumberType va{ 0 }, vb{ 0 }, vc{ 0 };
        const unsigned NR_ENCODINGS = (1u << NumberType::nbits);

        // Generate closure results
        for (unsigned i = 0; i < NR_ENCODINGS; ++i) {
            unsigned y_pixel, y_encoding;

            if (mappingMode == MappingMode::ENCODING_DIRECT) {
                // direct mapping, no coordinate transformation
                y_pixel = i;
            }
            else {
                // VALUE_CENTERED: flip Y for mathematical orientation (positive up)
                y_pixel = NR_ENCODINGS - 1 - i;
            }

            y_encoding = getEncodingForPixel(y_pixel, mappingMode);
            va.setbits(y_encoding);

            for (unsigned j = 0; j < NR_ENCODINGS; ++j) {
                unsigned x_encoding = getEncodingForPixel(j, mappingMode);
                vb.setbits(x_encoding);

                // Perform operation
                if constexpr (Op == '+') {
                    vc = va + vb;
                }
                else if constexpr (Op == '-') {
                    vc = va - vb;
                }
                else if constexpr (Op == '*') {
                    vc = va * vb;
                }
                else if constexpr (Op == '/') {
                    vc = va / vb;
                }

                // Calculate target value using double precision
                double dvc;
                double dva = double(va);
                double dvb = double(vb);
                if constexpr (Op == '+') {
                    dvc = dva + dvb;
                }
                else if constexpr (Op == '-') {
                    dvc = dva - dvb;
                }
                else if constexpr (Op == '*') {
                    dvc = dva * dvb;
                }
                else if constexpr (Op == '/') {
                    dvc = dva / dvb;
                }
				data.results[i][j] = vc;
				data.errorLevels[i][j] = dvc - double(vc);
            }
        }
    }

    // Generate closure data for a specific operation
    template<typename NumberType, char Op>
    ClosureData<NumberType> generateClosureData(ClosureData<NumberType>& data, MappingMode mappingMode = MappingMode::VALUE_CENTERED) {

        NumberType va{ 0 }, vb{ 0 }, vc{ 0 };

        const unsigned NR_ENCODINGS = (1u << NumberType::nbits);

        // Generate closure results
        for (unsigned i = 0; i < NR_ENCODINGS; ++i) {
            unsigned y_pixel, y_encoding;

            if (mappingMode == MappingMode::ENCODING_DIRECT) {
                // direct mapping, no coordinate transformation
                y_pixel = i;
            }
            else {
                // VALUE_CENTERED: flip Y for mathematical orientation (positive up)
                y_pixel = NR_ENCODINGS - 1 - i;
            }

            y_encoding = getEncodingForPixel(y_pixel, mappingMode);
            va.setbits(y_encoding);

            for (unsigned j = 0; j < NR_ENCODINGS; ++j) {
                unsigned x_encoding = getEncodingForPixel(j, mappingMode);
                vb.setbits(x_encoding);

                // Perform operation
                if constexpr (Op == '+') {
                    vc = va + vb;
                }
                else if constexpr (Op == '-') {
                    vc = va - vb;
                }
                else if constexpr (Op == '*') {
                    vc = va * vb;
                }
                else if constexpr (Op == '/') {
                    vc = va / vb;
                }

                // Calculate target value using double precision
                double targetValue;
                double dva = double(va);
                double dvb = double(vb);
                if constexpr (Op == '+') {
                    targetValue = dva + dvb;
                }
                else if constexpr (Op == '-') {
                    targetValue = dva - dvb;
                }
                else if constexpr (Op == '*') {
                    targetValue = dva * dvb;
                }
                else if constexpr (Op == '/') {
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

        return data;
    }
}

int main() {
	using namespace sw::universal;

    constexpr bool ArithmeticMode = Modulo;
    using FIXPNT = fixpnt<4, 2, ArithmeticMode>;
	constexpr unsigned NR_ENCODINGS = (1u << FIXPNT::nbits);
	createValueBasedEncodingMap<FIXPNT>(encodingMap);  // create global cache for value-based encoding

	ResultData<FIXPNT> results(NR_ENCODINGS);
	generateResultsTable<FIXPNT, '/'>(results);

	FIXPNT va{ 0 }, vb{ 0 };

    // VALUE_CENTERED: flip Y for mathematical orientation (positive up)

    std::cout << "Results of FIXPNT operation\n";
	// first print the header with the value-based encodings of the x operand
    std::cout << "      ";
    for (unsigned j = 0; j < NR_ENCODINGS; ++j) {
        unsigned x_encoding = getEncodingForPixel(j, MappingMode::VALUE_CENTERED);
        vb.setbits(x_encoding);
        std::cout << std::setw(5) << float(vb) << ' ';
    }
	std::cout << '\n';
    for (unsigned i = 0; i < NR_ENCODINGS; ++i) {
		// then print the value-based encodings of the y operand
        unsigned y_pixel, y_encoding;

        // VALUE_CENTERED: flip Y for mathematical orientation (positive up)
        y_pixel = NR_ENCODINGS - 1 - i;

        y_encoding = getEncodingForPixel(y_pixel, MappingMode::VALUE_CENTERED);
        va.setbits(y_encoding);
        std::cout << std::setw(5) << float(va) << ' ';
		// followed by the results
        for (unsigned j = 0; j < NR_ENCODINGS; ++j) {
            std::cout << std::setw(5) << float(results.results[i][j]) << ' ';
        }
        std::cout << '\n';
    }


	std::cout << "\nError levels:\n";
    // first print the header with the value-based encodings of the x operand
    std::cout << "      ";
    for (unsigned j = 0; j < NR_ENCODINGS; ++j) {
        unsigned x_encoding = getEncodingForPixel(j, MappingMode::VALUE_CENTERED);
        vb.setbits(x_encoding);
        std::cout << std::setw(5) << float(vb) << ' ';
    }
    std::cout << '\n';
    for (unsigned i = 0; i < NR_ENCODINGS; ++i) {
        // then print the value-based encodings of the y operand
        unsigned y_pixel, y_encoding;

        // VALUE_CENTERED: flip Y for mathematical orientation (positive up)
        y_pixel = NR_ENCODINGS - 1 - i;

        y_encoding = getEncodingForPixel(y_pixel, MappingMode::VALUE_CENTERED);
        va.setbits(y_encoding);
        std::cout << std::setw(5) << float(va) << ' ';
        // followed by the results
        for (unsigned j = 0; j < NR_ENCODINGS; ++j) {
            std::cout << std::setw(5) << float(results.errorLevels[i][j]) << ' ';
        }
        std::cout << '\n';
    }

    ClosureData<FIXPNT> data(NR_ENCODINGS);
	generateClosureData<FIXPNT, '+'>(data);

    std::cout << "\nClosure results:\n";
    // first print the header with the value-based encodings of the x operand
    std::cout << "      ";
    for (unsigned j = 0; j < NR_ENCODINGS; ++j) {
        unsigned x_encoding = getEncodingForPixel(j, MappingMode::VALUE_CENTERED);
        vb.setbits(x_encoding);
        std::cout << std::setw(5) << float(vb) << ' ';
    }
    std::cout << '\n';
    for (unsigned i = 0; i < NR_ENCODINGS; ++i) {
        // then print the value-based encodings of the y operand
        unsigned y_pixel, y_encoding;

        // VALUE_CENTERED: flip Y for mathematical orientation (positive up)
        y_pixel = NR_ENCODINGS - 1 - i;

        y_encoding = getEncodingForPixel(y_pixel, MappingMode::VALUE_CENTERED);
        va.setbits(y_encoding);
        std::cout << std::setw(5) << float(va) << ' ';
        // followed by the results
        for (unsigned j = 0; j < NR_ENCODINGS; ++j) {
            std::cout << std::setw(5) << int(data.results[i][j]) << ' ';
        }
        std::cout << '\n';
	}

    for (int cr = static_cast<int>(ClosureResult::EXACT); cr <= static_cast<int>(ClosureResult::SATURATE); ++cr) {
        std::cout << cr << " : " << static_cast<ClosureResult>(cr) << '\n';
	}

    for (unsigned i = 0; i < NR_ENCODINGS; ++i) {
        FIXPNT a;
        a.setbits(i);
		std::cout << to_binary(a) << " : " << float(a) << '\n';
    }

}
