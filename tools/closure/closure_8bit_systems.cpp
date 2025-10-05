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
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/utility/closure_plot_png.hpp>

int main() {
    using namespace sw::universal;

    constexpr unsigned nbits = 8;
    constexpr unsigned cfloat_exp = 4;
    constexpr unsigned posit_exp = 0;
    constexpr unsigned lns_exp = 3;
    constexpr unsigned fixpnt_frac = 3;

    // Define number systems with similar dynamic ranges
    using RealC = cfloat<nbits, cfloat_exp, uint8_t, true, false, false>; // range ~[2^-9, 2^8]
    using RealP = posit<nbits, posit_exp>;                                // range ~[2^-6, 2^6]
    using RealL = lns<nbits, lns_exp>;                                    // range ~[2^-8, 2^8]
	using RealF = fixpnt<nbits, fixpnt_frac>;                             // range ~[-8,]

	std::cout << type_tag(RealC()) << " : " << dynamic_range(RealC()) << '\n';
	std::cout << type_tag(RealP()) << " : " << dynamic_range(RealP()) << '\n';
	std::cout << type_tag(RealL()) << " : " << dynamic_range(RealL()) << '\n';
	std::cout << type_tag(RealF()) << " : " << dynamic_range(RealF()) << '\n';

    std::string outputDir = "closure_plots_8bit";
    std::string tag;

    std::cout << "Generating closure plots for 8-bit number systems...\n" << std::endl;

    // Generate closure plots for cfloat<8,4>
    tag = "cfloat_" + std::to_string(nbits) + "_" + std::to_string(cfloat_exp);
    std::cout << "Generating plots for " << type_tag(RealC()) << "..." << std::endl;
    bool cfloat_success = generateClosurePlotsPNG<RealC>(tag, outputDir);

    // Generate closure plots for posit<8,0>
    tag = "posit_" + std::to_string(nbits) + "_" + std::to_string(posit_exp);
    std::cout << "Generating plots for " << type_tag(RealP()) << "..." << std::endl;
    bool posit_success = generateClosurePlotsPNG<RealP>(tag, outputDir);

    // Generate closure plots for lns<8,3>
    tag = "lns_" + std::to_string(nbits) + "_" + std::to_string(lns_exp);
    std::cout << "Generating plots for " << type_tag(RealL()) << "..." << std::endl;
    bool lns_success = generateClosurePlotsPNG<RealL>(tag, outputDir);

    // Generate closure plots for fixpnt<8,3>
    tag = "fixpnt_" + std::to_string(nbits) + "_" + std::to_string(fixpnt_frac);
    std::cout << "Generating plots for " << type_tag(RealF()) << "..." << std::endl;
    bool fixpnt_success = generateClosurePlotsPNG<RealF>(tag, outputDir);

    std::cout << "\n=== Results ===" << std::endl;

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