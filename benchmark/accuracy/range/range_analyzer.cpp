// range_analyzer.cpp: test and demonstration of range analysis for precision selection
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// This benchmark demonstrates the range_analyzer utility for determining
// appropriate precision in mixed-precision algorithm design.

#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <cmath>

// Universal number systems for comparison
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>

// Range analyzer, type advisor, and memory profiler
#include <universal/utility/range_analyzer.hpp>
#include <universal/utility/type_advisor.hpp>
#include <universal/utility/memory_profiler.hpp>

using namespace sw::universal;

// Generate test data with known characteristics
std::vector<double> generateNarrowRange(size_t n) {
    // Values clustered around 1.0 with small variations
    std::vector<double> data(n);
    std::mt19937 gen(42);
    std::normal_distribution<> dist(1.0, 0.1);
    for (size_t i = 0; i < n; ++i) {
        data[i] = dist(gen);
    }
    return data;
}

std::vector<double> generateWideRange(size_t n) {
    // Values spanning many orders of magnitude
    std::vector<double> data(n);
    std::mt19937 gen(42);
    std::uniform_real_distribution<> exp_dist(-30, 30);
    std::uniform_real_distribution<> sign_dist(-1, 1);
    for (size_t i = 0; i < n; ++i) {
        double exp = exp_dist(gen);
        double sign = (sign_dist(gen) > 0) ? 1.0 : -1.0;
        data[i] = sign * std::pow(10.0, exp);
    }
    return data;
}

std::vector<double> generateMixedData(size_t n) {
    // Mix of normal values, zeros, and special cases
    std::vector<double> data;
    data.reserve(n);

    std::mt19937 gen(42);
    std::normal_distribution<> normal_dist(0.0, 100.0);

    for (size_t i = 0; i < n; ++i) {
        if (i % 100 == 0) {
            data.push_back(0.0);  // Some zeros
        } else if (i % 500 == 1) {
            data.push_back(std::numeric_limits<double>::infinity());
        } else if (i % 500 == 2) {
            data.push_back(-std::numeric_limits<double>::infinity());
        } else if (i % 1000 == 3) {
            data.push_back(std::numeric_limits<double>::quiet_NaN());
        } else if (i % 200 == 4) {
            data.push_back(1e-310);  // Subnormal
        } else {
            data.push_back(normal_dist(gen));
        }
    }
    return data;
}

void demonstrateBasicUsage() {
    std::cout << "========================================\n";
    std::cout << "Basic Range Analyzer Usage\n";
    std::cout << "========================================\n\n";

    // Create analyzer and observe some values
    range_analyzer<double> analyzer;

    std::vector<double> values = {1.0, 2.5, -3.7, 100.0, 0.001, -0.0001};
    for (double v : values) {
        analyzer.observe(v);
    }

    analyzer.report(std::cout);
}

void demonstrateNarrowRange() {
    std::cout << "\n========================================\n";
    std::cout << "Narrow Range Data Analysis\n";
    std::cout << "========================================\n\n";

    auto data = generateNarrowRange(10000);
    auto analyzer = analyzeRange(data);

    std::cout << "Data: 10000 samples ~ N(1.0, 0.1)\n\n";
    analyzer.report(std::cout);

    std::cout << "\nInterpretation:\n";
    std::cout << "- Small scale span suggests low-precision type is sufficient\n";
    std::cout << "- High DR utilization waste suggests using smaller type\n";
}

void demonstrateWideRange() {
    std::cout << "\n========================================\n";
    std::cout << "Wide Dynamic Range Data Analysis\n";
    std::cout << "========================================\n\n";

    auto data = generateWideRange(10000);
    auto analyzer = analyzeRange(data);

    std::cout << "Data: 10000 samples spanning 10^-30 to 10^30\n\n";
    analyzer.report(std::cout);

    std::cout << "\nInterpretation:\n";
    std::cout << "- Large scale span requires more exponent bits\n";
    std::cout << "- Consider posit for better dynamic range utilization\n";
}

void demonstrateMixedData() {
    std::cout << "\n========================================\n";
    std::cout << "Mixed Data with Special Values\n";
    std::cout << "========================================\n\n";

    auto data = generateMixedData(10000);
    auto analyzer = analyzeRange(data);

    std::cout << "Data: 10000 samples with zeros, infinities, NaNs, subnormals\n\n";
    analyzer.report(std::cout);

    std::cout << "\nInterpretation:\n";
    std::cout << "- Presence of subnormals suggests need for gradual underflow\n";
    std::cout << "- NaNs and infinities indicate potential numerical issues\n";
}

void demonstrateTypeComparison() {
    std::cout << "\n========================================\n";
    std::cout << "Type Compatibility Analysis\n";
    std::cout << "========================================\n\n";

    // Analyze data in double precision
    auto data = generateNarrowRange(1000);
    range_analyzer<double> analyzer;
    for (double v : data) {
        analyzer.observe(v);
    }

    std::cout << "Source data analyzed as double:\n";
    std::cout << "  " << analyzer.summary() << "\n\n";

    // Check compatibility with various target types
    std::cout << "Compatibility with target types:\n\n";

    compareRanges<double, float>(analyzer, std::cout);
    std::cout << "\n";

    // Manual check for half precision range
    std::cout << "Target: half (cfloat<16,5>)\n";
    std::cout << std::string(40, '-') << "\n";
    double half_max = 65504.0;  // Max value for IEEE half
    double half_min = 6.1e-5;   // Min normal for IEEE half
    double src_min = static_cast<double>(analyzer.minAbsValue());
    double src_max = static_cast<double>(analyzer.maxAbsValue());
    std::cout << std::scientific << std::setprecision(3);
    std::cout << "Source range:  [" << src_min << ", " << src_max << "]\n";
    std::cout << "Target range:  [" << half_min << ", " << half_max << "]\n";
    bool fits_half = (src_min >= half_min || src_min == 0) && (src_max <= half_max);
    std::cout << "Fits in target: " << (fits_half ? "YES" : "NO") << "\n";
}

void demonstrateAlgorithmAnalysis() {
    std::cout << "\n========================================\n";
    std::cout << "Algorithm Range Analysis: Dot Product\n";
    std::cout << "========================================\n\n";

    // Simulate a dot product computation tracking intermediate values
    range_analyzer<double> input_analyzer;
    range_analyzer<double> product_analyzer;
    range_analyzer<double> accumulator_analyzer;

    std::mt19937 gen(42);
    std::uniform_real_distribution<> dist(-10.0, 10.0);

    const size_t N = 1000;
    double accumulator = 0.0;

    for (size_t i = 0; i < N; ++i) {
        double a = dist(gen);
        double b = dist(gen);

        input_analyzer.observe(a);
        input_analyzer.observe(b);

        double product = a * b;
        product_analyzer.observe(product);

        accumulator += product;
        accumulator_analyzer.observe(accumulator);
    }

    std::cout << "Dot product of " << N << " element vectors:\n\n";

    std::cout << "INPUT VALUES:\n";
    std::cout << "  " << input_analyzer.summary() << "\n";
    auto input_rec = input_analyzer.recommendPrecision();
    std::cout << "  Recommendation: " << input_rec.type_suggestion << "\n\n";

    std::cout << "PRODUCTS (a[i] * b[i]):\n";
    std::cout << "  " << product_analyzer.summary() << "\n";
    auto prod_rec = product_analyzer.recommendPrecision();
    std::cout << "  Recommendation: " << prod_rec.type_suggestion << "\n\n";

    std::cout << "ACCUMULATOR (running sum):\n";
    std::cout << "  " << accumulator_analyzer.summary() << "\n";
    auto acc_rec = accumulator_analyzer.recommendPrecision();
    std::cout << "  Recommendation: " << acc_rec.type_suggestion << "\n\n";

    std::cout << "Mixed-Precision Strategy:\n";
    std::cout << "  - Inputs: " << input_rec.recommended_bits << "-bit\n";
    std::cout << "  - Products: " << prod_rec.recommended_bits << "-bit\n";
    std::cout << "  - Accumulator: " << acc_rec.recommended_bits << "-bit\n";
}

void demonstrateMemoryProfiler() {
    std::cout << "\n========================================\n";
    std::cout << "Memory Profiler Analysis\n";
    std::cout << "========================================\n\n";

    // Profile GEMM at different sizes and precisions
    std::cout << "GEMM Memory Analysis (C = A * B):\n";
    std::cout << std::string(60, '-') << "\n";

    std::cout << std::left << std::setw(15) << "Size"
              << std::setw(12) << "Precision"
              << std::setw(12) << "Working Set"
              << std::setw(12) << "Cache Tier"
              << std::setw(15) << "Memory Energy" << "\n";
    std::cout << std::string(60, '-') << "\n";

    auto printGEMM = [](const char* size_str, uint64_t M, uint64_t N, uint64_t K,
                        const char* prec, size_t elem_size) {
        auto profile = profileGEMM(M, N, K, elem_size);
        std::cout << std::left << std::setw(15) << size_str
                  << std::setw(12) << prec
                  << std::setw(12) << profile.summary().substr(3, 8)  // Extract WS
                  << std::setw(12) << memoryTierName(profile.estimatePrimaryTier())
                  << std::fixed << std::setprecision(2)
                  << profile.estimateEnergyUJ() << " uJ\n";
    };

    // Small matrix (fits in L1)
    printGEMM("64x64", 64, 64, 64, "FP32", 4);
    printGEMM("64x64", 64, 64, 64, "FP16", 2);
    printGEMM("64x64", 64, 64, 64, "INT8", 1);

    // Medium matrix (fits in L2/L3)
    printGEMM("256x256", 256, 256, 256, "FP32", 4);
    printGEMM("256x256", 256, 256, 256, "FP16", 2);
    printGEMM("256x256", 256, 256, 256, "INT8", 1);

    // Large matrix (spills to DRAM)
    printGEMM("1024x1024", 1024, 1024, 1024, "FP32", 4);
    printGEMM("1024x1024", 1024, 1024, 1024, "FP16", 2);
    printGEMM("1024x1024", 1024, 1024, 1024, "INT8", 1);

    // Very large matrix
    printGEMM("4096x4096", 4096, 4096, 4096, "FP32", 4);
    printGEMM("4096x4096", 4096, 4096, 4096, "FP16", 2);

    std::cout << "\nDetailed profile for 1024x1024 FP32 GEMM:\n";
    auto profile = profileGEMM(1024, 1024, 1024, 4);
    profile.report(std::cout);

    // Compare dot product at different sizes
    std::cout << "\n\nDot Product Memory Analysis:\n";
    std::cout << std::string(50, '-') << "\n";

    std::cout << std::left << std::setw(15) << "Vector Size"
              << std::setw(12) << "Working Set"
              << std::setw(12) << "Cache Tier"
              << std::setw(15) << "Memory Energy" << "\n";
    std::cout << std::string(50, '-') << "\n";

    for (uint64_t n : {1000ULL, 10000ULL, 100000ULL, 1000000ULL}) {
        auto dp = profileDotProduct(n, 4);  // FP32
        std::stringstream ss;
        ss << n;
        std::cout << std::left << std::setw(15) << ss.str()
                  << std::setw(12) << dp.summary().substr(3, 8)
                  << std::setw(12) << memoryTierName(dp.estimatePrimaryTier())
                  << std::fixed << std::setprecision(4)
                  << dp.estimateEnergyUJ() << " uJ\n";
    }

    std::cout << "\nKey insight: Memory energy dominates for large working sets!\n";
    std::cout << "Reducing precision from FP32 to FP16 halves memory traffic.\n";
}

void demonstrateTypeAdvisor() {
    std::cout << "\n========================================\n";
    std::cout << "Type Advisor Recommendations\n";
    std::cout << "========================================\n\n";

    TypeAdvisor advisor;

    // Scenario 1: Narrow range, high accuracy
    std::cout << "Scenario 1: Narrow range, high accuracy (1e-6)\n";
    std::cout << std::string(50, '-') << "\n";
    {
        auto data = generateNarrowRange(1000);
        auto analyzer = analyzeRange(data);

        AccuracyRequirement acc(1e-6);  // Need high accuracy
        advisor.report(std::cout, analyzer, acc);
    }

    // Scenario 2: Wide range, moderate accuracy
    std::cout << "\nScenario 2: Wide range, moderate accuracy (1e-3)\n";
    std::cout << std::string(50, '-') << "\n";
    {
        auto data = generateWideRange(1000);
        auto analyzer = analyzeRange(data);

        AccuracyRequirement acc(1e-3);  // Moderate accuracy OK
        advisor.report(std::cout, analyzer, acc);
    }

    // Scenario 3: ML inference (low accuracy OK, energy matters)
    std::cout << "\nScenario 3: ML inference (1e-2 accuracy, energy-focused)\n";
    std::cout << std::string(50, '-') << "\n";
    {
        std::vector<double> weights(1000);
        std::mt19937 gen(42);
        std::normal_distribution<> dist(0.0, 0.1);  // Small weights typical in ML
        for (auto& w : weights) w = dist(gen);

        auto analyzer = analyzeRange(weights);

        AccuracyRequirement acc(1e-2);  // Low accuracy OK for inference
        advisor.report(std::cout, analyzer, acc);
    }
}

void demonstrateMatrixAnalysis() {
    std::cout << "\n========================================\n";
    std::cout << "Matrix Operation Range Analysis\n";
    std::cout << "========================================\n\n";

    // Simulate matrix multiply tracking element ranges
    const size_t M = 100, N = 100, K = 100;

    range_analyzer<double> A_analyzer, B_analyzer, C_analyzer;

    std::mt19937 gen(42);
    std::normal_distribution<> dist(0.0, 1.0);

    // Generate random matrices
    std::vector<double> A(M * K), B(K * N), C(M * N, 0.0);

    for (size_t i = 0; i < M * K; ++i) {
        A[i] = dist(gen);
        A_analyzer.observe(A[i]);
    }
    for (size_t i = 0; i < K * N; ++i) {
        B[i] = dist(gen);
        B_analyzer.observe(B[i]);
    }

    // Compute C = A * B and track ranges
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < K; ++k) {
                sum += A[i * K + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
            C_analyzer.observe(sum);
        }
    }

    std::cout << "Matrix multiply: C[" << M << "x" << N << "] = A[" << M << "x" << K
              << "] * B[" << K << "x" << N << "]\n\n";

    std::cout << "Matrix A: " << A_analyzer.summary() << "\n";
    std::cout << "Matrix B: " << B_analyzer.summary() << "\n";
    std::cout << "Matrix C: " << C_analyzer.summary() << "\n\n";

    auto A_rec = A_analyzer.recommendPrecision();
    auto B_rec = B_analyzer.recommendPrecision();
    auto C_rec = C_analyzer.recommendPrecision();

    std::cout << "Precision Recommendations:\n";
    std::cout << "  Matrix A: " << A_rec.type_suggestion << "\n";
    std::cout << "  Matrix B: " << B_rec.type_suggestion << "\n";
    std::cout << "  Matrix C: " << C_rec.type_suggestion << "\n\n";

    std::cout << "Note: C has wider range due to accumulation of " << K << " products\n";
    std::cout << "Consider using higher precision for accumulation (mixed-precision GEMM)\n";
}

int main()
try {
    std::cout << "Universal Numbers Library: Range Analyzer\n";
    std::cout << "=========================================\n\n";

    demonstrateBasicUsage();
    demonstrateNarrowRange();
    demonstrateWideRange();
    demonstrateMixedData();
    demonstrateTypeComparison();
    demonstrateAlgorithmAnalysis();
    demonstrateMatrixAnalysis();

    demonstrateTypeAdvisor();
    demonstrateMemoryProfiler();

    std::cout << "\n\nKey Takeaways:\n";
    std::cout << "1. Range analysis helps select appropriate precision per variable\n";
    std::cout << "2. Intermediate values (products, accumulators) often need higher precision\n";
    std::cout << "3. Narrow dynamic range allows aggressive precision reduction\n";
    std::cout << "4. Track ranges at each computation stage for optimal mixed-precision\n";
    std::cout << "5. Type advisor recommends specific Universal types based on requirements\n";
    std::cout << "6. Memory energy dominates for large working sets - reduce precision!\n";

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
