// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// closure_lns8.cpp: generate closure plots for 8-bit LNS configurations
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <universal/number/lns/lns.hpp>
#include <universal/utility/closure_plot_png.hpp>

using namespace sw::universal;

bool generatePlots(const std::string& outputDir, MappingMode mode = MappingMode::VALUE_CENTERED) {
    using namespace sw::universal;
    std::string tag;
    bool success;

    using lns8_6 = lns<8, 6, std::uint8_t>;
    using lns8_5 = lns<8, 5, std::uint8_t>;
    using lns8_4 = lns<8, 4, std::uint8_t>;
    using lns8_3 = lns<8, 3, std::uint8_t>;
    using lns8_2 = lns<8, 2, std::uint8_t>;

    // Generate closure plots for 8 bit lns
    tag = "lns_" + std::to_string(8) + "_" + std::to_string(6);
    std::cout << "Generating plots for " << type_tag(lns8_6()) << "..." << std::endl;
    success  = generateClosurePlotsPNG<lns8_6>(tag, outputDir, mode);

    tag = "lns_" + std::to_string(8) + "_" + std::to_string(5);
    std::cout << "Generating plots for " << type_tag(lns8_5()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<lns8_5>(tag, outputDir, mode);

    tag = "lns_" + std::to_string(8) + "_" + std::to_string(4);
    std::cout << "Generating plots for " << type_tag(lns8_4()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<lns8_4>(tag, outputDir, mode);

    tag = "lns_" + std::to_string(8) + "_" + std::to_string(3);
    std::cout << "Generating plots for " << type_tag(lns8_3()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<lns8_3>(tag, outputDir, mode);

    tag = "lns_" + std::to_string(8) + "_" + std::to_string(2);
    std::cout << "Generating plots for " << type_tag(lns8_2()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<lns8_2>(tag, outputDir, mode);

    return success;
}

int main() {

    std::string outputDir = "closure_plots_lns8";

    std::cout << "Generating closure plots for lns number system configurations...\n" << std::endl;

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
