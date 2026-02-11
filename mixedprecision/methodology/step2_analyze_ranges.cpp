// step2_analyze_ranges.cpp: Analyze value ranges to understand precision requirements
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// STEP 2 OF MIXED-PRECISION METHODOLOGY:
// Analyze the distribution of values in your algorithm to understand:
// - What dynamic range is actually used?
// - Are there denormal values?
// - What precision is needed?
//
// Key concepts:
// - range_analyzer tracks min/max, scale range, special values
// - scale_tracker provides histogram of exponent distribution
// - Results feed into type recommendation
//
// Build: part of mp_step2_ranges target
// Run:   ./mp_step2_ranges

#include <universal/utility/directives.hpp>
#include <universal/utility/range_analyzer.hpp>
#include <universal/utility/scale_tracker.hpp>
#include <universal/number/cfloat/cfloat.hpp>  // for half type
#include <vector>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>

using namespace sw::universal;

// Simulate an iterative algorithm that produces a range of values
std::vector<double> simulate_iterative_algorithm(size_t iterations) {
    std::vector<double> values;
    values.reserve(iterations * 3);

    double x = 1.0;
    for (size_t i = 0; i < iterations; ++i) {
        // Newton-Raphson iteration for sqrt(2)
        x = 0.5 * (x + 2.0 / x);
        values.push_back(x);

        // Track the error (gets very small)
        double error = x * x - 2.0;
        values.push_back(error);

        // Track the correction term
        double correction = 2.0 / x - x;
        values.push_back(correction);
    }

    return values;
}

// Simulate a computation with varying magnitudes (like FFT)
std::vector<double> simulate_varying_magnitude(size_t n) {
    std::vector<double> values;
    values.reserve(n);

    for (size_t i = 1; i <= n; ++i) {
        // Power spectrum: magnitudes vary over many orders
        double freq = static_cast<double>(i);
        double magnitude = 1000.0 / (freq * freq);  // 1/f^2 spectrum
        values.push_back(magnitude);
    }

    return values;
}

// Simulate near-zero values (tests subnormal handling)
std::vector<double> simulate_near_zero(size_t n) {
    std::vector<double> values;
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(-1e-300, 1e-300);

    for (size_t i = 0; i < n; ++i) {
        values.push_back(dist(rng));
    }

    return values;
}

int main()
try {
    std::cout << "Step 2: Analyze Value Ranges with range_analyzer\n";
    std::cout << std::string(60, '=') << "\n\n";

    // =========================================
    // Example 1: Iterative algorithm analysis
    // =========================================
    std::cout << "Example 1: Newton-Raphson Iteration\n";
    std::cout << std::string(40, '-') << "\n";

    auto newton_values = simulate_iterative_algorithm(100);

    range_analyzer<double> analyzer1;
    for (const auto& v : newton_values) {
        analyzer1.observe(v);
    }

    analyzer1.report(std::cout);

    // Get precision recommendation
    auto rec1 = analyzer1.recommendPrecision();
    std::cout << "\nRecommendation:\n";
    std::cout << "  Type: " << rec1.type_suggestion << "\n";
    std::cout << "  Needs subnormals: " << (rec1.needs_subnormals ? "yes" : "no") << "\n";

    // =========================================
    // Example 2: Varying magnitude analysis
    // =========================================
    std::cout << "\n\nExample 2: Power Spectrum (1/f^2)\n";
    std::cout << std::string(40, '-') << "\n";

    auto spectrum_values = simulate_varying_magnitude(1000);

    range_analyzer<double> analyzer2;
    analyzer2.observe(spectrum_values.begin(), spectrum_values.end());

    analyzer2.report(std::cout);

    // =========================================
    // Example 3: Scale tracking with histogram
    // =========================================
    std::cout << "\n\nExample 3: Scale Distribution Histogram\n";
    std::cout << std::string(40, '-') << "\n";

    // Track scales from 2^-20 to 2^10
    scaleTracker tracker(-20, 10);

    for (const auto& v : spectrum_values) {
        if (v != 0.0) {
            int scale = static_cast<int>(std::floor(std::log2(std::abs(v))));
            tracker.incr(scale);
        }
    }

    std::cout << "Exponent distribution for spectrum values:\n";
    tracker.report(std::cout);

    // =========================================
    // Example 4: Near-zero values (subnormals)
    // =========================================
    std::cout << "\n\nExample 4: Near-Zero Values\n";
    std::cout << std::string(40, '-') << "\n";

    auto tiny_values = simulate_near_zero(1000);

    range_analyzer<double> analyzer3;
    analyzer3.observe(tiny_values.begin(), tiny_values.end());

    analyzer3.report(std::cout);

    auto rec3 = analyzer3.recommendPrecision();
    std::cout << "\nNote: " << analyzer3.statistics().denormals
              << " denormal values detected\n";
    std::cout << "Subnormal support needed: "
              << (rec3.needs_subnormals ? "YES" : "no") << "\n";

    // =========================================
    // Example 5: Compare ranges to target type
    // =========================================
    std::cout << "\n\nExample 5: Range Compatibility Check\n";
    std::cout << std::string(40, '-') << "\n";

    std::cout << "Checking if spectrum values fit in float (FP32):\n";
    compareRanges<double, float>(analyzer2, std::cout);

    std::cout << "\nChecking if tiny values fit in half (FP16):\n";
    compareRanges<double, half>(analyzer3, std::cout);

    // =========================================
    // Summary
    // =========================================
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Key Insights:\n";
    std::cout << "  - Scale span tells you how many exponent bits needed\n";
    std::cout << "  - Denormal count indicates if gradual underflow matters\n";
    std::cout << "  - Dynamic range utilization shows if type is oversized\n";
    std::cout << "\nNext Step: Use type_advisor to get specific recommendations\n";
    std::cout << "See: step3_recommend_types.cpp\n";

    return EXIT_SUCCESS;
}
catch (const char* msg) {
    std::cerr << "Caught exception: " << msg << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception& e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
}
