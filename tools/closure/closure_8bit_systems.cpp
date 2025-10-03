// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// closure_8bit_systems.cpp: generate closure plots for comparable 8-bit number systems
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/utility/closure_plot_png.hpp>

int main() {
    using namespace sw::universal;

    constexpr unsigned nbits = 8;
    constexpr unsigned cfloat_exp = 4;
    constexpr unsigned posit_exp = 0;
    constexpr unsigned lns_exp = 3;

    // Define number systems with similar dynamic ranges
    using RealC = cfloat<nbits, cfloat_exp, uint8_t, true, false, false>; // range ~[2^-9, 2^8]
    using RealP = posit<nbits, posit_exp>;                                 // range ~[2^-6, 2^6]
    using RealL = lns<nbits, lns_exp>;                                     // range ~[2^-8, 2^8]

    std::string outputDir = "closure_plots_8bit";

    std::cout << "Generating closure plots for 8-bit number systems...\n" << std::endl;

    // Generate closure plots for cfloat<8,4>
    std::string cfloat_name = "cfloat_" + std::to_string(nbits) + "_" + std::to_string(cfloat_exp);
    std::cout << "Generating plots for " << cfloat_name << "..." << std::endl;
    bool cfloat_success = generateClosurePlotsPNG<RealC>(cfloat_name, outputDir);

    // Generate closure plots for posit<8,0>
    std::string posit_name = "posit_" + std::to_string(nbits) + "_" + std::to_string(posit_exp);
    std::cout << "Generating plots for " << posit_name << "..." << std::endl;
    bool posit_success = generateClosurePlotsPNG<RealP>(posit_name, outputDir);

    // Generate closure plots for lns<8,3>
    std::string lns_name = "lns_" + std::to_string(nbits) + "_" + std::to_string(lns_exp);
    std::cout << "Generating plots for " << lns_name << "..." << std::endl;
    bool lns_success = generateClosurePlotsPNG<RealL>(lns_name, outputDir);

    std::cout << "\n=== Results ===" << std::endl;
    std::cout << cfloat_name << ": " << (cfloat_success ? "SUCCESS" : "FAILED") << std::endl;
    std::cout << posit_name << ": " << (posit_success ? "SUCCESS" : "FAILED") << std::endl;
    std::cout << lns_name << ": " << (lns_success ? "SUCCESS" : "FAILED") << std::endl;

    if (cfloat_success && posit_success && lns_success) {
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