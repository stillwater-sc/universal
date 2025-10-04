// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// closure_half_precision.cpp: generate closure plots for half precision IEEE-754 configurations
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/utility/closure_plot_png.hpp>

int main() {
    using namespace sw::universal;

    std::string outputDir = "closure_plots_half_precision";

    std::cout << "Generating closure plots for cfloat number system configurations...\n" << std::endl;

	// Generate closure plots for half precision IEEE-754 floating-point
    std::string cfloat_name = "cfloat_" + std::to_string(16) + "_" + std::to_string(5);
    std::cout << "Generating plots for " << cfloat_name << "..." << std::endl;
    bool cfloat_success = generateClosurePlotsPNG<cfloat<16, 5, std::uint16_t, true, false, false>>(cfloat_name, outputDir);

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
