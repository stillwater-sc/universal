// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// cfloat_closure.cpp: generate closure plots for different cfloat configurations
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/utility/closure_plot_png.hpp>

int main() {
    using namespace sw::universal;

    std::string outputDir = "closure_plots_cfloat8";

    std::cout << "Generating closure plots for cfloat number system configurations...\n" << std::endl;

    // Generate closure plots for 8, 10, 12, 14, 16 bit cfloats
    std::string cfloat_name;
    bool cfloat_success;
    
    cfloat_name = "cfloat_" + std::to_string(8) + "_" + std::to_string(6);
    std::cout << "Generating plots for " << cfloat_name << "..." << std::endl;
    cfloat_success = generateClosurePlotsPNG<cfloat<8, 6, std::uint8_t, true, true, false>>(cfloat_name, outputDir);
    cfloat_name = "cfloat_" + std::to_string(8) + "_" + std::to_string(5);
    std::cout << "Generating plots for " << cfloat_name << "..." << std::endl;
    cfloat_success &= generateClosurePlotsPNG<cfloat<8, 5, std::uint8_t, true, true, false>>(cfloat_name, outputDir);
    cfloat_name = "cfloat_" + std::to_string(8) + "_" + std::to_string(4);
    std::cout << "Generating plots for " << cfloat_name << "..." << std::endl;
    cfloat_success &= generateClosurePlotsPNG<cfloat<8, 4, std::uint8_t, true, true, false>>(cfloat_name, outputDir);
    cfloat_name = "cfloat_" + std::to_string(8) + "_" + std::to_string(3);
    std::cout << "Generating plots for " << cfloat_name << "..." << std::endl;
    cfloat_success &= generateClosurePlotsPNG<cfloat<8, 3, std::uint8_t, true, true, false>>(cfloat_name, outputDir);
    cfloat_name = "cfloat_" + std::to_string(8) + "_" + std::to_string(2);
    std::cout << "Generating plots for " << cfloat_name << "..." << std::endl;
    cfloat_success &= generateClosurePlotsPNG<cfloat<8, 2, std::uint8_t, true, true, false>>(cfloat_name, outputDir);

    std::cout << "\n=== Results ===" << std::endl;

    if (cfloat_success) {
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
