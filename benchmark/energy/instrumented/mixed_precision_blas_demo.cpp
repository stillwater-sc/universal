// mixed_precision_blas_demo.cpp: demonstrate mixed-precision BLAS operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <cmath>

// Mixed-precision BLAS
#include <blas/mixed_precision.hpp>

using namespace sw::blas;
using namespace sw::universal;

// Generate random test data
std::vector<double> generateRandomVector(size_t n, double min_val = -1.0, double max_val = 1.0) {
    std::vector<double> v(n);
    std::mt19937 gen(42);  // Fixed seed for reproducibility
    std::uniform_real_distribution<> dis(min_val, max_val);
    for (size_t i = 0; i < n; ++i) {
        v[i] = dis(gen);
    }
    return v;
}

void demonstrateDotProduct() {
    std::cout << "========================================\n";
    std::cout << "Mixed-Precision Dot Product\n";
    std::cout << "========================================\n\n";

    // Generate test vectors
    size_t n = 10000;
    auto x = generateRandomVector(n);
    auto y = generateRandomVector(n);

    std::cout << "Vector size: " << n << " elements\n\n";

    // Test different configurations
    auto results = benchmarkMixedPrecisionConfigs(x, y, 1e-4);
    reportMixedPrecisionBenchmark(std::cout, results, 1e-4);
}

void demonstrateGEMM() {
    std::cout << "\n\n========================================\n";
    std::cout << "Mixed-Precision GEMM\n";
    std::cout << "========================================\n\n";

    // Small GEMM for accuracy testing
    size_t m = 64, n = 64, k = 64;

    std::cout << "Matrix dimensions: " << m << " x " << k << " x " << n << "\n\n";

    // Generate test matrices
    auto A_double = generateRandomVector(m * k);
    auto B_double = generateRandomVector(k * n);

    // FP32 reference
    std::vector<float> A_f32(A_double.begin(), A_double.end());
    std::vector<float> B_f32(B_double.begin(), B_double.end());
    std::vector<float> C_f32(m * n, 0.0f);
    MixedPrecisionStats stats_f32;
    mp_gemm<MP_FP32_Only>(m, n, k, 1.0f, A_f32, B_f32, 0.0f, C_f32, &stats_f32);

    // FP16 with FP32 accumulator
    std::vector<half> A_f16(A_double.begin(), A_double.end());
    std::vector<half> B_f16(B_double.begin(), B_double.end());
    std::vector<half> C_f16(m * n, half(0));
    MixedPrecisionStats stats_f16;
    mp_gemm<MP_FP16_Accum32>(m, n, k, half(1), A_f16, B_f16, half(0), C_f16, &stats_f16);

    // Posit16 with Posit32 accumulator
    using p16 = posit<16, 1>;
    std::vector<p16> A_p16(A_double.begin(), A_double.end());
    std::vector<p16> B_p16(B_double.begin(), B_double.end());
    std::vector<p16> C_p16(m * n, p16(0));
    MixedPrecisionStats stats_p16;
    mp_gemm<MP_Posit16_Accum32>(m, n, k, p16(1), A_p16, B_p16, p16(0), C_p16, &stats_p16);

    // Compute Frobenius norm error
    double sum_sq_f16 = 0.0, sum_sq_p16 = 0.0, sum_sq_ref = 0.0;
    for (size_t i = 0; i < m * n; ++i) {
        double ref = static_cast<double>(C_f32[i]);
        double f16 = static_cast<double>(C_f16[i]);
        double p16d = static_cast<double>(C_p16[i]);
        sum_sq_ref += ref * ref;
        sum_sq_f16 += (f16 - ref) * (f16 - ref);
        sum_sq_p16 += (p16d - ref) * (p16d - ref);
    }
    double frob_ref = std::sqrt(sum_sq_ref);
    double rel_err_f16 = std::sqrt(sum_sq_f16) / frob_ref;
    double rel_err_p16 = std::sqrt(sum_sq_p16) / frob_ref;

    // Energy comparison
    auto energy_f32 = compareMixedPrecisionEnergy<MP_FP32_Only>(stats_f32);
    auto energy_f16 = compareMixedPrecisionEnergy<MP_FP16_Accum32>(stats_f16);
    auto energy_p16 = compareMixedPrecisionEnergy<MP_Posit16_Accum32>(stats_p16);

    std::cout << std::left << std::setw(20) << "Configuration"
              << std::right << std::setw(15) << "Rel. Error"
              << std::setw(15) << "Energy Ratio" << "\n";
    std::cout << std::string(50, '-') << "\n";

    std::cout << std::left << std::setw(20) << "FP32 (reference)"
              << std::right << std::scientific << std::setprecision(2) << std::setw(15) << 0.0
              << std::fixed << std::setprecision(3) << std::setw(14) << energy_f32.energy_ratio << "x\n";

    std::cout << std::left << std::setw(20) << "FP16+FP32acc"
              << std::right << std::scientific << std::setw(15) << rel_err_f16
              << std::fixed << std::setprecision(3) << std::setw(14) << energy_f16.energy_ratio << "x\n";

    std::cout << std::left << std::setw(20) << "posit16+posit32acc"
              << std::right << std::scientific << std::setw(15) << rel_err_p16
              << std::fixed << std::setprecision(3) << std::setw(14) << energy_p16.energy_ratio << "x\n";

    std::cout << "\nEnergy savings vs FP32:\n";
    std::cout << "  FP16+FP32acc:     " << std::fixed << std::setprecision(1)
              << energy_f16.savings_percent << "%\n";
    std::cout << "  posit16+32acc:    " << energy_p16.savings_percent << "%\n";
}

void demonstrateMatVec() {
    std::cout << "\n\n========================================\n";
    std::cout << "Mixed-Precision Matrix-Vector Product\n";
    std::cout << "========================================\n\n";

    size_t m = 1024, n = 1024;

    std::cout << "Matrix: " << m << " x " << n << "\n\n";

    auto A = generateRandomVector(m * n);
    auto x = generateRandomVector(n);

    // FP32
    std::vector<float> A_f32(A.begin(), A.end());
    std::vector<float> x_f32(x.begin(), x.end());
    std::vector<float> y_f32(m, 0.0f);
    MixedPrecisionStats stats_f32;
    mp_gemv<MP_FP32_Only>(m, n, 1.0f, A_f32, x_f32, 0.0f, y_f32, &stats_f32);

    // FP16 with FP32 acc
    std::vector<half> A_f16(A.begin(), A.end());
    std::vector<half> x_f16(x.begin(), x.end());
    std::vector<half> y_f16(m, half(0));
    MixedPrecisionStats stats_f16;
    mp_gemv<MP_FP16_Accum32>(m, n, half(1), A_f16, x_f16, half(0), y_f16, &stats_f16);

    // Compute relative error
    double sum_sq_err = 0.0, sum_sq_ref = 0.0;
    for (size_t i = 0; i < m; ++i) {
        double ref = static_cast<double>(y_f32[i]);
        double f16 = static_cast<double>(y_f16[i]);
        sum_sq_ref += ref * ref;
        sum_sq_err += (f16 - ref) * (f16 - ref);
    }
    double rel_err = std::sqrt(sum_sq_err) / std::sqrt(sum_sq_ref);

    auto energy_f32 = compareMixedPrecisionEnergy<MP_FP32_Only>(stats_f32);
    auto energy_f16 = compareMixedPrecisionEnergy<MP_FP16_Accum32>(stats_f16);

    std::cout << "FP32 baseline energy:   " << std::fixed << std::setprecision(2)
              << (energy_f32.single_precision_pj / 1e6) << " uJ\n";
    std::cout << "FP16+FP32acc energy:    " << (energy_f16.mixed_precision_pj / 1e6) << " uJ\n";
    std::cout << "Energy savings:         " << std::setprecision(1)
              << energy_f16.savings_percent << "%\n";
    std::cout << "Relative error:         " << std::scientific << rel_err << "\n";
}

void demonstrateAccuracyVsEnergy() {
    std::cout << "\n\n========================================\n";
    std::cout << "Accuracy vs Energy Trade-off\n";
    std::cout << "========================================\n\n";

    std::vector<size_t> sizes = {100, 1000, 10000, 100000};

    std::cout << std::left << std::setw(10) << "Size"
              << std::right << std::setw(15) << "FP32 Energy"
              << std::setw(15) << "FP16+32 Energy"
              << std::setw(12) << "Savings"
              << std::setw(15) << "FP16 Error" << "\n";
    std::cout << std::string(70, '-') << "\n";

    for (size_t n : sizes) {
        auto x = generateRandomVector(n);
        auto y = generateRandomVector(n);

        // FP32
        std::vector<float> x_f32(x.begin(), x.end());
        std::vector<float> y_f32(y.begin(), y.end());
        MixedPrecisionStats stats_f32;
        mp_dot<MP_FP32_Only>(x_f32, y_f32, &stats_f32);

        // FP16+FP32acc
        std::vector<half> x_f16(x.begin(), x.end());
        std::vector<half> y_f16(y.begin(), y.end());
        MixedPrecisionStats stats_f16;
        auto result_f16 = mp_dot<MP_FP16_Accum32>(x_f16, y_f16, &stats_f16);

        // Reference
        double ref = 0.0;
        for (size_t i = 0; i < n; ++i) ref += x[i] * y[i];

        double rel_err = std::abs(static_cast<double>(result_f16) - ref) / std::abs(ref);

        auto energy_f32 = estimateMixedPrecisionEnergy<MP_FP32_Only>(stats_f32);
        auto energy_f16 = estimateMixedPrecisionEnergy<MP_FP16_Accum32>(stats_f16);
        double savings = (1.0 - energy_f16 / energy_f32) * 100.0;

        std::cout << std::left << std::setw(10) << n
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(14) << (energy_f32 / 1e6) << " uJ"
                  << std::setw(14) << (energy_f16 / 1e6) << " uJ"
                  << std::setprecision(1) << std::setw(11) << savings << "%"
                  << std::scientific << std::setprecision(2) << std::setw(15) << rel_err << "\n";
    }
}

void demonstrateRecommendations() {
    std::cout << "\n\n========================================\n";
    std::cout << "Precision Recommendations by Use Case\n";
    std::cout << "========================================\n\n";

    size_t n = 10000;
    auto x = generateRandomVector(n);
    auto y = generateRandomVector(n);

    struct UseCase {
        std::string name;
        double accuracy_req;
    };

    std::vector<UseCase> use_cases = {
        {"ML Inference", 1e-2},
        {"Graphics/Gaming", 1e-3},
        {"Signal Processing", 1e-4},
        {"CAD/CAM", 1e-6},
        {"Scientific Computing", 1e-8}
    };

    for (const auto& uc : use_cases) {
        std::cout << uc.name << " (accuracy " << std::scientific << std::setprecision(0)
                  << uc.accuracy_req << "):\n";

        auto results = benchmarkMixedPrecisionConfigs(x, y, uc.accuracy_req);

        // Find best
        const MixedPrecisionRecommendation* best = nullptr;
        for (const auto& r : results) {
            if (r.meets_accuracy_requirement) {
                if (!best || r.estimated_energy_ratio < best->estimated_energy_ratio) {
                    best = &r;
                }
            }
        }

        if (best) {
            std::cout << "  -> " << best->config_name
                      << " (energy " << std::fixed << std::setprecision(3)
                      << best->estimated_energy_ratio << "x, error "
                      << std::scientific << best->measured_accuracy << ")\n\n";
        } else {
            std::cout << "  -> No suitable configuration found\n\n";
        }
    }
}

int main()
try {
    std::cout << "Universal Numbers Library: Mixed-Precision BLAS Demo\n";
    std::cout << "================================================================\n\n";

    demonstrateDotProduct();
    demonstrateGEMM();
    demonstrateMatVec();
    demonstrateAccuracyVsEnergy();
    demonstrateRecommendations();

    std::cout << "\n\nKey Takeaways:\n";
    std::cout << "1. Mixed-precision reduces memory bandwidth and compute energy\n";
    std::cout << "2. Higher-precision accumulators maintain accuracy in reductions\n";
    std::cout << "3. Energy savings of 30-50% are typical for FP16+FP32acc\n";
    std::cout << "4. Posits can provide similar accuracy with better dynamic range\n";
    std::cout << "5. The right configuration depends on accuracy requirements\n";

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
