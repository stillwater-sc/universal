// algorithm_profiler.cpp: demonstrate algorithm profiling and Pareto analysis
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

#include <iostream>
#include <iomanip>
#include <vector>

// Algorithm profiler, Pareto explorer, and config generator
#include <universal/utility/algorithm_profiler.hpp>
#include <universal/utility/pareto_explorer.hpp>
#include <universal/utility/precision_config_generator.hpp>

using namespace sw::universal;

void demonstrateAlgorithmProfiler() {
    std::cout << "========================================\n";
    std::cout << "Algorithm Profiler Demonstration\n";
    std::cout << "========================================\n\n";

    // Profile GEMM at different precisions
    std::cout << "GEMM 1024x1024 at different precisions:\n";
    std::cout << std::string(60, '-') << "\n\n";

    auto gemm_fp64 = AlgorithmProfiler::profileGEMM(1024, 1024, 1024, "FP64", 64);
    auto gemm_fp32 = AlgorithmProfiler::profileGEMM(1024, 1024, 1024, "FP32", 32);
    auto gemm_fp16 = AlgorithmProfiler::profileGEMM(1024, 1024, 1024, "FP16", 16);
    auto gemm_int8 = AlgorithmProfiler::profileGEMM(1024, 1024, 1024, "INT8", 8);

    std::vector<AlgorithmProfile> gemm_profiles = {gemm_fp64, gemm_fp32, gemm_fp16, gemm_int8};
    AlgorithmProfiler::compareMultiple(std::cout, gemm_profiles);

    // Detailed profile for FP32
    std::cout << "\n\nDetailed FP32 GEMM Profile:\n";
    gemm_fp32.report(std::cout);

    // Compare FP32 vs FP16
    std::cout << "\n\nFP32 vs FP16 Comparison:\n";
    auto comparison = AlgorithmProfiler::compare(gemm_fp32, gemm_fp16);
    comparison.report(std::cout);
}

void demonstrateDotProduct() {
    std::cout << "\n\n========================================\n";
    std::cout << "Dot Product Analysis\n";
    std::cout << "========================================\n\n";

    std::vector<uint64_t> sizes = {1000, 10000, 100000, 1000000};

    std::cout << "Dot product at different sizes (FP32):\n";
    std::cout << std::string(70, '-') << "\n";
    std::cout << std::left << std::setw(12) << "Size"
              << std::right << std::setw(15) << "Operations"
              << std::setw(12) << "Memory"
              << std::setw(12) << "Cache Tier"
              << std::setw(15) << "Energy (uJ)" << "\n";
    std::cout << std::string(70, '-') << "\n";

    for (uint64_t n : sizes) {
        auto profile = AlgorithmProfiler::profileDotProduct(n, "FP32", 32);
        std::cout << std::left << std::setw(12) << n
                  << std::right << std::setw(15) << profile.total_ops
                  << std::setw(12) << (profile.working_set_bytes / 1024) << " KB"
                  << std::setw(12) << profile.primary_cache_tier
                  << std::fixed << std::setprecision(4)
                  << std::setw(15) << (profile.total_energy_pj / 1e6) << "\n";
    }
}

void demonstrateConv2D() {
    std::cout << "\n\n========================================\n";
    std::cout << "Conv2D Analysis (ML Inference)\n";
    std::cout << "========================================\n\n";

    // Typical ResNet-like layer sizes
    struct ConvConfig {
        uint64_t H, W, C_in, C_out, K;
        const char* name;
    };

    std::vector<ConvConfig> layers = {
        {224, 224, 3, 64, 7, "conv1 (7x7)"},
        {56, 56, 64, 128, 3, "conv2 (3x3)"},
        {28, 28, 128, 256, 3, "conv3 (3x3)"},
        {14, 14, 256, 512, 3, "conv4 (3x3)"},
        {7, 7, 512, 512, 3, "conv5 (3x3)"}
    };

    std::cout << "ResNet-like layers at different precisions:\n";
    std::cout << std::string(80, '-') << "\n";
    std::cout << std::left << std::setw(18) << "Layer"
              << std::right << std::setw(12) << "FP32 (uJ)"
              << std::setw(12) << "FP16 (uJ)"
              << std::setw(12) << "INT8 (uJ)"
              << std::setw(15) << "FP16 Savings"
              << std::setw(15) << "INT8 Savings" << "\n";
    std::cout << std::string(80, '-') << "\n";

    double total_fp32 = 0, total_fp16 = 0, total_int8 = 0;

    for (const auto& layer : layers) {
        auto fp32 = AlgorithmProfiler::profileConv2D(
            layer.H, layer.W, layer.C_in, layer.C_out, layer.K, "FP32", 32);
        auto fp16 = AlgorithmProfiler::profileConv2D(
            layer.H, layer.W, layer.C_in, layer.C_out, layer.K, "FP16", 16);
        auto int8 = AlgorithmProfiler::profileConv2D(
            layer.H, layer.W, layer.C_in, layer.C_out, layer.K, "INT8", 8);

        double fp32_uj = fp32.total_energy_pj / 1e6;
        double fp16_uj = fp16.total_energy_pj / 1e6;
        double int8_uj = int8.total_energy_pj / 1e6;

        total_fp32 += fp32_uj;
        total_fp16 += fp16_uj;
        total_int8 += int8_uj;

        std::cout << std::left << std::setw(18) << layer.name
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(12) << fp32_uj
                  << std::setw(12) << fp16_uj
                  << std::setw(12) << int8_uj
                  << std::setw(14) << ((1.0 - fp16_uj/fp32_uj) * 100) << "%"
                  << std::setw(14) << ((1.0 - int8_uj/fp32_uj) * 100) << "%\n";
    }

    std::cout << std::string(80, '-') << "\n";
    std::cout << std::left << std::setw(18) << "TOTAL"
              << std::right << std::fixed << std::setprecision(2)
              << std::setw(12) << total_fp32
              << std::setw(12) << total_fp16
              << std::setw(12) << total_int8
              << std::setw(14) << ((1.0 - total_fp16/total_fp32) * 100) << "%"
              << std::setw(14) << ((1.0 - total_int8/total_fp32) * 100) << "%\n";
}

void demonstrateParetoAnalysis() {
    std::cout << "\n\n========================================\n";
    std::cout << "Pareto Analysis: Accuracy vs Energy\n";
    std::cout << "========================================\n\n";

    ParetoExplorer explorer;
    explorer.report(std::cout);
    explorer.plotFrontier(std::cout);
}

void demonstrateMixedPrecisionRecommendation() {
    std::cout << "\n\n========================================\n";
    std::cout << "Mixed-Precision Recommendations\n";
    std::cout << "========================================\n\n";

    std::vector<std::pair<std::string, double>> scenarios = {
        {"ML Training", 1e-4},
        {"ML Inference", 1e-2},
        {"Scientific Computing", 1e-10},
        {"Real-time Graphics", 1e-3},
        {"Financial Modeling", 1e-12}
    };

    for (const auto& scenario : scenarios) {
        auto rec = recommendMixedPrecision(scenario.first, scenario.second);

        std::cout << scenario.first << " (accuracy " << std::scientific
                  << scenario.second << "):\n";
        std::cout << "  Input:       " << rec.input_precision.name << "\n";
        std::cout << "  Compute:     " << rec.compute_precision.name << "\n";
        std::cout << "  Accumulator: " << rec.accumulator_precision.name << "\n";
        std::cout << "  Output:      " << rec.output_precision.name << "\n";
        std::cout << "  Est. energy: " << std::fixed << std::setprecision(2)
                  << rec.estimated_energy_factor << "x (vs all-FP32)\n\n";
    }
}

void demonstrateConfigGenerator() {
    std::cout << "\n\n========================================\n";
    std::cout << "Precision Configuration Generator\n";
    std::cout << "========================================\n\n";

    // Generate config for ML inference
    PrecisionConfigGenerator gen;
    gen.setAlgorithm("ML_Inference_GEMM");
    gen.setAccuracyRequirement(1e-2);
    gen.setEnergyBudget(0.3);
    gen.setProblemSize("batch=32, M=1024, N=1024, K=1024");

    std::cout << "Configuration for ML Inference:\n";
    std::cout << std::string(50, '-') << "\n";
    std::cout << gen.generateComparisonReport() << "\n";

    std::cout << "Generated Header (excerpt):\n";
    std::cout << std::string(50, '-') << "\n";
    std::string header = gen.generateConfigHeader();
    // Print first 30 lines
    std::istringstream iss(header);
    std::string line;
    int line_count = 0;
    while (std::getline(iss, line) && line_count < 35) {
        std::cout << line << "\n";
        ++line_count;
    }

    // Generate config for scientific computing
    std::cout << "\n\nConfiguration for Scientific Computing:\n";
    std::cout << std::string(50, '-') << "\n";
    gen.setAlgorithm("Scientific_DGEMM");
    gen.setAccuracyRequirement(1e-10);
    gen.setEnergyBudget(1.0);
    std::cout << gen.generateComparisonReport();
}

void demonstrateGEMMRecommendation() {
    std::cout << "\n\n========================================\n";
    std::cout << "GEMM Precision Recommendations\n";
    std::cout << "========================================\n\n";

    std::cout << "Recommended precision for 1024x1024 GEMM:\n";
    std::cout << std::string(50, '-') << "\n";

    std::vector<double> accuracy_requirements = {1e-2, 1e-4, 1e-7, 1e-10};

    for (double acc : accuracy_requirements) {
        auto rec = recommendGEMMPrecision(1024, 1024, 1024, acc);
        std::cout << "  Accuracy " << std::scientific << std::setprecision(0) << acc
                  << ": " << rec.name
                  << " (energy " << std::fixed << std::setprecision(2)
                  << rec.energy_factor << "x)\n";
    }
}

int main()
try {
    std::cout << "Universal Numbers Library: Algorithm Profiler & Pareto Analysis\n";
    std::cout << "================================================================\n\n";

    demonstrateAlgorithmProfiler();
    demonstrateDotProduct();
    demonstrateConv2D();
    demonstrateParetoAnalysis();
    demonstrateMixedPrecisionRecommendation();
    demonstrateGEMMRecommendation();

    demonstrateConfigGenerator();

    std::cout << "\n\nKey Takeaways:\n";
    std::cout << "1. Algorithm profiler combines compute, memory, and energy analysis\n";
    std::cout << "2. Pareto frontier shows optimal accuracy/energy trade-offs\n";
    std::cout << "3. Mixed-precision strategies can reduce energy by 50-80%\n";
    std::cout << "4. For ML inference, INT8 saves ~75% energy vs FP32\n";
    std::cout << "5. Memory energy often dominates for large working sets\n";
    std::cout << "6. Config generator produces ready-to-use type definitions\n";

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
