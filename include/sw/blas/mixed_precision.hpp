#pragma once
// mixed_precision.hpp: Mixed-precision BLAS operations for energy efficiency
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Mixed-precision BLAS provides energy-efficient implementations of common
// linear algebra operations by using different precisions for different
// stages of computation (input, compute, accumulation, output).
//
// This integrates with the energy cost modeling and Pareto analysis tools
// to provide data-driven precision selection.

#include <iostream>
#include <vector>
#include <cmath>
#include <type_traits>

// Number types
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>

// Energy estimation
#include <universal/utility/occurrence.hpp>
#include <universal/energy/energy.hpp>

namespace sw { namespace blas {

/// Mixed-precision configuration for BLAS operations
template<typename InputT, typename ComputeT, typename AccumT, typename OutputT = InputT>
struct MixedPrecisionConfig {
    using input_type = InputT;
    using compute_type = ComputeT;
    using accumulator_type = AccumT;
    using output_type = OutputT;

    static constexpr unsigned input_bits = sizeof(InputT) * 8;
    static constexpr unsigned compute_bits = sizeof(ComputeT) * 8;
    static constexpr unsigned accum_bits = sizeof(AccumT) * 8;
    static constexpr unsigned output_bits = sizeof(OutputT) * 8;

    static void describe(std::ostream& os) {
        os << "MixedPrecision<"
           << "input=" << input_bits << "b, "
           << "compute=" << compute_bits << "b, "
           << "accum=" << accum_bits << "b, "
           << "output=" << output_bits << "b>";
    }
};

/// Common mixed-precision configurations
using MP_FP32_Only = MixedPrecisionConfig<float, float, float, float>;
using MP_FP16_Accum32 = MixedPrecisionConfig<sw::universal::half, sw::universal::half, float, sw::universal::half>;
using MP_INT8_Accum32 = MixedPrecisionConfig<int8_t, int8_t, int32_t, int8_t>;
// Note: bfloat16 requires explicit include of universal/number/bfloat/bfloat.hpp
// using MP_BF16_Accum32 = MixedPrecisionConfig<sw::universal::bfloat_t, sw::universal::bfloat_t, float, sw::universal::bfloat_t>;
using MP_Posit16_Accum32 = MixedPrecisionConfig<sw::universal::posit<16,1>, sw::universal::posit<16,1>, sw::universal::posit<32,2>, sw::universal::posit<16,1>>;

/// Energy-tracking wrapper for mixed-precision operations
struct MixedPrecisionStats {
    uint64_t input_loads = 0;
    uint64_t compute_ops = 0;   // multiplications
    uint64_t accum_ops = 0;     // additions
    uint64_t output_stores = 0;
    double estimated_energy_pj = 0.0;

    void reset() {
        input_loads = 0;
        compute_ops = 0;
        accum_ops = 0;
        output_stores = 0;
        estimated_energy_pj = 0.0;
    }

    void report(std::ostream& os) const {
        os << "Mixed-Precision Statistics:\n";
        os << "  Input loads:    " << input_loads << "\n";
        os << "  Compute ops:    " << compute_ops << "\n";
        os << "  Accumulator ops: " << accum_ops << "\n";
        os << "  Output stores:  " << output_stores << "\n";
        os << "  Est. energy:    " << (estimated_energy_pj / 1e6) << " uJ\n";
    }
};

// Global stats for tracking
inline MixedPrecisionStats& getMixedPrecisionStats() {
    static MixedPrecisionStats stats;
    return stats;
}

/// Mixed-precision dot product
/// Uses InputT for storage, ComputeT for multiplication, AccumT for accumulation
template<typename MPC>
typename MPC::output_type
mp_dot(const std::vector<typename MPC::input_type>& x,
       const std::vector<typename MPC::input_type>& y,
       MixedPrecisionStats* stats = nullptr) {

    using InputT = typename MPC::input_type;
    using ComputeT = typename MPC::compute_type;
    using AccumT = typename MPC::accumulator_type;
    using OutputT = typename MPC::output_type;

    size_t n = std::min(x.size(), y.size());

    // Track operations
    if (stats) {
        stats->input_loads += 2 * n;
        stats->compute_ops += n;
        stats->accum_ops += n;
        stats->output_stores += 1;
    }

    // Accumulator at higher precision
    AccumT sum = AccumT(0);

    for (size_t i = 0; i < n; ++i) {
        // Load inputs
        ComputeT xi = static_cast<ComputeT>(x[i]);
        ComputeT yi = static_cast<ComputeT>(y[i]);

        // Multiply at compute precision
        ComputeT prod = xi * yi;

        // Accumulate at accumulator precision
        sum += static_cast<AccumT>(prod);
    }

    return static_cast<OutputT>(sum);
}

/// Mixed-precision GEMM: C = alpha * A * B + beta * C
/// A is m x k, B is k x n, C is m x n
template<typename MPC>
void mp_gemm(size_t m, size_t n, size_t k,
             typename MPC::compute_type alpha,
             const std::vector<typename MPC::input_type>& A,
             const std::vector<typename MPC::input_type>& B,
             typename MPC::compute_type beta,
             std::vector<typename MPC::output_type>& C,
             MixedPrecisionStats* stats = nullptr) {

    using InputT = typename MPC::input_type;
    using ComputeT = typename MPC::compute_type;
    using AccumT = typename MPC::accumulator_type;
    using OutputT = typename MPC::output_type;

    // Track operations
    if (stats) {
        stats->input_loads += m * k + k * n + m * n;
        stats->compute_ops += 2 * m * n * k;  // Each element requires k MACs
        stats->accum_ops += 2 * m * n * k;
        stats->output_stores += m * n;
    }

    // Ensure C has correct size
    if (C.size() != m * n) {
        C.resize(m * n);
    }

    // GEMM: C[i,j] = alpha * sum_p(A[i,p] * B[p,j]) + beta * C[i,j]
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            // Accumulate at high precision
            AccumT sum = AccumT(0);

            for (size_t p = 0; p < k; ++p) {
                ComputeT a_ip = static_cast<ComputeT>(A[i * k + p]);
                ComputeT b_pj = static_cast<ComputeT>(B[p * n + j]);
                sum += static_cast<AccumT>(a_ip * b_pj);
            }

            // Scale and add to output
            ComputeT result = alpha * static_cast<ComputeT>(sum) +
                              beta * static_cast<ComputeT>(C[i * n + j]);
            C[i * n + j] = static_cast<OutputT>(result);
        }
    }
}

/// Mixed-precision matrix-vector product: y = alpha * A * x + beta * y
template<typename MPC>
void mp_gemv(size_t m, size_t n,
             typename MPC::compute_type alpha,
             const std::vector<typename MPC::input_type>& A,
             const std::vector<typename MPC::input_type>& x,
             typename MPC::compute_type beta,
             std::vector<typename MPC::output_type>& y,
             MixedPrecisionStats* stats = nullptr) {

    using ComputeT = typename MPC::compute_type;
    using AccumT = typename MPC::accumulator_type;
    using OutputT = typename MPC::output_type;

    if (stats) {
        stats->input_loads += m * n + n + m;
        stats->compute_ops += 2 * m * n;
        stats->accum_ops += 2 * m * n;
        stats->output_stores += m;
    }

    if (y.size() != m) {
        y.resize(m);
    }

    for (size_t i = 0; i < m; ++i) {
        AccumT sum = AccumT(0);

        for (size_t j = 0; j < n; ++j) {
            ComputeT a_ij = static_cast<ComputeT>(A[i * n + j]);
            ComputeT x_j = static_cast<ComputeT>(x[j]);
            sum += static_cast<AccumT>(a_ij * x_j);
        }

        ComputeT result = alpha * static_cast<ComputeT>(sum) +
                          beta * static_cast<ComputeT>(y[i]);
        y[i] = static_cast<OutputT>(result);
    }
}

/// Mixed-precision AXPY: y = alpha * x + y
template<typename MPC>
void mp_axpy(size_t n,
             typename MPC::compute_type alpha,
             const std::vector<typename MPC::input_type>& x,
             std::vector<typename MPC::output_type>& y,
             MixedPrecisionStats* stats = nullptr) {

    using ComputeT = typename MPC::compute_type;
    using OutputT = typename MPC::output_type;

    if (stats) {
        stats->input_loads += 2 * n;
        stats->compute_ops += n;
        stats->accum_ops += n;
        stats->output_stores += n;
    }

    for (size_t i = 0; i < n; ++i) {
        ComputeT xi = static_cast<ComputeT>(x[i]);
        ComputeT yi = static_cast<ComputeT>(y[i]);
        y[i] = static_cast<OutputT>(alpha * xi + yi);
    }
}

/// Estimate energy for mixed-precision operation
template<typename MPC>
double estimateMixedPrecisionEnergy(const MixedPrecisionStats& stats,
                                     const sw::universal::energy::EnergyCostModel& model =
                                         sw::universal::energy::getDefaultModel()) {
    using namespace sw::universal::energy;

    // Map bit widths to BitWidth enum
    auto toBitWidth = [](unsigned bits) -> BitWidth {
        if (bits <= 8) return BitWidth::bits_8;
        if (bits <= 16) return BitWidth::bits_16;
        if (bits <= 32) return BitWidth::bits_32;
        return BitWidth::bits_64;
    };

    BitWidth input_bw = toBitWidth(MPC::input_bits);
    BitWidth compute_bw = toBitWidth(MPC::compute_bits);
    BitWidth accum_bw = toBitWidth(MPC::accum_bits);
    BitWidth output_bw = toBitWidth(MPC::output_bits);

    double energy = 0.0;

    // Input loads (from L1 cache assumption)
    energy += model.memoryTransferEnergy(MemoryLevel::L1_Cache,
                                          stats.input_loads * (MPC::input_bits / 8), false);

    // Compute operations (multiplications)
    energy += model.totalOperationEnergy(Operation::FloatMultiply, compute_bw, stats.compute_ops);

    // Accumulation operations (additions)
    energy += model.totalOperationEnergy(Operation::FloatAdd, accum_bw, stats.accum_ops);

    // Output stores
    energy += model.memoryTransferEnergy(MemoryLevel::L1_Cache,
                                          stats.output_stores * (MPC::output_bits / 8), true);

    return energy;
}

/// Compare energy between single-precision and mixed-precision
struct EnergyComparison {
    double single_precision_pj;
    double mixed_precision_pj;
    double energy_ratio;
    double savings_percent;

    void report(std::ostream& os) const {
        os << "Energy Comparison:\n";
        os << "  Single precision: " << std::fixed << std::setprecision(2)
           << (single_precision_pj / 1e6) << " uJ\n";
        os << "  Mixed precision:  " << (mixed_precision_pj / 1e6) << " uJ\n";
        os << "  Ratio (MP/SP):    " << std::setprecision(3) << energy_ratio << "x\n";
        os << "  Savings:          " << std::setprecision(1) << savings_percent << "%\n";
    }
};

template<typename MPC>
EnergyComparison compareMixedPrecisionEnergy(const MixedPrecisionStats& stats) {
    EnergyComparison cmp;

    // Single precision stats (same operations but all at 32-bit)
    MixedPrecisionStats sp_stats = stats;
    cmp.single_precision_pj = estimateMixedPrecisionEnergy<MP_FP32_Only>(sp_stats);
    cmp.mixed_precision_pj = estimateMixedPrecisionEnergy<MPC>(stats);

    cmp.energy_ratio = cmp.mixed_precision_pj / cmp.single_precision_pj;
    cmp.savings_percent = (1.0 - cmp.energy_ratio) * 100.0;

    return cmp;
}

/// Accuracy test for mixed-precision dot product
template<typename MPC>
struct AccuracyTest {
    double reference_value;       // Computed with double precision
    double mixed_value;           // Computed with mixed precision
    double single_value;          // Computed with single precision
    double mixed_relative_error;
    double single_relative_error;

    void report(std::ostream& os) const {
        os << "Accuracy Test:\n";
        os << "  Reference (FP64): " << std::scientific << std::setprecision(15)
           << reference_value << "\n";
        os << "  Single (FP32):    " << single_value << "\n";
        os << "  Mixed precision:  " << mixed_value << "\n";
        os << "  FP32 rel. error:  " << std::setprecision(2) << (single_relative_error * 100) << "%\n";
        os << "  Mixed rel. error: " << (mixed_relative_error * 100) << "%\n";
    }
};

/// Run accuracy test for dot product
template<typename MPC>
AccuracyTest<MPC> testDotProductAccuracy(const std::vector<double>& x_double,
                                          const std::vector<double>& y_double) {
    AccuracyTest<MPC> test;

    size_t n = std::min(x_double.size(), y_double.size());

    // Reference: double precision
    double ref_sum = 0.0;
    for (size_t i = 0; i < n; ++i) {
        ref_sum += x_double[i] * y_double[i];
    }
    test.reference_value = ref_sum;

    // Single precision
    float sp_sum = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        sp_sum += static_cast<float>(x_double[i]) * static_cast<float>(y_double[i]);
    }
    test.single_value = static_cast<double>(sp_sum);

    // Mixed precision
    using InputT = typename MPC::input_type;
    std::vector<InputT> x_mp(n), y_mp(n);
    for (size_t i = 0; i < n; ++i) {
        x_mp[i] = static_cast<InputT>(x_double[i]);
        y_mp[i] = static_cast<InputT>(y_double[i]);
    }
    auto mp_result = mp_dot<MPC>(x_mp, y_mp);
    test.mixed_value = static_cast<double>(mp_result);

    // Compute errors
    test.single_relative_error = std::abs((test.single_value - test.reference_value) / test.reference_value);
    test.mixed_relative_error = std::abs((test.mixed_value - test.reference_value) / test.reference_value);

    return test;
}

/// Benchmark and recommend best mixed-precision configuration
struct MixedPrecisionRecommendation {
    std::string config_name;
    double estimated_energy_ratio;
    double measured_accuracy;
    bool meets_accuracy_requirement;
};

inline std::vector<MixedPrecisionRecommendation>
benchmarkMixedPrecisionConfigs(const std::vector<double>& x,
                                const std::vector<double>& y,
                                double accuracy_requirement = 1e-4) {

    std::vector<MixedPrecisionRecommendation> results;

    // Test FP32 baseline
    {
        auto accuracy = testDotProductAccuracy<MP_FP32_Only>(x, y);
        MixedPrecisionStats stats;
        mp_dot<MP_FP32_Only>(
            std::vector<float>(x.begin(), x.end()),
            std::vector<float>(y.begin(), y.end()),
            &stats);
        auto energy = compareMixedPrecisionEnergy<MP_FP32_Only>(stats);

        results.push_back({
            "FP32-only",
            energy.energy_ratio,
            accuracy.single_relative_error,
            accuracy.single_relative_error <= accuracy_requirement
        });
    }

    // Test FP16 with FP32 accumulator
    {
        auto accuracy = testDotProductAccuracy<MP_FP16_Accum32>(x, y);
        MixedPrecisionStats stats;
        std::vector<sw::universal::half> x_h(x.begin(), x.end());
        std::vector<sw::universal::half> y_h(y.begin(), y.end());
        mp_dot<MP_FP16_Accum32>(x_h, y_h, &stats);
        auto energy = compareMixedPrecisionEnergy<MP_FP16_Accum32>(stats);

        results.push_back({
            "FP16+FP32acc",
            energy.energy_ratio,
            accuracy.mixed_relative_error,
            accuracy.mixed_relative_error <= accuracy_requirement
        });
    }

    // Test posit<16,1> with posit<32,2> accumulator
    {
        auto accuracy = testDotProductAccuracy<MP_Posit16_Accum32>(x, y);
        MixedPrecisionStats stats;
        std::vector<sw::universal::posit<16,1>> x_p(x.begin(), x.end());
        std::vector<sw::universal::posit<16,1>> y_p(y.begin(), y.end());
        mp_dot<MP_Posit16_Accum32>(x_p, y_p, &stats);
        auto energy = compareMixedPrecisionEnergy<MP_Posit16_Accum32>(stats);

        results.push_back({
            "posit16+32acc",
            energy.energy_ratio,
            accuracy.mixed_relative_error,
            accuracy.mixed_relative_error <= accuracy_requirement
        });
    }

    return results;
}

/// Print benchmark results
inline void reportMixedPrecisionBenchmark(std::ostream& os,
                                           const std::vector<MixedPrecisionRecommendation>& results,
                                           double accuracy_requirement) {
    os << "Mixed-Precision Configuration Benchmark\n";
    os << std::string(60, '=') << "\n";
    os << "Accuracy requirement: " << std::scientific << std::setprecision(1)
       << accuracy_requirement << "\n\n";

    os << std::left << std::setw(18) << "Configuration"
       << std::right << std::setw(12) << "Energy"
       << std::setw(15) << "Rel. Error"
       << std::setw(12) << "Meets Acc" << "\n";
    os << std::string(60, '-') << "\n";

    for (const auto& r : results) {
        os << std::left << std::setw(18) << r.config_name
           << std::right << std::fixed << std::setprecision(3)
           << std::setw(11) << r.estimated_energy_ratio << "x"
           << std::scientific << std::setprecision(2) << std::setw(15) << r.measured_accuracy
           << std::setw(12) << (r.meets_accuracy_requirement ? "YES" : "NO") << "\n";
    }

    // Find best configuration
    const MixedPrecisionRecommendation* best = nullptr;
    for (const auto& r : results) {
        if (r.meets_accuracy_requirement) {
            if (!best || r.estimated_energy_ratio < best->estimated_energy_ratio) {
                best = &r;
            }
        }
    }

    os << std::string(60, '-') << "\n";
    if (best) {
        os << "Recommended: " << best->config_name
           << " (energy=" << std::fixed << std::setprecision(3)
           << best->estimated_energy_ratio << "x)\n";
    } else {
        os << "Warning: No configuration meets accuracy requirement\n";
    }
}

}} // namespace sw::blas

