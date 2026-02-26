// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// closure_cfloat8.cpp: generate closure plots for different 8-bit cfloat configurations
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/utility/closure_plot_png.hpp>

using namespace sw::universal;

bool generatePlots(const std::string& outputDir, MappingMode mode = MappingMode::VALUE_CENTERED) {
    using namespace sw::universal;
    std::string name;
    bool success;

    using cfloat8_6 = cfloat<8, 6, std::uint8_t, true, true, false>;  // subnormals, max-exponent values, not-saturating
    using cfloat8_5 = cfloat<8, 5, std::uint8_t, true, true, false>;
    using cfloat8_4 = cfloat<8, 4, std::uint8_t, true, true, false>;
    using cfloat8_3 = cfloat<8, 3, std::uint8_t, true, true, false>;
    using cfloat8_2 = cfloat<8, 2, std::uint8_t, true, true, false>;
								      
    // Generate closure plots for 8 bit cfloats
    name = "cfloat_" + std::to_string(8) + "_" + std::to_string(6);
    std::cout << "Generating plots for " << type_tag(cfloat8_6()) << "..." << std::endl;
    success = generateClosurePlotsPNG<cfloat8_6>(name, outputDir, mode);

    name = "cfloat_" + std::to_string(8) + "_" + std::to_string(5);
    std::cout << "Generating plots for " << type_tag(cfloat8_5()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<cfloat8_5>(name, outputDir, mode);

    name = "cfloat_" + std::to_string(8) + "_" + std::to_string(4);
    std::cout << "Generating plots for " << type_tag(cfloat8_4()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<cfloat8_4>(name, outputDir, mode);

    name = "cfloat_" + std::to_string(8) + "_" + std::to_string(3);
    std::cout << "Generating plots for " << type_tag(cfloat8_3()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<cfloat8_3>(name, outputDir, mode);

    name = "cfloat_" + std::to_string(8) + "_" + std::to_string(2);
    std::cout << "Generating plots for " << type_tag(cfloat8_2()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<cfloat8_2>(name, outputDir, mode);

    return success;
}

int main() {

    std::string outputDir = "closure_plots_cfloat8";

    std::cout << "Generating closure plots for cfloat number system configurations...\n" << std::endl;

     std::cout << "VALUE_CENTERED mode (mathematical layout)..." << std::endl;
    bool success = generatePlots(outputDir, MappingMode::VALUE_CENTERED);

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
