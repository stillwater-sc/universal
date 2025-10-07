// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// closure_posits.cpp: generate closure plots for different posit configurations
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

    using posit8  = posit<8, 2>;
    using posit10 = posit<10, 2>;
    using posit12 = posit<12, 2>;
    using posit14 = posit<14, 2>;
    using posit16 = posit<16, 2>;
    using posit18 = posit<18, 2>;
    using posit20 = posit<20, 2>;

    // Generate closure plots for 8 bit posit
    tag = "posit_" + std::to_string(8) + "_" + std::to_string(2);
    std::cout << "Generating plots for " << type_tag(posit8()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<posit8>(tag, outputDir, mode);

    tag = "posit_" + std::to_string(10) + "_" + std::to_string(2);
    std::cout << "Generating plots for " << type_tag(posit10()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<posit10>(tag, outputDir, mode);

    tag = "posit_" + std::to_string(12) + "_" + std::to_string(2);
    std::cout << "Generating plots for " << type_tag(posit12()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<posit12>(tag, outputDir, mode);

    tag = "posit_" + std::to_string(14) + "_" + std::to_string(2);
    std::cout << "Generating plots for " << type_tag(posit14()) << "..." << std::endl;
    success &= generateClosurePlotsPNG<posit14>(tag, outputDir, mode);

    tag = "posit_" + std::to_string(16) + "_" + std::to_string(2);
    std::cout << "Generating plots for " << type_tag(posit16()) << "..." << std::endl;
    success  = generateClosurePlotsPNG<posit16>(tag, outputDir, mode);

    tag = "posit_" + std::to_string(18) + "_" + std::to_string(2);
    std::cout << "Generating plots for " << type_tag(posit18()) << "..." << std::endl;
    success  = generateClosurePlotsPNG<posit18>(tag, outputDir, mode);

    tag = "posit_" + std::to_string(20) + "_" + std::to_string(2);
    std::cout << "Generating plots for " << type_tag(posit20()) << "..." << std::endl;
    success  = generateClosurePlotsPNG<posit20>(tag, outputDir, mode);

    return success;
}

int main() {

    std::string outputDir = "closure_plots_posits";

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
