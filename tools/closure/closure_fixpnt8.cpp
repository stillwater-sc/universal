// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// closure_fixpnt8.cpp: generate closure plots for 8-bit fixpnt configurations
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/utility/closure_plot_png.hpp>

using namespace sw::universal;

template<bool ArithmeticMode = sw::universal::Saturate>
bool generatePlots(const std::string& outputDir, MappingMode mode = MappingMode::VALUE_CENTERED) {
    using namespace sw::universal;
    std::string tag;
    bool success;

    using fixpnt8_6 = fixpnt<8, 6, ArithmeticMode, std::uint8_t>;
    using fixpnt8_5 = fixpnt<8, 5, ArithmeticMode, std::uint8_t>;
    using fixpnt8_4 = fixpnt<8, 4, ArithmeticMode, std::uint8_t>;
    using fixpnt8_3 = fixpnt<8, 3, ArithmeticMode, std::uint8_t>;
    using fixpnt8_2 = fixpnt<8, 2, ArithmeticMode, std::uint8_t>;

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

int main() {

    std::string outputDir = "closure_plots_fixpnt8";

    std::cout << "Generating closure plots for fixpnt number system configurations...\n" << std::endl;

    std::cout << "VALUE_CENTERED mode (mathematical layout)..." << std::endl;
    bool success = generatePlots<Saturate>(outputDir + std::string("/saturate"), sw::universal::MappingMode::VALUE_CENTERED);
    success &= generatePlots<Modulo>(outputDir + std::string("/modulo"), sw::universal::MappingMode::VALUE_CENTERED);


    std::cout << "\n=== Results ===" << std::endl;

    if (success) {
        std::cout << "\nAll closure plots generated successfully in: " << outputDir << std::endl;
        std::cout << "\nEach system generated 4 plots:" << std::endl;
        std::cout << "  - *_add.png (Addition closure plot)" << std::endl;
        std::cout << "  - *_sub.png (Subtraction closure plot)" << std::endl;
        std::cout << "  - *_mul.png (Multiplication closure plot)" << std::endl;
        std::cout << "  - *_div.png (Division closure plot)" << std::endl;
        return 0;
    } else {
        std::cerr << "\nSome closure plots failed to generate." << std::endl;
        return 1;
    }
}
