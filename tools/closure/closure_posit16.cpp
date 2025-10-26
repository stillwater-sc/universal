// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// closure_posit16.cpp: generate closure plots for 16-bit posit configurations
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/closure_plot_png.hpp>

using namespace sw::universal;

int main() {

    std::string outputDir = "closure_plots_posit16";

    std::cout << "Generating closure plots for posit number system configurations...\n" << std::endl;

    std::cout << "VALUE_CENTERED mode (mathematical layout)..." << std::endl;
    using posit16_2 = posit<16, 2>;

    // Generate sampled plots (2500x2500)
    std::cout << "=== Generating SAMPLED plots ===" << std::endl;
    std::string sampled_name = "posit_16_2";
    std::string sampled_dir = "closure_plots_posit16";

    bool success = generateClosurePlotsPNG<posit<16, 2>>(
        sampled_name,
        sampled_dir,
        MappingMode::VALUE_CENTERED,
        true  // Enable sampling (default)
    );

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
