// step3_recommend_types.cpp: Get type recommendations based on requirements
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// STEP 3 OF MIXED-PRECISION METHODOLOGY:
// Use TypeAdvisor to get ranked recommendations for number types
// based on your accuracy requirements and observed value ranges.
//
// Key concepts:
// - TypeAdvisor scores all known Universal types
// - Considers accuracy, dynamic range, energy, special values
// - Provides rationale for each recommendation
//
// Build: part of mp_step3_types target
// Run:   ./mp_step3_types

#include <universal/utility/directives.hpp>
#include <universal/utility/range_analyzer.hpp>
#include <universal/utility/type_advisor.hpp>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace sw::universal;

// Generate test data with controlled characteristics
std::vector<double> generate_ml_inference_data(size_t n) {
    // ML inference: values typically in [-10, 10], moderate precision needed
    std::vector<double> values;
    for (size_t i = 0; i < n; ++i) {
        double t = static_cast<double>(i) / n * 20.0 - 10.0;
        values.push_back(std::tanh(t));  // Activation outputs in [-1, 1]
    }
    return values;
}

std::vector<double> generate_scientific_data(size_t n) {
    // Scientific computing: wide range, high precision needed
    std::vector<double> values;
    for (size_t i = 1; i <= n; ++i) {
        // Bessel function-like oscillating decay
        double x = static_cast<double>(i) * 0.1;
        values.push_back(std::sin(x) / x * std::exp(-x * 0.01));
    }
    return values;
}

std::vector<double> generate_financial_data(size_t n) {
    // Financial: needs exact representation of decimal values
    std::vector<double> values;
    for (size_t i = 0; i < n; ++i) {
        // Simulate price movements (dollars and cents)
        double price = 100.0 + static_cast<double>(i % 100) * 0.01;
        values.push_back(price);
    }
    return values;
}

int main()
try {
    std::cout << "Step 3: Type Recommendations with TypeAdvisor\n";
    std::cout << std::string(60, '=') << "\n\n";

    TypeAdvisor advisor;

    // Show all known types
    std::cout << "Known types in database:\n";
    std::cout << std::string(40, '-') << "\n";
    for (const auto& type : advisor.knownTypes()) {
        std::cout << std::left << std::setw(20) << type.name
                  << std::right << std::setw(4) << type.total_bits << "-bit"
                  << ", eps=" << std::scientific << std::setprecision(1) << type.epsilon
                  << ", energy=" << std::fixed << std::setprecision(2) << type.energy_per_fma << " pJ\n";
    }

    // =========================================
    // Example 1: ML Inference (low precision OK)
    // =========================================
    std::cout << "\n\nExample 1: ML Inference Workload\n";
    std::cout << std::string(40, '-') << "\n";

    auto ml_data = generate_ml_inference_data(1000);
    range_analyzer<double> ml_analyzer;
    ml_analyzer.observe(ml_data.begin(), ml_data.end());

    std::cout << "Data characteristics:\n";
    std::cout << "  Min value: " << ml_analyzer.minValue() << "\n";
    std::cout << "  Max value: " << ml_analyzer.maxValue() << "\n";
    std::cout << "  Scale span: " << ml_analyzer.scaleRange() << " decades\n\n";

    AccuracyRequirement ml_accuracy;
    ml_accuracy.relative_error = 1e-2;  // 1% error OK for inference
    ml_accuracy.require_inf = false;
    ml_accuracy.require_nan = false;

    std::cout << "Requirements: " << std::scientific << ml_accuracy.relative_error
              << " relative error\n\n";

    advisor.report(std::cout, ml_analyzer, ml_accuracy);

    // =========================================
    // Example 2: Scientific Computing (high precision)
    // =========================================
    std::cout << "\n\nExample 2: Scientific Computing Workload\n";
    std::cout << std::string(40, '-') << "\n";

    auto sci_data = generate_scientific_data(1000);
    range_analyzer<double> sci_analyzer;
    sci_analyzer.observe(sci_data.begin(), sci_data.end());

    std::cout << "Data characteristics:\n";
    std::cout << "  Min value: " << std::scientific << sci_analyzer.minValue() << "\n";
    std::cout << "  Max value: " << sci_analyzer.maxValue() << "\n";
    std::cout << "  Scale span: " << sci_analyzer.scaleRange() << " decades\n";
    std::cout << "  Denormals: " << sci_analyzer.statistics().denormals << "\n\n";

    AccuracyRequirement sci_accuracy;
    sci_accuracy.relative_error = 1e-10;  // High precision needed
    sci_accuracy.require_inf = true;      // Need infinity handling
    sci_accuracy.require_nan = true;      // Need NaN handling

    std::cout << "Requirements: " << std::scientific << sci_accuracy.relative_error
              << " relative error, inf/nan support\n\n";

    advisor.report(std::cout, sci_analyzer, sci_accuracy);

    // =========================================
    // Example 3: Financial Computing
    // =========================================
    std::cout << "\n\nExample 3: Financial Computing Workload\n";
    std::cout << std::string(40, '-') << "\n";

    auto fin_data = generate_financial_data(1000);
    range_analyzer<double> fin_analyzer;
    fin_analyzer.observe(fin_data.begin(), fin_data.end());

    std::cout << "Data characteristics:\n";
    std::cout << "  Min value: " << std::fixed << std::setprecision(2) << fin_analyzer.minValue() << "\n";
    std::cout << "  Max value: " << fin_analyzer.maxValue() << "\n";
    std::cout << "  Scale span: " << fin_analyzer.scaleRange() << " decades\n\n";

    AccuracyRequirement fin_accuracy;
    fin_accuracy.relative_error = 1e-7;  // Need exact cents
    fin_accuracy.require_exact_zero = true;

    std::cout << "Requirements: " << std::scientific << fin_accuracy.relative_error
              << " relative error, exact zero\n\n";

    // Get best type
    auto best = advisor.bestType(fin_analyzer, fin_accuracy);
    std::cout << "Best recommendation: " << best.type.name << "\n";
    std::cout << "  Score: " << std::fixed << std::setprecision(1) << best.suitability_score << "%\n";
    std::cout << "  Energy: " << std::setprecision(2) << best.estimated_energy << "x FP32\n";
    std::cout << "  Rationale: " << best.rationale << "\n";

    // =========================================
    // Example 4: Custom accuracy levels
    // =========================================
    std::cout << "\n\nExample 4: Recommendations at Different Accuracy Levels\n";
    std::cout << std::string(40, '-') << "\n";

    std::vector<double> accuracy_levels = {1e-2, 1e-4, 1e-7, 1e-10, 1e-15};

    std::cout << std::left << std::setw(12) << "Accuracy"
              << std::setw(20) << "Best Type"
              << std::right << std::setw(10) << "Score"
              << std::setw(10) << "Energy" << "\n";
    std::cout << std::string(52, '-') << "\n";

    for (double acc : accuracy_levels) {
        AccuracyRequirement req(acc);
        auto best_type = advisor.bestType(ml_analyzer, req);

        std::cout << std::left << std::scientific << std::setprecision(0)
                  << std::setw(12) << acc
                  << std::setw(20) << best_type.type.name
                  << std::right << std::fixed << std::setprecision(1)
                  << std::setw(9) << best_type.suitability_score << "%"
                  << std::setprecision(2)
                  << std::setw(9) << best_type.estimated_energy << "x\n";
    }

    // =========================================
    // Summary
    // =========================================
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Key Insights:\n";
    std::cout << "  - Lower precision = lower energy = higher efficiency\n";
    std::cout << "  - posit types often score well for numerical algorithms\n";
    std::cout << "  - cfloat types needed when inf/nan handling required\n";
    std::cout << "  - fixpnt ideal for narrow-range data\n";
    std::cout << "\nNext Step: Explore trade-offs with pareto_explorer\n";
    std::cout << "See: step4_explore_tradeoffs.cpp\n";

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
