// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// closure_posit16_validation.cpp: generate full enumeration closure plots for posit<16,2> validation
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/closure_plot_png.hpp>

int main() {
    using namespace sw::universal;

    std::string outputDir = "closure_plots_posit16_validation";

    std::cout << "Generating FULL ENUMERATION closure plots for posit<16,2> validation...\n" << std::endl;

    // Generate full enumeration closure plots for posit<16,2> (no sampling)
    std::string posit_name = "posit_16_2_full_enum";
    std::cout << "Generating plots for posit<16,2> (full enumeration)..." << std::endl;

    // Disable sampling to generate full 65536x65536 plots for validation
    bool success = generateClosurePlotsPNG<posit<16, 2>>(
        posit_name,
        outputDir,
        MappingMode::VALUE_CENTERED,
        false  // Disable sampling - generate full enumeration
    );

    std::cout << "\n=== Results ===" << std::endl;

    if (success) {
        std::cout << "\nAll validation closure plots generated successfully in: " << outputDir << std::endl;
        std::cout << "\nEach plot contains full 65536x65536 enumeration:" << std::endl;
        std::cout << "  - " << posit_name << "_add.png (Addition closure plot)" << std::endl;
        std::cout << "  - " << posit_name << "_sub.png (Subtraction closure plot)" << std::endl;
        std::cout << "  - " << posit_name << "_mul.png (Multiplication closure plot)" << std::endl;
        std::cout << "  - " << posit_name << "_div.png (Division closure plot)" << std::endl;
        std::cout << "\nThese can be used as reference to validate sampled plots." << std::endl;
        return 0;
    } else {
        std::cerr << "\nSome closure plots failed to generate." << std::endl;
        return 1;
    }
}
