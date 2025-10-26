// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// test_posit_closure.cpp: tracing out closure labeling of posits
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
    using posit8 = posit<8, 2>;

    // Generate sampled plots (2500x2500)
    std::cout << "=== Generating SAMPLED plots ===" << std::endl;
    std::string name = "trace_posit_8_2";
    std::string output_dir = "test_closure_plots";

    // set a break-point in classifyResult on the classifier of interest
    bool success = generateClosurePlotsPNG<posit<8, 2>>(
        name,
        output_dir,
        MappingMode::VALUE_CENTERED,
        true  // Enable sampling (default)
    );

}
