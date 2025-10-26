// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// closure_posit8.cpp: generate closure plots for 8-bit posit configurations
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/closure_plot_png.hpp>

using namespace sw::universal;

bool generatePlots(const std::string& outputDir, MappingMode mode = MappingMode::VALUE_CENTERED) {
    using namespace sw::universal;
    std::string tag;
    bool success;

    using posit8_5 = posit<8, 5>;
    using posit8_4 = posit<8, 4>;
    using posit8_3 = posit<8, 3>;
    using posit8_2 = posit<8, 2>;
    using posit8_1 = posit<8, 2>;
    using posit8_0 = posit<8, 0>;

    // Generate closure plots for 8 bit posit
    tag = "posit_" + std::to_string(8) + "_" + std::to_string(5);
    std::cout << "Generating plots for " << type_tag(posit8_5()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<posit8_5>(tag, outputDir, mode);

    tag = "posit_" + std::to_string(8) + "_" + std::to_string(4);
    std::cout << "Generating plots for " << type_tag(posit8_4()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<posit8_4>(tag, outputDir, mode);

    tag = "posit_" + std::to_string(8) + "_" + std::to_string(3);
    std::cout << "Generating plots for " << type_tag(posit8_3()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<posit8_3>(tag, outputDir, mode);

    tag = "posit_" + std::to_string(8) + "_" + std::to_string(2);
    std::cout << "Generating plots for " << type_tag(posit8_2()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<posit8_2>(tag, outputDir, mode);

    tag = "posit_" + std::to_string(8) + "_" + std::to_string(1);
    std::cout << "Generating plots for " << type_tag(posit8_1()) << "..." << std::endl;
    success  = generateClosurePlotsPNG<posit8_1>(tag, outputDir, mode);

    tag = "posit_" + std::to_string(8) + "_" + std::to_string(0);
    std::cout << "Generating plots for " << type_tag(posit8_0()) << "..." << std::endl;
    success  = generateClosurePlotsPNG<posit8_0>(tag, outputDir, mode);

    return success;
}

int main() {

    std::string outputDir = "closure_plots_posit8";

    std::cout << "Generating closure plots for posit number system configurations...\n" << std::endl;

    std::cout << "VALUE_CENTERED mode (mathematical layout)..." << std::endl;
    bool success = generatePlots(outputDir, sw::universal::MappingMode::VALUE_CENTERED);

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
