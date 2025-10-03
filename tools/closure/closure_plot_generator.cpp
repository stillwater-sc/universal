// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// closure_plot_generator.cpp: command-line tool for generating closure plot PNGs
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <map>
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/utility/closure_plot_png.hpp>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --type <type>       Number system type (posit, cfloat, lns, fixpnt, integer)\n";
    std::cout << "  --nbits <n>         Number of bits in encoding (default: 8)\n";
    std::cout << "  --es <n>            Exponent bits for posit (default: 0)\n";
    std::cout << "  --exp <n>           Exponent bits for cfloat (default: 4)\n";
    std::cout << "  --fbits <n>         Fraction bits for fixpnt (default: 4)\n";
    std::cout << "  --output <dir>      Output directory (default: closure_plots)\n";
    std::cout << "  --help              Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " --type posit --nbits 8 --es 0\n";
    std::cout << "  " << programName << " --type cfloat --nbits 8 --exp 4\n";
    std::cout << "  " << programName << " --type lns --nbits 8 --exp 3\n";
    std::cout << "  " << programName << " --type fixpnt --nbits 8 --fbits 4\n";
    std::cout << "  " << programName << " --type integer --nbits 8\n";
}

int main(int argc, char* argv[]) {
    using namespace sw::universal;

    // Default parameters
    std::string type = "posit";
    unsigned nbits = 8;
    unsigned es = 0;      // For posit
    unsigned exp = 4;     // For cfloat/lns
    unsigned fbits = 4;   // For fixpnt
    std::string outputDir = "closure_plots";

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--type" && i + 1 < argc) {
            type = argv[++i];
        } else if (arg == "--nbits" && i + 1 < argc) {
            nbits = std::stoul(argv[++i]);
        } else if (arg == "--es" && i + 1 < argc) {
            es = std::stoul(argv[++i]);
        } else if (arg == "--exp" && i + 1 < argc) {
            exp = std::stoul(argv[++i]);
        } else if (arg == "--fbits" && i + 1 < argc) {
            fbits = std::stoul(argv[++i]);
        } else if (arg == "--output" && i + 1 < argc) {
            outputDir = argv[++i];
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    // Validate inputs
    if (nbits < 3 || nbits > 16) {
        std::cerr << "Error: nbits must be between 3 and 16" << std::endl;
        return 1;
    }

    std::cout << "Generating closure plots for " << type << " with " << nbits << " bits..." << std::endl;

    bool success = false;
    std::string systemName;

    // Generate closure plots based on type and parameters
    if (type == "posit") {
        if (es >= nbits) {
            std::cerr << "Error: es must be less than nbits" << std::endl;
            return 1;
        }
        systemName = "posit_" + std::to_string(nbits) + "_" + std::to_string(es);

        // Use template specialization for common posit sizes
        if (nbits == 8 && es == 0) {
            success = generateClosurePlotsPNG<posit<8, 0>>(systemName, outputDir);
        } else if (nbits == 8 && es == 1) {
            success = generateClosurePlotsPNG<posit<8, 1>>(systemName, outputDir);
        } else if (nbits == 8 && es == 2) {
            success = generateClosurePlotsPNG<posit<8, 2>>(systemName, outputDir);
        } else {
            std::cerr << "Error: Unsupported posit configuration. Supported: (8,0), (8,1), (8,2)" << std::endl;
            return 1;
        }
    } else if (type == "cfloat") {
        if (exp >= nbits) {
            std::cerr << "Error: exp must be less than nbits" << std::endl;
            return 1;
        }
        systemName = "cfloat_" + std::to_string(nbits) + "_" + std::to_string(exp);

        // Use template specialization for common cfloat sizes
        if (nbits == 8 && exp == 4) {
            success = generateClosurePlotsPNG<cfloat<8, 4, uint8_t, true, false, false>>(systemName, outputDir);
        } else if (nbits == 8 && exp == 3) {
            success = generateClosurePlotsPNG<cfloat<8, 3, uint8_t, true, false, false>>(systemName, outputDir);
        } else if (nbits == 8 && exp == 5) {
            success = generateClosurePlotsPNG<cfloat<8, 5, uint8_t, true, false, false>>(systemName, outputDir);
        } else {
            std::cerr << "Error: Unsupported cfloat configuration. Supported: (8,3), (8,4), (8,5)" << std::endl;
            return 1;
        }
    } else if (type == "lns") {
        if (exp >= nbits) {
            std::cerr << "Error: exp must be less than nbits" << std::endl;
            return 1;
        }
        systemName = "lns_" + std::to_string(nbits) + "_" + std::to_string(exp);

        // Use template specialization for common lns sizes
        if (nbits == 8 && exp == 3) {
            success = generateClosurePlotsPNG<lns<8, 3>>(systemName, outputDir);
        } else if (nbits == 8 && exp == 4) {
            success = generateClosurePlotsPNG<lns<8, 4>>(systemName, outputDir);
        } else {
            std::cerr << "Error: Unsupported lns configuration. Supported: (8,3), (8,4)" << std::endl;
            return 1;
        }
    } else if (type == "fixpnt") {
        if (fbits >= nbits) {
            std::cerr << "Error: fbits must be less than nbits" << std::endl;
            return 1;
        }
        systemName = "fixpnt_" + std::to_string(nbits) + "_" + std::to_string(fbits);

        // Use template specialization for common fixpnt sizes
        if (nbits == 8 && fbits == 4) {
            success = generateClosurePlotsPNG<fixpnt<8, 4>>(systemName, outputDir);
        } else if (nbits == 8 && fbits == 3) {
            success = generateClosurePlotsPNG<fixpnt<8, 3>>(systemName, outputDir);
        } else if (nbits == 8 && fbits == 5) {
            success = generateClosurePlotsPNG<fixpnt<8, 5>>(systemName, outputDir);
        } else {
            std::cerr << "Error: Unsupported fixpnt configuration. Supported: (8,3), (8,4), (8,5)" << std::endl;
            return 1;
        }
    } else if (type == "integer") {
        systemName = "integer_" + std::to_string(nbits);

        // Use template specialization for common integer sizes
        if (nbits == 8) {
            success = generateClosurePlotsPNG<integer<8>>(systemName, outputDir);
        } else {
            std::cerr << "Error: Unsupported integer configuration. Supported: (8)" << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Error: Unknown number system type: " << type << std::endl;
        std::cerr << "Supported types: posit, cfloat, lns, fixpnt, integer" << std::endl;
        return 1;
    }

    if (success) {
        std::cout << "Successfully generated closure plots in directory: " << outputDir << std::endl;
        return 0;
    } else {
        std::cerr << "Failed to generate closure plots" << std::endl;
        return 1;
    }
}