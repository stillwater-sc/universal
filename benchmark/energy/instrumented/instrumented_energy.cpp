// instrumented_energy.cpp: demonstrate instrumented number types with energy estimation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

#include <iostream>
#include <iomanip>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>

// Universal number types
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>

// Instrumentation and energy estimation
#include <universal/utility/instrumented.hpp>
#include <universal/energy/occurrence_energy.hpp>

using namespace sw::universal;
using namespace sw::universal::energy;

// Dot product algorithm - works with any numeric type
template<typename Real>
Real dot_product(const std::vector<Real>& a, const std::vector<Real>& b) {
    Real sum = Real(0);
    for (size_t i = 0; i < a.size(); ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

// Matrix-vector multiply (simplified 1D representation)
template<typename Real>
std::vector<Real> matvec(const std::vector<Real>& A, const std::vector<Real>& x, size_t n) {
    std::vector<Real> y(n);
    for (size_t i = 0; i < n; ++i) {
        y[i] = Real(0);
        for (size_t j = 0; j < n; ++j) {
            y[i] += A[i * n + j] * x[j];
        }
    }
    return y;
}

// Newton-Raphson square root
template<typename Real>
Real newton_sqrt(Real x, int iterations = 5) {
    if (x <= Real(0)) return Real(0);
    Real guess = x / Real(2);
    for (int i = 0; i < iterations; ++i) {
        guess = (guess + x / guess) / Real(2);
    }
    return guess;
}

// Polynomial evaluation using Horner's method
template<typename Real>
Real horner(const std::vector<Real>& coeffs, Real x) {
    Real result = coeffs.back();
    for (int i = static_cast<int>(coeffs.size()) - 2; i >= 0; --i) {
        result = result * x + coeffs[i];
    }
    return result;
}

void demonstrateDotProduct() {
    std::cout << "========================================\n";
    std::cout << "Dot Product Energy Analysis\n";
    std::cout << "========================================\n\n";

    constexpr size_t N = 1000;

    // Test with different types
    std::cout << "Vector size: " << N << "\n\n";

    // --- Float (baseline) ---
    {
        instrumented_stats::reset();

        std::vector<instrumented<float>> a(N), b(N);
        for (size_t i = 0; i < N; ++i) {
            a[i] = static_cast<float>(i) * 0.001f;
            b[i] = static_cast<float>(N - i) * 0.001f;
        }

        auto result = dot_product(a, b);

        std::cout << "Float (32-bit):\n";
        std::cout << "  Result: " << result << "\n";

        auto stats = instrumented_stats::snapshot<float>();
        const auto& model = getIntelSkylakeModel();
        double energy_pj = calculateEnergy(stats, model, BitWidth::bits_32);

        std::cout << "  Operations: " << stats.add << " adds, " << stats.mul << " muls\n";
        std::cout << "  Energy: " << std::fixed << std::setprecision(2)
                  << (energy_pj / 1000.0) << " nJ\n\n";
    }

    // --- cfloat<16,5> (half precision) ---
    {
        using half = cfloat<16, 5, uint16_t, true, false, false>;
        instrumented_stats::reset();

        std::vector<instrumented<half>> a(N), b(N);
        for (size_t i = 0; i < N; ++i) {
            a[i] = half(static_cast<float>(i) * 0.001f);
            b[i] = half(static_cast<float>(N - i) * 0.001f);
        }

        auto result = dot_product(a, b);

        std::cout << "cfloat<16,5> (half precision):\n";
        std::cout << "  Result: " << float(result) << "\n";

        auto stats = instrumented_stats::snapshot<half>();
        const auto& model = getIntelSkylakeModel();
        double energy_pj = calculateEnergy(stats, model, BitWidth::bits_16);

        std::cout << "  Operations: " << stats.add << " adds, " << stats.mul << " muls\n";
        std::cout << "  Energy: " << std::fixed << std::setprecision(2)
                  << (energy_pj / 1000.0) << " nJ\n\n";
    }

    // --- posit<32,2> ---
    {
        using p32 = posit<32, 2>;
        instrumented_stats::reset();

        std::vector<instrumented<p32>> a(N), b(N);
        for (size_t i = 0; i < N; ++i) {
            a[i] = p32(static_cast<float>(i) * 0.001f);
            b[i] = p32(static_cast<float>(N - i) * 0.001f);
        }

        auto result = dot_product(a, b);

        std::cout << "posit<32,2>:\n";
        std::cout << "  Result: " << float(result) << "\n";

        auto stats = instrumented_stats::snapshot<p32>();
        const auto& model = getIntelSkylakeModel();
        double energy_pj = calculateEnergy(stats, model, BitWidth::bits_32);

        std::cout << "  Operations: " << stats.add << " adds, " << stats.mul << " muls\n";
        std::cout << "  Energy: " << std::fixed << std::setprecision(2)
                  << (energy_pj / 1000.0) << " nJ\n\n";
    }
}

void demonstrateMatvec() {
    std::cout << "\n========================================\n";
    std::cout << "Matrix-Vector Multiply Energy Analysis\n";
    std::cout << "========================================\n\n";

    constexpr size_t N = 64;  // 64x64 matrix

    std::cout << "Matrix size: " << N << "x" << N << "\n\n";

    // --- Float ---
    {
        instrumented_stats::reset();

        std::vector<instrumented<float>> A(N * N), x(N);
        for (size_t i = 0; i < N * N; ++i) {
            A[i] = static_cast<float>(i % 100) * 0.01f;
        }
        for (size_t i = 0; i < N; ++i) {
            x[i] = static_cast<float>(i) * 0.1f;
        }

        auto y = matvec(A, x, N);

        std::cout << "Float (32-bit):\n";

        auto stats = instrumented_stats::snapshot<float>();
        const auto& model = getIntelSkylakeModel();
        double energy_pj = calculateEnergy(stats, model, BitWidth::bits_32);

        std::cout << "  Operations: " << stats.add << " adds, " << stats.mul << " muls\n";
        std::cout << "  Expected: " << (N * N) << " adds, " << (N * N) << " muls (O(n²))\n";
        std::cout << "  Energy: " << std::fixed << std::setprecision(2)
                  << (energy_pj / 1000.0) << " nJ\n\n";
    }

    // --- cfloat<16,5> ---
    {
        using half = cfloat<16, 5, uint16_t, true, false, false>;
        instrumented_stats::reset();

        std::vector<instrumented<half>> A(N * N), x(N);
        for (size_t i = 0; i < N * N; ++i) {
            A[i] = half(static_cast<float>(i % 100) * 0.01f);
        }
        for (size_t i = 0; i < N; ++i) {
            x[i] = half(static_cast<float>(i) * 0.1f);
        }

        auto y = matvec(A, x, N);

        std::cout << "cfloat<16,5> (half):\n";

        auto stats = instrumented_stats::snapshot<half>();
        const auto& model = getIntelSkylakeModel();
        double energy_pj = calculateEnergy(stats, model, BitWidth::bits_16);

        std::cout << "  Operations: " << stats.add << " adds, " << stats.mul << " muls\n";
        std::cout << "  Energy: " << std::fixed << std::setprecision(2)
                  << (energy_pj / 1000.0) << " nJ (";

        // Calculate savings
        instrumented_stats::reset();
        std::vector<instrumented<float>> Af(N * N), xf(N);
        for (size_t i = 0; i < N * N; ++i) Af[i] = static_cast<float>(i % 100) * 0.01f;
        for (size_t i = 0; i < N; ++i) xf[i] = static_cast<float>(i) * 0.1f;
        auto yf = matvec(Af, xf, N);
        auto stats_f = instrumented_stats::snapshot<float>();
        double energy_f = calculateEnergy(stats_f, model, BitWidth::bits_32);

        double savings = (1.0 - energy_pj / energy_f) * 100;
        std::cout << std::setprecision(1) << savings << "% savings vs FP32)\n\n";
    }
}

void demonstrateNewtonSqrt() {
    std::cout << "\n========================================\n";
    std::cout << "Newton-Raphson Sqrt Energy Analysis\n";
    std::cout << "========================================\n\n";

    constexpr int iterations = 10;
    float test_value = 2.0f;

    std::cout << "Computing sqrt(" << test_value << ") with " << iterations << " iterations\n\n";

    // --- Float ---
    {
        instrumented_stats::reset();

        instrumented<float> x = test_value;
        auto result = newton_sqrt(x, iterations);

        std::cout << "Float (32-bit):\n";
        std::cout << "  Result: " << result << " (exact: " << std::sqrt(test_value) << ")\n";

        auto stats = instrumented_stats::snapshot<float>();
        instrumented_stats::report(std::cout);

        const auto& model = getIntelSkylakeModel();
        auto breakdown = calculateEnergyBreakdown(stats, model, BitWidth::bits_32);

        std::cout << "\n  Energy breakdown:\n";
        std::cout << "    Compute: " << std::fixed << std::setprecision(2)
                  << breakdown.computeEnergy() << " pJ\n";
        std::cout << "    Memory:  " << breakdown.memoryEnergy() << " pJ\n";
        std::cout << "    Total:   " << breakdown.total_energy << " pJ\n\n";
    }
}

void demonstratePolynomialEval() {
    std::cout << "\n========================================\n";
    std::cout << "Polynomial Evaluation (Horner's Method)\n";
    std::cout << "========================================\n\n";

    // Evaluate p(x) = 1 + 2x + 3x² + 4x³ + 5x⁴ + 6x⁵
    std::vector<float> coeffs_f = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    float x_val = 0.5f;

    std::cout << "Polynomial: 1 + 2x + 3x² + 4x³ + 5x⁴ + 6x⁵\n";
    std::cout << "Evaluating at x = " << x_val << "\n\n";

    // --- Float ---
    {
        instrumented_stats::reset();

        std::vector<instrumented<float>> coeffs(coeffs_f.begin(), coeffs_f.end());
        instrumented<float> x = x_val;

        auto result = horner(coeffs, x);

        std::cout << "Float (32-bit):\n";
        std::cout << "  Result: " << result << "\n";

        auto stats = instrumented_stats::snapshot<float>();
        std::cout << "  Operations: " << stats.add << " adds, " << stats.mul << " muls\n";
        std::cout << "  (Horner's: n-1 muls, n-1 adds for degree n polynomial)\n\n";
    }
}

void demonstrateEnergyComparison() {
    std::cout << "\n========================================\n";
    std::cout << "Energy Comparison Across Architectures\n";
    std::cout << "========================================\n\n";

    // Simulate a fixed workload: 10000 FMAs (add + mul)
    constexpr size_t N = 10000;

    // Get all architecture models
    const auto& skylake = getIntelSkylakeModel();
    const auto& zen3 = getAmdZen3Model();
    const auto& zen4 = getAmdZen4Model();
    const auto& arm_a76 = getArmCortexA76Model();
    const auto& m1 = getAppleM1Model();
    const auto& m3 = getAppleM3Model();

    std::cout << "Simulating " << N << " FMA operations (add + mul)\n\n";

    // Create synthetic occurrence
    occurrence<void> ops;
    ops.add = N;
    ops.mul = N;
    ops.load = N * 3;  // 3 operands per FMA
    ops.store = N;     // 1 result

    // Table header
    std::cout << std::left << std::setw(22) << "Architecture"
              << std::right << std::setw(10) << "16-bit"
              << std::setw(10) << "32-bit"
              << std::setw(10) << "64-bit"
              << std::setw(12) << "vs Skylake" << "\n";
    std::cout << std::string(64, '-') << "\n";

    auto printRow = [&](const char* name, const EnergyCostModel& model) {
        double e16 = calculateEnergy(ops, model, BitWidth::bits_16);
        double e32 = calculateEnergy(ops, model, BitWidth::bits_32);
        double e64 = calculateEnergy(ops, model, BitWidth::bits_64);
        double skylake_e32 = calculateEnergy(ops, skylake, BitWidth::bits_32);

        std::cout << std::left << std::setw(22) << name
                  << std::right << std::fixed << std::setprecision(0)
                  << std::setw(10) << e16
                  << std::setw(10) << e32
                  << std::setw(10) << e64
                  << std::setprecision(2) << std::setw(11)
                  << (skylake_e32 / e32) << "x\n";
    };

    printRow("Intel Skylake (14nm)", skylake);
    printRow("AMD Zen 3 (7nm)", zen3);
    printRow("AMD Zen 4 (5nm)", zen4);
    printRow("ARM Cortex-A76 (7nm)", arm_a76);
    printRow("Apple M1 (5nm)", m1);
    printRow("Apple M3 (3nm)", m3);

    std::cout << "\n(Values in picojoules. Higher 'vs Skylake' = more efficient)\n";

    // 32-bit FMA energy comparison
    std::cout << "\n32-bit FMA Energy Ranking (most to least efficient):\n";
    std::cout << std::string(50, '-') << "\n";

    struct ArchEnergy {
        const char* name;
        double energy;
    };

    std::vector<ArchEnergy> rankings = {
        {"Intel Skylake (14nm)", calculateEnergy(ops, skylake, BitWidth::bits_32)},
        {"AMD Zen 3 (7nm)", calculateEnergy(ops, zen3, BitWidth::bits_32)},
        {"AMD Zen 4 (5nm)", calculateEnergy(ops, zen4, BitWidth::bits_32)},
        {"ARM Cortex-A76 (7nm)", calculateEnergy(ops, arm_a76, BitWidth::bits_32)},
        {"Apple M1 (5nm)", calculateEnergy(ops, m1, BitWidth::bits_32)},
        {"Apple M3 (3nm)", calculateEnergy(ops, m3, BitWidth::bits_32)}
    };

    std::sort(rankings.begin(), rankings.end(),
              [](const ArchEnergy& a, const ArchEnergy& b) { return a.energy < b.energy; });

    for (size_t i = 0; i < rankings.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << std::left << std::setw(22) << rankings[i].name
                  << std::right << std::fixed << std::setprecision(0)
                  << std::setw(8) << rankings[i].energy << " pJ\n";
    }
}

void demonstrateScopedMeasurement() {
    std::cout << "\n========================================\n";
    std::cout << "Scoped Measurement with RAII\n";
    std::cout << "========================================\n\n";

    std::cout << "Using instrumented_scope for automatic reset/capture:\n\n";

    {
        instrumented_scope scope;  // Automatically resets counters

        // Do some computation
        instrumented<float> a = 1.5f;
        instrumented<float> b = 2.5f;
        instrumented<float> c = a + b;
        instrumented<float> d = a * b;
        instrumented<float> e = c / d;
        instrumented<float> f = sqrt(e);

        std::cout << "Computed: ((1.5 + 2.5) / (1.5 * 2.5))^0.5 = " << f << "\n\n";

        // Get stats within scope
        scope.report(std::cout);

        // Calculate energy
        auto stats = scope.stats<float>();
        const auto& model = getIntelSkylakeModel();
        double energy = calculateEnergy(stats, model, BitWidth::bits_32);
        std::cout << "\nTotal energy: " << std::fixed << std::setprecision(2)
                  << energy << " pJ\n";
    }
}

int main()
try {
    std::cout << "Universal Numbers Library: Instrumented Types with Energy Estimation\n";
    std::cout << "=====================================================================\n\n";

    demonstrateDotProduct();
    demonstrateMatvec();
    demonstrateNewtonSqrt();
    demonstratePolynomialEval();
    demonstrateEnergyComparison();
    demonstrateScopedMeasurement();

    std::cout << "\n\nKey Takeaways:\n";
    std::cout << "1. instrumented<T> transparently wraps any numeric type\n";
    std::cout << "2. All arithmetic operations are automatically counted\n";
    std::cout << "3. Energy estimation combines operation counts with cost models\n";
    std::cout << "4. Lower precision types show significant energy savings\n";
    std::cout << "5. Use instrumented_scope for RAII-style measurement\n";

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
