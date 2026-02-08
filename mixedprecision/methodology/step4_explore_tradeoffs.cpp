// step4_explore_tradeoffs.cpp: Explore accuracy/energy/bandwidth trade-offs
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// STEP 4 OF MIXED-PRECISION METHODOLOGY:
// Use ParetoExplorer to find the Pareto-optimal precision configurations.
// Understand the trade-off frontier between accuracy, energy, and bandwidth.
//
// Key concepts:
// - Pareto frontier: configurations where nothing else is better in ALL dimensions
// - 2D analysis: accuracy vs energy
// - 3D analysis: accuracy vs energy vs memory bandwidth
// - Algorithm-aware selection based on arithmetic intensity
//
// Build: part of mp_step4_pareto target
// Run:   ./mp_step4_pareto

#include <universal/utility/directives.hpp>
#include <universal/utility/pareto_explorer.hpp>
#include <universal/utility/algorithm_profiler.hpp>
#include <iostream>
#include <iomanip>

using namespace sw::universal;

int main()
try {
    std::cout << "Step 4: Explore Trade-offs with ParetoExplorer\n";
    std::cout << std::string(60, '=') << "\n\n";

    // Create explorer (pre-loaded with standard configurations)
    ParetoExplorer explorer;

    // =========================================
    // Example 1: Show all configurations
    // =========================================
    std::cout << "Example 1: All Available Precision Configurations\n";
    std::cout << std::string(50, '-') << "\n\n";

    explorer.report(std::cout);

    // =========================================
    // Example 2: 2D Pareto frontier
    // =========================================
    std::cout << "\n\nExample 2: 2D Pareto Frontier (Accuracy vs Energy)\n";
    std::cout << std::string(50, '-') << "\n\n";

    auto result = explorer.computeFrontier();

    std::cout << "Pareto-optimal configurations:\n";
    for (const auto& cfg : result.frontier) {
        std::cout << "  " << std::left << std::setw(16) << cfg.name
                  << "acc=" << std::scientific << std::setprecision(1) << cfg.relative_accuracy
                  << ", energy=" << std::fixed << std::setprecision(2) << cfg.energy_factor << "x\n";
    }

    std::cout << "\nDominated configurations (not on frontier):\n";
    for (const auto& cfg : result.dominated) {
        std::cout << "  " << std::left << std::setw(16) << cfg.name
                  << "acc=" << std::scientific << std::setprecision(1) << cfg.relative_accuracy
                  << ", energy=" << std::fixed << std::setprecision(2) << cfg.energy_factor << "x\n";
    }

    // =========================================
    // Example 3: Query by requirements
    // =========================================
    std::cout << "\n\nExample 3: Query Best Type for Requirements\n";
    std::cout << std::string(50, '-') << "\n\n";

    std::cout << "Best type for 1e-4 accuracy:\n";
    auto best_acc = explorer.recommendForAccuracy(1e-4);
    std::cout << "  " << best_acc.name << " (energy=" << best_acc.energy_factor << "x)\n";

    std::cout << "\nBest type for 0.5x energy budget:\n";
    auto best_energy = explorer.recommendForEnergy(0.5);
    std::cout << "  " << best_energy.name << " (accuracy=" << std::scientific
              << best_energy.relative_accuracy << ")\n";

    std::cout << "\nBest with combined constraints (acc=1e-4, energy<=0.5x, bw<=0.5x):\n";
    auto best_combined = explorer.recommendWithConstraints(1e-4, 0.5, 0.5);
    std::cout << "  " << best_combined.name << "\n";

    // =========================================
    // Example 4: Algorithm-specific recommendations
    // =========================================
    std::cout << "\n\nExample 4: Algorithm-Specific Recommendations\n";
    std::cout << std::string(50, '-') << "\n\n";

    // Create algorithm profiles
    auto dot_product = ParetoExplorer::profileDotProduct(1000000);
    auto gemm_small = ParetoExplorer::profileGEMM(256, 256, 256);
    auto gemm_large = ParetoExplorer::profileGEMM(1024, 1024, 1024);
    auto conv2d = ParetoExplorer::profileConv2D(224, 224, 3, 64, 3);

    struct AlgoTest {
        std::string name;
        AlgorithmCharacteristics profile;
    };

    std::vector<AlgoTest> algos = {
        {"Dot Product (N=1M)", dot_product},
        {"GEMM 256x256", gemm_small},
        {"GEMM 1024x1024", gemm_large},
        {"Conv2D (224x224, 3->64)", conv2d}
    };

    auto result3d = explorer.computeFrontier3D();

    std::cout << std::left << std::setw(28) << "Algorithm"
              << std::right << std::setw(8) << "AI"
              << std::setw(12) << "Type"
              << std::setw(18) << "Best (acc=1e-4)" << "\n";
    std::cout << std::string(66, '-') << "\n";

    for (const auto& algo : algos) {
        auto best = result3d.bestForAlgorithm(1e-4, algo.profile);
        std::string type_str = algo.profile.is_memory_bound ? "mem-bound" : "compute";

        std::cout << std::left << std::setw(28) << algo.name
                  << std::right << std::fixed << std::setprecision(1)
                  << std::setw(8) << algo.profile.arithmetic_intensity
                  << std::setw(12) << type_str
                  << std::setw(18) << best.name << "\n";
    }

    // =========================================
    // Example 5: Roofline analysis
    // =========================================
    std::cout << "\n\nExample 5: Roofline Analysis\n";
    std::cout << std::string(50, '-') << "\n";

    explorer.rooflineAnalysis(std::cout, 100.0);  // 100 GB/s system

    // =========================================
    // Example 6: Visual plot (ASCII)
    // =========================================
    std::cout << "\n\nExample 6: Visual Pareto Plot\n";
    std::cout << std::string(50, '-') << "\n";

    explorer.plotFrontier(std::cout, 50, 15);

    // =========================================
    // Example 7: Adding custom configurations
    // =========================================
    std::cout << "\n\nExample 7: Adding Custom Configurations\n";
    std::cout << std::string(50, '-') << "\n";

    // Add a hypothetical custom type
    explorer.addConfiguration("custom<24,5>", 24, 1e-5, 0.65, 0.75);

    std::cout << "Added: custom<24,5> (24-bit, acc=1e-5, energy=0.65x, bw=0.75x)\n\n";

    auto new_result = explorer.computeFrontier();
    std::cout << "Updated Pareto frontier:\n";
    for (const auto& cfg : new_result.frontier) {
        std::cout << "  " << std::left << std::setw(16) << cfg.name
                  << (cfg.name == "custom<24,5>" ? " <-- NEW" : "") << "\n";
    }

    // =========================================
    // Summary
    // =========================================
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Key Insights:\n";
    std::cout << "  - Pareto frontier shows optimal trade-offs\n";
    std::cout << "  - Memory-bound algorithms benefit from smaller types\n";
    std::cout << "  - Compute-bound algorithms can use higher precision\n";
    std::cout << "  - posit types often on the Pareto frontier\n";
    std::cout << "\nNext Step: Generate configuration code\n";
    std::cout << "See: step5_generate_config.cpp\n";

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
