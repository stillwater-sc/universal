// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// closure_posit16_compare.cpp: generate both sampled and full enumeration closure plots for comparison
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/closure_plot_png.hpp>

int main() {
    using namespace sw::universal;

    std::cout << "Generating closure plots for posit<16,2> comparison (sampled vs full enumeration)...\n" << std::endl;

    // Generate sampled plots (2500x2500)
    std::cout << "=== Generating SAMPLED plots ===" << std::endl;
    std::string sampled_name = "posit_16_2_sampled";
    std::string sampled_dir = "closure_plots_posit16_sampled";

    bool sampled_success = generateClosurePlotsPNG<posit<16, 2>>(
        sampled_name,
        sampled_dir,
        MappingMode::VALUE_CENTERED,
        true  // Enable sampling (default)
    );

    // Generate full enumeration plots (65536x65536)
    std::cout << "\n=== Generating FULL ENUMERATION plots ===" << std::endl;
    std::string full_name = "posit_16_2_full";
    std::string full_dir = "closure_plots_posit16_full";

    bool full_success = generateClosurePlotsPNG<posit<16, 2>>(
        full_name,
        full_dir,
        MappingMode::VALUE_CENTERED,
        false  // Disable sampling
    );

    std::cout << "\n=== Results ===" << std::endl;

    if (sampled_success && full_success) {
        std::cout << "\nAll closure plots generated successfully!" << std::endl;
        std::cout << "\nSampled plots (2500x2500) in: " << sampled_dir << std::endl;
        std::cout << "  - " << sampled_name << "_add.png" << std::endl;
        std::cout << "  - " << sampled_name << "_sub.png" << std::endl;
        std::cout << "  - " << sampled_name << "_mul.png" << std::endl;
        std::cout << "  - " << sampled_name << "_div.png" << std::endl;

        std::cout << "\nFull enumeration plots (65536x65536) in: " << full_dir << std::endl;
        std::cout << "  - " << full_name << "_add.png" << std::endl;
        std::cout << "  - " << full_name << "_sub.png" << std::endl;
        std::cout << "  - " << full_name << "_mul.png" << std::endl;
        std::cout << "  - " << full_name << "_div.png" << std::endl;

        std::cout << "\nCompare the sampled vs full plots to validate sampling accuracy." << std::endl;
        return 0;
    } else {
        std::cerr << "\nSome closure plots failed to generate." << std::endl;
        if (!sampled_success) std::cerr << "  - Sampled plots failed" << std::endl;
        if (!full_success) std::cerr << "  - Full enumeration plots failed" << std::endl;
        return 1;
    }
}
