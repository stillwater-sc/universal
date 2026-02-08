// pgo_demo.cpp: demonstrate Profile-Guided Optimization for energy efficiency
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

// Number types
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>

// PGO framework
#include <universal/utility/pgo_energy.hpp>

using namespace sw::universal;

void demonstrateCalibration() {
    std::cout << "========================================\n";
    std::cout << "PGO Calibration Demo\n";
    std::cout << "========================================\n\n";

    // Run calibration
    runPGOCalibration(std::cout, 50);
}

void demonstrateModelValidation() {
    std::cout << "\n\n========================================\n";
    std::cout << "Model Validation\n";
    std::cout << "========================================\n\n";

    // Validate different models
    std::vector<std::pair<std::string, energy::Architecture>> models = {
        {"Intel Skylake", energy::Architecture::IntelSkylake},
        {"AMD Zen 3", energy::Architecture::AmdZen3},
        {"Apple M2", energy::Architecture::AppleM2},
        {"Generic 45nm", energy::Architecture::Generic}
    };

    for (const auto& [name, arch] : models) {
        const auto& model = energy::getModel(arch);
        ModelValidator validator(model);

        auto results = validator.validateAll();

        std::cout << "Model: " << name << " (" << model.process_nm << "nm)\n";
        std::cout << std::string(50, '-') << "\n";

        // Summary statistics
        if (results.empty()) {
            std::cout << "  No operations validated\n\n";
        } else {
            double sum_energy = 0.0;
            for (const auto& r : results) {
                sum_energy += r.predicted_pj_per_op;
            }
            double avg_energy = sum_energy / results.size();

            std::cout << "  Operations validated: " << results.size() << "\n";
            std::cout << "  Avg predicted energy: " << std::fixed << std::setprecision(2)
                      << avg_energy << " pJ/op\n\n";
        }
    }
}

void demonstratePGORecommendations() {
    std::cout << "\n\n========================================\n";
    std::cout << "PGO Precision Recommendations\n";
    std::cout << "========================================\n\n";

    PGOOptimizer optimizer;

    // Candidate precisions
    std::vector<std::pair<std::string, unsigned>> candidates = {
        {"FP64", 64},
        {"FP32", 32},
        {"FP16", 16},
        {"posit<32,2>", 32},
        {"posit<16,1>", 16},
        {"posit<8,0>", 8}
    };

    // Different scenarios
    struct Scenario {
        std::string name;
        double accuracy;
        double energy_budget;
    };

    std::vector<Scenario> scenarios = {
        {"ML Inference", 1e-2, 0.25},
        {"Graphics", 1e-3, 0.35},
        {"Signal Processing", 1e-4, 0.50},
        {"Scientific Computing", 1e-8, 1.0},
        {"Financial", 1e-12, 2.0}
    };

    std::cout << std::left << std::setw(22) << "Scenario"
              << std::setw(12) << "Accuracy"
              << std::setw(12) << "Budget"
              << std::setw(15) << "Recommended"
              << std::setw(12) << "Energy" << "\n";
    std::cout << std::string(70, '-') << "\n";

    for (const auto& scenario : scenarios) {
        auto rec = optimizer.recommend(scenario.accuracy, scenario.energy_budget, candidates);

        std::cout << std::left << std::setw(22) << scenario.name
                  << std::scientific << std::setprecision(0) << std::setw(12) << scenario.accuracy
                  << std::fixed << std::setprecision(2) << std::setw(11) << scenario.energy_budget << "x"
                  << std::left << std::setw(15) << rec.precision
                  << std::setw(11) << rec.calibrated_energy_factor << "x\n";
    }

    std::cout << "\n* Uncalibrated recommendations (no RAPL data)\n";
}

void demonstrateCalibratedRecommendations() {
    std::cout << "\n\n========================================\n";
    std::cout << "Calibrated vs Uncalibrated Recommendations\n";
    std::cout << "========================================\n\n";

    // Create optimizer with simulated calibration
    PGOOptimizer optimizer;

    // Simulate calibration coefficients
    // In real use, these would come from RAPL measurements
    CalibrationCoefficients cal;
    cal.compute_scale = 0.85;  // Model overestimates by 15%
    cal.memory_scale = 1.1;    // Model underestimates memory by 10%
    cal.bitwidth_scales[64] = 3.2;   // FP64 close to model
    cal.bitwidth_scales[32] = 0.9;   // FP32 more efficient than model
    cal.bitwidth_scales[16] = 0.75;  // FP16 much more efficient
    cal.bitwidth_scales[8] = 0.6;    // INT8 very efficient

    // Show effect of calibration
    std::cout << "Simulated calibration coefficients:\n";
    std::cout << std::string(40, '-') << "\n";
    std::cout << "  Overall scale: " << std::fixed << std::setprecision(2)
              << cal.compute_scale << "\n";
    std::cout << "  FP64 scale: " << cal.bitwidth_scales[64] << "\n";
    std::cout << "  FP32 scale: " << cal.bitwidth_scales[32] << "\n";
    std::cout << "  FP16 scale: " << cal.bitwidth_scales[16] << "\n";
    std::cout << "  INT8 scale: " << cal.bitwidth_scales[8] << "\n\n";

    std::vector<std::pair<std::string, unsigned>> candidates = {
        {"FP64", 64}, {"FP32", 32}, {"FP16", 16}, {"INT8", 8}
    };

    std::cout << std::left << std::setw(15) << "Precision"
              << std::right << std::setw(15) << "Uncalibrated"
              << std::setw(15) << "Calibrated" << "\n";
    std::cout << std::string(45, '-') << "\n";

    PGOOptimizer uncal_opt;
    PGOOptimizer cal_opt;
    cal_opt.setCalibration(cal);

    for (const auto& [name, bits] : candidates) {
        auto uncal_rec = uncal_opt.recommend(1e-4, 2.0, {{name, bits}});
        auto cal_rec = cal_opt.recommend(1e-4, 2.0, {{name, bits}});

        std::cout << std::left << std::setw(15) << name
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(14) << uncal_rec.raw_energy_factor << "x"
                  << std::setw(14) << cal_rec.calibrated_energy_factor << "x\n";
    }

    std::cout << "\nCalibration adjusts energy estimates based on hardware measurements.\n";
    std::cout << "This can significantly change precision recommendations.\n";
}

void demonstrateIterativeOptimization() {
    std::cout << "\n\n========================================\n";
    std::cout << "Iterative PGO Workflow\n";
    std::cout << "========================================\n\n";

    std::cout << "Recommended PGO workflow:\n\n";

    std::cout << "1. INITIAL PROFILING\n";
    std::cout << "   - Run application with FP32 baseline\n";
    std::cout << "   - Measure energy with RAPL\n";
    std::cout << "   - Identify hotspots\n\n";

    std::cout << "2. MODEL CALIBRATION\n";
    std::cout << "   - Run calibration benchmarks\n";
    std::cout << "   - Compare model predictions to RAPL\n";
    std::cout << "   - Learn correction factors\n\n";

    std::cout << "3. PRECISION SELECTION\n";
    std::cout << "   - Use autotuner with calibrated model\n";
    std::cout << "   - Select precision per kernel based on:\n";
    std::cout << "     - Accuracy requirements\n";
    std::cout << "     - Energy budget\n";
    std::cout << "     - Calibrated energy estimates\n\n";

    std::cout << "4. VALIDATION\n";
    std::cout << "   - Verify accuracy with new precisions\n";
    std::cout << "   - Measure actual energy savings\n";
    std::cout << "   - Compare to predictions\n\n";

    std::cout << "5. ITERATION\n";
    std::cout << "   - Repeat if targets not met\n";
    std::cout << "   - Refine calibration with more data\n";
    std::cout << "   - Adjust precision choices\n";
}

void demonstrateRAPLInfo() {
    std::cout << "\n\n========================================\n";
    std::cout << "RAPL System Information\n";
    std::cout << "========================================\n\n";

    if (energy::RaplReader::isAvailable()) {
        energy::RaplReader rapl;
        std::cout << rapl.systemInfo();

        // Quick measurement demo
        std::cout << "\nQuick measurement demo (1M FMA ops):\n";
        std::cout << std::string(40, '-') << "\n";

        rapl.start();

        // Do some work
        volatile double a = 1.0001, b = 0.9999, c = 0.0;
        for (int i = 0; i < 1000000; ++i) {
            c = a * b + c;
            a = c * 0.99999 + a;
        }

        auto result = rapl.stop();

        if (result.valid) {
            std::cout << "  Package energy: " << result.package_uj << " uJ\n";
            std::cout << "  Elapsed time:   " << std::fixed << std::setprecision(3)
                      << result.elapsed_ms << " ms\n";
            std::cout << "  Avg power:      " << std::setprecision(2)
                      << result.averagePowerWatts() << " W\n";
            std::cout << "  Energy/op:      " << std::setprecision(4)
                      << (result.package_uj * 1e6 / 2000000) << " pJ (estimated)\n";
        }
    } else {
        std::cout << "RAPL not available on this system.\n";
        std::cout << "\nRALP requires:\n";
        std::cout << "  - Linux kernel >= 3.13 with powercap\n";
        std::cout << "  - Intel or AMD processor with RAPL support\n";
        std::cout << "  - Read access to /sys/class/powercap/intel-rapl/\n";
        std::cout << "\nTo enable RAPL access (as root):\n";
        std::cout << "  sudo chmod -R a+r /sys/class/powercap/intel-rapl/\n";
    }
}

int main()
try {
    std::cout << "Universal Numbers Library: Profile-Guided Optimization Demo\n";
    std::cout << "================================================================\n\n";

    demonstrateCalibration();
    demonstrateModelValidation();
    demonstratePGORecommendations();
    demonstrateCalibratedRecommendations();
    demonstrateIterativeOptimization();
    demonstrateRAPLInfo();

    std::cout << "\n\nKey Takeaways:\n";
    std::cout << "1. PGO uses hardware measurements to calibrate energy models\n";
    std::cout << "2. Calibration improves prediction accuracy on specific hardware\n";
    std::cout << "3. Different architectures have different energy characteristics\n";
    std::cout << "4. Iterative refinement converges to optimal precision selection\n";
    std::cout << "5. RAPL provides direct hardware energy measurement on Intel/AMD\n";

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
