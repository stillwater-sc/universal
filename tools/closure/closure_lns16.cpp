// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// closure_lns16.cpp: generate closure plots for Google Brain Float
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <universal/number/lns/lns.hpp>
#include <universal/utility/closure_plot_png.hpp>

int main() {
    using namespace sw::universal;

    std::string outputDir = "closure_plots_lns16";

    std::cout << "Generating closure plots for lns16 number system configurations...\n" << std::endl;

    std::cout << "VALUE_CENTERED mode (mathematical layout)..." << std::endl;

    // Generate sampled plots (2500x2500)
    std::cout << "=== Generating SAMPLED plots ===" << std::endl;
    std::string sampled_name = "lns_16_7";
    std::string sampled_dir = "closure_plots_lns16";

    bool success = generateClosurePlotsPNG<lns<16,7,std::uint16_t>>(
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
