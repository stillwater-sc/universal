// autotuner_demo.cpp: demonstrate automatic precision selection
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

// Autotuner
#include <universal/utility/autotuner.hpp>

using namespace sw::universal;

void demonstrateSqrtTuning() {
    std::cout << "========================================\n";
    std::cout << "Autotuning: Square Root Function\n";
    std::cout << "========================================\n\n";

    // Scenario 1: ML inference (relaxed accuracy)
    std::cout << "Scenario 1: ML Inference (1e-2 accuracy, 30% energy budget)\n";
    std::cout << std::string(60, '-') << "\n";
    auto result_ml = autotuneSqrt(1e-2, 0.3);
    result_ml.report(std::cout);

    // Scenario 2: Graphics (moderate accuracy)
    std::cout << "\n\nScenario 2: Graphics (1e-4 accuracy, 50% energy budget)\n";
    std::cout << std::string(60, '-') << "\n";
    auto result_gfx = autotuneSqrt(1e-4, 0.5);
    result_gfx.report(std::cout);

    // Scenario 3: Scientific computing (high accuracy)
    std::cout << "\n\nScenario 3: Scientific (1e-8 accuracy, 100% energy budget)\n";
    std::cout << std::string(60, '-') << "\n";
    auto result_sci = autotuneSqrt(1e-8, 1.0);
    result_sci.report(std::cout);
}

void demonstrateExpTuning() {
    std::cout << "\n\n========================================\n";
    std::cout << "Autotuning: Exponential Function\n";
    std::cout << "========================================\n\n";

    auto result = autotuneExp(1e-4, 0.5);
    result.report(std::cout);
}

void demonstrateLogTuning() {
    std::cout << "\n\n========================================\n";
    std::cout << "Autotuning: Natural Logarithm Function\n";
    std::cout << "========================================\n\n";

    auto result = autotuneLog(1e-4, 0.5);
    result.report(std::cout);
}

void demonstrateSumTuning() {
    std::cout << "\n\n========================================\n";
    std::cout << "Autotuning: Vector Sum Reduction\n";
    std::cout << "========================================\n\n";

    std::cout << "Vector size: 1000 elements\n\n";

    // Different accuracy requirements
    std::cout << "Low accuracy (1e-2):\n";
    auto result_low = autotuneSum(1000, 1e-2, 0.3);
    std::cout << "  Recommended: " << result_low.recommended.precision_name
              << " (energy=" << std::fixed << std::setprecision(2)
              << result_low.recommended.estimated_energy_factor << "x)\n\n";

    std::cout << "Medium accuracy (1e-4):\n";
    auto result_med = autotuneSum(1000, 1e-4, 0.5);
    std::cout << "  Recommended: " << result_med.recommended.precision_name
              << " (energy=" << result_med.recommended.estimated_energy_factor << "x)\n\n";

    std::cout << "High accuracy (1e-8):\n";
    auto result_high = autotuneSum(1000, 1e-8, 1.0);
    std::cout << "  Recommended: " << result_high.recommended.precision_name
              << " (energy=" << result_high.recommended.estimated_energy_factor << "x)\n";
}

void demonstrateCustomKernel() {
    std::cout << "\n\n========================================\n";
    std::cout << "Autotuning: Custom Polynomial Kernel\n";
    std::cout << "========================================\n\n";

    // Horner's method for polynomial evaluation: x^3 - 2x^2 + 3x - 4
    Autotuner tuner;
    tuner.setAccuracyRequirement(1e-4);
    tuner.setEnergyBudget(0.4);
    tuner.enableTiming(true);

    auto inputs = Autotuner::generateTestInputs(-10.0, 10.0, 200);

    auto result = tuner.tuneUnaryFunction("polynomial_eval",
        [](auto x) {
            // p(x) = x^3 - 2x^2 + 3x - 4
            // Horner: ((x - 2)*x + 3)*x - 4
            auto y = x - decltype(x)(2);
            y = y * x + decltype(x)(3);
            y = y * x - decltype(x)(4);
            return y;
        },
        inputs);

    result.report(std::cout);

    std::cout << "\nTiming measurements (ns/operation):\n";
    std::cout << std::string(40, '-') << "\n";
    for (const auto& pt : result.all_points) {
        std::cout << std::left << std::setw(18) << pt.precision_name
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(12) << pt.execution_time_ns << " ns\n";
    }
}

void demonstrateBinaryFunction() {
    std::cout << "\n\n========================================\n";
    std::cout << "Autotuning: Binary Power Function\n";
    std::cout << "========================================\n\n";

    Autotuner tuner;
    tuner.setAccuracyRequirement(1e-3);
    tuner.setEnergyBudget(0.5);

    // Generate test pairs for pow(x, y)
    std::vector<std::pair<double, double>> test_pairs;
    for (double x = 0.1; x <= 10.0; x += 0.5) {
        for (double y = -2.0; y <= 2.0; y += 0.5) {
            test_pairs.push_back({x, y});
        }
    }

    auto result = tuner.tuneBinaryFunction("pow",
        [](auto a, auto b) { using std::pow; return pow(a, b); },
        test_pairs);

    result.report(std::cout);
}

void demonstrateComparisonSummary() {
    std::cout << "\n\n========================================\n";
    std::cout << "Summary: Precision Selection by Domain\n";
    std::cout << "========================================\n\n";

    struct DomainConfig {
        std::string name;
        double accuracy;
        double energy;
    };

    std::vector<DomainConfig> domains = {
        {"ML Inference", 1e-2, 0.25},
        {"Real-time Graphics", 1e-3, 0.35},
        {"Audio/Signal Proc", 1e-4, 0.50},
        {"CAD/Engineering", 1e-6, 0.75},
        {"Scientific/FEM", 1e-8, 1.0},
        {"Financial/HFT", 1e-10, 1.5}
    };

    std::cout << std::left << std::setw(20) << "Domain"
              << std::setw(12) << "Accuracy"
              << std::setw(15) << "sqrt"
              << std::setw(15) << "exp"
              << std::setw(15) << "sum" << "\n";
    std::cout << std::string(75, '-') << "\n";

    for (const auto& domain : domains) {
        auto sqrt_res = autotuneSqrt(domain.accuracy, domain.energy);
        auto exp_res = autotuneExp(domain.accuracy, domain.energy);
        auto sum_res = autotuneSum(1000, domain.accuracy, domain.energy);

        std::cout << std::left << std::setw(20) << domain.name
                  << std::scientific << std::setprecision(0) << std::setw(12) << domain.accuracy
                  << std::left << std::setw(15) << sqrt_res.recommended.precision_name
                  << std::setw(15) << exp_res.recommended.precision_name
                  << std::setw(15) << sum_res.recommended.precision_name << "\n";
    }

    std::cout << "\nKey:\n";
    std::cout << "  FP64 = 64-bit IEEE double\n";
    std::cout << "  FP32 = 32-bit IEEE float\n";
    std::cout << "  FP16 = 16-bit IEEE half\n";
    std::cout << "  posit<N,E> = N-bit posit with E exponent bits\n";
}

int main()
try {
    std::cout << "Universal Numbers Library: Autotuning for Precision Selection\n";
    std::cout << "================================================================\n\n";

    demonstrateSqrtTuning();
    demonstrateExpTuning();
    demonstrateLogTuning();
    demonstrateSumTuning();
    demonstrateCustomKernel();
    demonstrateBinaryFunction();
    demonstrateComparisonSummary();

    std::cout << "\n\nKey Takeaways:\n";
    std::cout << "1. Autotuning measures actual accuracy for each precision\n";
    std::cout << "2. Energy estimates help select efficient configurations\n";
    std::cout << "3. Different functions have different precision requirements\n";
    std::cout << "4. Posits often provide better accuracy per bit than IEEE floats\n";
    std::cout << "5. Custom kernels can be tuned with the same framework\n";

    return EXIT_SUCCESS;
}
catch (const char* msg) {
    std::cerr << "Error: " << msg << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
}
