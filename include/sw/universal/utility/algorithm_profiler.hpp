#pragma once
// algorithm_profiler.hpp: unified profiler for mixed-precision algorithm analysis
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// The algorithm_profiler provides a unified view of algorithm characteristics
// by combining operation counting, energy estimation, range analysis, and
// memory profiling into a single analysis framework.
//
// Usage:
//   #include <universal/utility/algorithm_profiler.hpp>
//
//   using namespace sw::universal;
//
//   AlgorithmProfile profile = AlgorithmProfiler::profileGEMM<float>(M, N, K);
//   profile.report(std::cout);
//
//   // Compare precisions
//   auto comparison = AlgorithmProfiler::comparePrecisions<float, half>(profile_fp32, profile_fp16);

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <cmath>
#include <functional>

#include "occurrence.hpp"
#include "range_analyzer.hpp"
#include "memory_profiler.hpp"

// Forward declare energy types if available
namespace sw { namespace universal { namespace energy {
    struct EnergyCostModel;
    enum class BitWidth;
    enum class Operation;
}}}

namespace sw { namespace universal {

/// Complete algorithm profile combining all analysis dimensions
struct AlgorithmProfile {
    std::string name;                   // Algorithm name
    std::string precision;              // Precision used (e.g., "float", "posit<32,2>")
    int bit_width;                      // Bit width of the type

    // Problem size
    uint64_t problem_size;              // Primary size parameter (N for vectors, N^2 for matrices)
    std::string size_description;       // Human-readable size (e.g., "1024x1024")

    // Operation counts
    uint64_t additions;
    uint64_t subtractions;
    uint64_t multiplications;
    uint64_t divisions;
    uint64_t fmas;                      // Fused multiply-adds
    uint64_t sqrts;
    uint64_t comparisons;
    uint64_t total_ops;

    // Memory statistics
    uint64_t bytes_read;
    uint64_t bytes_written;
    uint64_t working_set_bytes;
    std::string primary_cache_tier;     // Where most data lives

    // Range statistics
    double min_value;
    double max_value;
    int min_scale;
    int max_scale;
    int scale_span;

    // Energy estimates (picojoules)
    double compute_energy_pj;
    double memory_energy_pj;
    double total_energy_pj;

    // Derived metrics
    double ops_per_byte;                // Arithmetic intensity
    double energy_per_op_pj;            // Average energy per operation

    AlgorithmProfile()
        : name("unknown"), precision("unknown"), bit_width(32)
        , problem_size(0), size_description("")
        , additions(0), subtractions(0), multiplications(0)
        , divisions(0), fmas(0), sqrts(0), comparisons(0), total_ops(0)
        , bytes_read(0), bytes_written(0), working_set_bytes(0)
        , primary_cache_tier("unknown")
        , min_value(0), max_value(0), min_scale(0), max_scale(0), scale_span(0)
        , compute_energy_pj(0), memory_energy_pj(0), total_energy_pj(0)
        , ops_per_byte(0), energy_per_op_pj(0)
    {}

    /// Calculate derived metrics
    void calculateDerivedMetrics() {
        total_ops = additions + subtractions + multiplications + divisions + fmas + sqrts;
        uint64_t total_bytes = bytes_read + bytes_written;
        ops_per_byte = (total_bytes > 0) ? static_cast<double>(total_ops) / total_bytes : 0;
        total_energy_pj = compute_energy_pj + memory_energy_pj;
        energy_per_op_pj = (total_ops > 0) ? total_energy_pj / total_ops : 0;
    }

    /// Report profile to stream
    void report(std::ostream& ostr) const {
        ostr << "Algorithm Profile: " << name << "\n";
        ostr << std::string(60, '=') << "\n\n";

        ostr << "Configuration:\n";
        ostr << "  Precision:     " << precision << " (" << bit_width << "-bit)\n";
        ostr << "  Problem size:  " << size_description << "\n\n";

        ostr << "Operation Counts:\n";
        ostr << "  Additions:       " << std::setw(15) << additions << "\n";
        ostr << "  Subtractions:    " << std::setw(15) << subtractions << "\n";
        ostr << "  Multiplications: " << std::setw(15) << multiplications << "\n";
        ostr << "  Divisions:       " << std::setw(15) << divisions << "\n";
        ostr << "  FMAs:            " << std::setw(15) << fmas << "\n";
        ostr << "  Sqrt:            " << std::setw(15) << sqrts << "\n";
        ostr << "  Total:           " << std::setw(15) << total_ops << "\n\n";

        ostr << "Memory Access:\n";
        ostr << "  Bytes read:      " << formatBytes(bytes_read) << "\n";
        ostr << "  Bytes written:   " << formatBytes(bytes_written) << "\n";
        ostr << "  Working set:     " << formatBytes(working_set_bytes) << "\n";
        ostr << "  Primary tier:    " << primary_cache_tier << "\n";
        ostr << std::fixed << std::setprecision(2);
        ostr << "  Arithmetic intensity: " << ops_per_byte << " ops/byte\n\n";

        if (scale_span > 0) {
            ostr << "Value Range:\n";
            ostr << std::scientific << std::setprecision(2);
            ostr << "  Min value:   " << min_value << "\n";
            ostr << "  Max value:   " << max_value << "\n";
            ostr << std::fixed;
            ostr << "  Scale span:  " << scale_span << " decades\n\n";
        }

        ostr << "Energy Estimate:\n";
        ostr << std::fixed << std::setprecision(2);
        ostr << "  Compute:     " << (compute_energy_pj / 1e6) << " uJ\n";
        ostr << "  Memory:      " << (memory_energy_pj / 1e6) << " uJ\n";
        ostr << "  Total:       " << (total_energy_pj / 1e6) << " uJ\n";
        ostr << "  Per-op avg:  " << energy_per_op_pj << " pJ/op\n";
    }

    /// Get one-line summary
    std::string summary() const {
        std::stringstream ss;
        ss << precision << ": ";
        ss << total_ops << " ops, ";
        ss << formatBytes(working_set_bytes) << " WS, ";
        ss << std::fixed << std::setprecision(2);
        ss << (total_energy_pj / 1e6) << " uJ";
        return ss.str();
    }

private:
    static std::string formatBytes(uint64_t bytes) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1);
        if (bytes >= 1024ULL * 1024 * 1024) {
            ss << (bytes / (1024.0 * 1024 * 1024)) << " GB";
        } else if (bytes >= 1024 * 1024) {
            ss << (bytes / (1024.0 * 1024)) << " MB";
        } else if (bytes >= 1024) {
            ss << (bytes / 1024.0) << " KB";
        } else {
            ss << bytes << " B";
        }
        return ss.str();
    }
};

/// Comparison between two precision configurations
struct PrecisionComparison {
    AlgorithmProfile baseline;
    AlgorithmProfile alternative;

    double ops_ratio;           // alternative / baseline
    double memory_ratio;        // alternative / baseline
    double compute_energy_ratio;
    double memory_energy_ratio;
    double total_energy_ratio;
    double energy_savings_pct;  // Percentage saved by alternative

    void calculate() {
        ops_ratio = (baseline.total_ops > 0) ?
            static_cast<double>(alternative.total_ops) / baseline.total_ops : 1.0;
        memory_ratio = (baseline.bytes_read + baseline.bytes_written > 0) ?
            static_cast<double>(alternative.bytes_read + alternative.bytes_written) /
            (baseline.bytes_read + baseline.bytes_written) : 1.0;
        compute_energy_ratio = (baseline.compute_energy_pj > 0) ?
            alternative.compute_energy_pj / baseline.compute_energy_pj : 1.0;
        memory_energy_ratio = (baseline.memory_energy_pj > 0) ?
            alternative.memory_energy_pj / baseline.memory_energy_pj : 1.0;
        total_energy_ratio = (baseline.total_energy_pj > 0) ?
            alternative.total_energy_pj / baseline.total_energy_pj : 1.0;
        energy_savings_pct = (1.0 - total_energy_ratio) * 100.0;
    }

    void report(std::ostream& ostr) const {
        ostr << "Precision Comparison\n";
        ostr << std::string(50, '=') << "\n\n";

        ostr << "Baseline:    " << baseline.precision << "\n";
        ostr << "Alternative: " << alternative.precision << "\n\n";

        ostr << std::fixed << std::setprecision(2);
        ostr << std::left << std::setw(25) << "Metric"
             << std::right << std::setw(12) << "Baseline"
             << std::setw(12) << "Alternative"
             << std::setw(10) << "Ratio" << "\n";
        ostr << std::string(60, '-') << "\n";

        ostr << std::left << std::setw(25) << "Operations"
             << std::right << std::setw(12) << baseline.total_ops
             << std::setw(12) << alternative.total_ops
             << std::setw(9) << ops_ratio << "x\n";

        ostr << std::left << std::setw(25) << "Memory (bytes)"
             << std::right << std::setw(12) << (baseline.bytes_read + baseline.bytes_written)
             << std::setw(12) << (alternative.bytes_read + alternative.bytes_written)
             << std::setw(9) << memory_ratio << "x\n";

        ostr << std::left << std::setw(25) << "Compute Energy (uJ)"
             << std::right << std::setw(12) << (baseline.compute_energy_pj / 1e6)
             << std::setw(12) << (alternative.compute_energy_pj / 1e6)
             << std::setw(9) << compute_energy_ratio << "x\n";

        ostr << std::left << std::setw(25) << "Memory Energy (uJ)"
             << std::right << std::setw(12) << (baseline.memory_energy_pj / 1e6)
             << std::setw(12) << (alternative.memory_energy_pj / 1e6)
             << std::setw(9) << memory_energy_ratio << "x\n";

        ostr << std::left << std::setw(25) << "Total Energy (uJ)"
             << std::right << std::setw(12) << (baseline.total_energy_pj / 1e6)
             << std::setw(12) << (alternative.total_energy_pj / 1e6)
             << std::setw(9) << total_energy_ratio << "x\n";

        ostr << "\nEnergy savings: " << energy_savings_pct << "%\n";
    }
};

/// Algorithm profiler for mixed-precision analysis
class AlgorithmProfiler {
public:
    /// Profile a GEMM operation (C = A * B)
    static AlgorithmProfile profileGEMM(
            uint64_t M, uint64_t N, uint64_t K,
            const std::string& precision, int bit_width,
            const CacheConfig& cache = CacheConfig()) {

        AlgorithmProfile profile;
        profile.name = "GEMM";
        profile.precision = precision;
        profile.bit_width = bit_width;
        profile.problem_size = M * N * K;

        std::stringstream ss;
        ss << "C[" << M << "x" << N << "] = A[" << M << "x" << K << "] * B[" << K << "x" << N << "]";
        profile.size_description = ss.str();

        // Operation counts for naive GEMM
        // Each C[i,j] = sum(A[i,k] * B[k,j]) for k=0..K-1
        // = K multiplications + (K-1) additions per element
        // Using FMA: K FMAs per element
        profile.fmas = M * N * K;
        profile.multiplications = 0;  // Using FMAs
        profile.additions = 0;        // Using FMAs

        // Memory access
        size_t elem_size = bit_width / 8;
        profile.bytes_read = (M * K + K * N) * elem_size;   // Read A and B
        profile.bytes_written = M * N * elem_size;          // Write C
        profile.working_set_bytes = (M * K + K * N + M * N) * elem_size;

        // Memory profiling
        auto mem_profile = sw::universal::profileGEMM(M, N, K, elem_size, cache);
        profile.primary_cache_tier = memoryTierName(mem_profile.estimatePrimaryTier());
        profile.memory_energy_pj = mem_profile.estimateEnergyPJ();

        // Compute energy (simplified model)
        // FMA energy depends on bit width
        double fma_energy_pj = estimateFMAEnergy(bit_width);
        profile.compute_energy_pj = profile.fmas * fma_energy_pj;

        profile.calculateDerivedMetrics();
        return profile;
    }

    /// Profile a dot product operation
    static AlgorithmProfile profileDotProduct(
            uint64_t N,
            const std::string& precision, int bit_width,
            const CacheConfig& cache = CacheConfig()) {

        AlgorithmProfile profile;
        profile.name = "Dot Product";
        profile.precision = precision;
        profile.bit_width = bit_width;
        profile.problem_size = N;

        std::stringstream ss;
        ss << "dot(x[" << N << "], y[" << N << "])";
        profile.size_description = ss.str();

        // Operation counts: N multiplications, N-1 additions
        // Using FMA: N FMAs
        profile.fmas = N;

        // Memory access
        size_t elem_size = bit_width / 8;
        profile.bytes_read = 2 * N * elem_size;     // Read x and y
        profile.bytes_written = elem_size;           // Write result
        profile.working_set_bytes = 2 * N * elem_size;

        // Memory profiling
        auto mem_profile = sw::universal::profileDotProduct(N, elem_size, cache);
        profile.primary_cache_tier = memoryTierName(mem_profile.estimatePrimaryTier());
        profile.memory_energy_pj = mem_profile.estimateEnergyPJ();

        // Compute energy
        double fma_energy_pj = estimateFMAEnergy(bit_width);
        profile.compute_energy_pj = profile.fmas * fma_energy_pj;

        profile.calculateDerivedMetrics();
        return profile;
    }

    /// Profile a matrix-vector multiply (y = A * x)
    static AlgorithmProfile profileGEMV(
            uint64_t M, uint64_t N,
            const std::string& precision, int bit_width,
            const CacheConfig& cache = CacheConfig()) {

        AlgorithmProfile profile;
        profile.name = "GEMV";
        profile.precision = precision;
        profile.bit_width = bit_width;
        profile.problem_size = M * N;

        std::stringstream ss;
        ss << "y[" << M << "] = A[" << M << "x" << N << "] * x[" << N << "]";
        profile.size_description = ss.str();

        // Operation counts: M*N FMAs
        profile.fmas = M * N;

        // Memory access
        size_t elem_size = bit_width / 8;
        profile.bytes_read = (M * N + N) * elem_size;  // Read A and x
        profile.bytes_written = M * elem_size;          // Write y
        profile.working_set_bytes = (M * N + N + M) * elem_size;

        // Memory profiling
        auto mem_profile = sw::universal::profileGEMV(M, N, elem_size, cache);
        profile.primary_cache_tier = memoryTierName(mem_profile.estimatePrimaryTier());
        profile.memory_energy_pj = mem_profile.estimateEnergyPJ();

        // Compute energy
        double fma_energy_pj = estimateFMAEnergy(bit_width);
        profile.compute_energy_pj = profile.fmas * fma_energy_pj;

        profile.calculateDerivedMetrics();
        return profile;
    }

    /// Profile convolution (simplified 2D)
    static AlgorithmProfile profileConv2D(
            uint64_t H, uint64_t W,      // Input height, width
            uint64_t C_in, uint64_t C_out, // Input/output channels
            uint64_t K,                   // Kernel size (KxK)
            const std::string& precision, int bit_width,
            const CacheConfig& cache = CacheConfig()) {

        AlgorithmProfile profile;
        profile.name = "Conv2D";
        profile.precision = precision;
        profile.bit_width = bit_width;

        // Output size (same padding assumed)
        uint64_t H_out = H;
        uint64_t W_out = W;
        profile.problem_size = H_out * W_out * C_out;

        std::stringstream ss;
        ss << "Conv2D(" << H << "x" << W << "x" << C_in << ", "
           << K << "x" << K << "x" << C_out << ")";
        profile.size_description = ss.str();

        // Operation counts: For each output pixel, K*K*C_in FMAs
        profile.fmas = H_out * W_out * C_out * K * K * C_in;

        // Memory access
        size_t elem_size = bit_width / 8;
        uint64_t input_bytes = H * W * C_in * elem_size;
        uint64_t kernel_bytes = K * K * C_in * C_out * elem_size;
        uint64_t output_bytes = H_out * W_out * C_out * elem_size;

        profile.bytes_read = input_bytes + kernel_bytes;
        profile.bytes_written = output_bytes;
        profile.working_set_bytes = input_bytes + kernel_bytes + output_bytes;

        // Memory profiling (simplified)
        MemoryProfiler mem_profile(cache);
        mem_profile.recordRead(input_bytes, AccessPattern::Strided);
        mem_profile.recordRead(kernel_bytes, AccessPattern::Reuse);
        mem_profile.recordWrite(output_bytes, AccessPattern::Sequential);
        mem_profile.setWorkingSetSize(profile.working_set_bytes);

        profile.primary_cache_tier = memoryTierName(mem_profile.estimatePrimaryTier());
        profile.memory_energy_pj = mem_profile.estimateEnergyPJ();

        // Compute energy
        double fma_energy_pj = estimateFMAEnergy(bit_width);
        profile.compute_energy_pj = profile.fmas * fma_energy_pj;

        profile.calculateDerivedMetrics();
        return profile;
    }

    /// Compare two profiles
    static PrecisionComparison compare(
            const AlgorithmProfile& baseline,
            const AlgorithmProfile& alternative) {
        PrecisionComparison cmp;
        cmp.baseline = baseline;
        cmp.alternative = alternative;
        cmp.calculate();
        return cmp;
    }

    /// Generate comparison table for multiple precisions
    static void compareMultiple(
            std::ostream& ostr,
            const std::vector<AlgorithmProfile>& profiles) {

        if (profiles.empty()) return;

        ostr << "Multi-Precision Comparison: " << profiles[0].name << "\n";
        ostr << std::string(80, '=') << "\n\n";

        ostr << std::left << std::setw(15) << "Precision"
             << std::right << std::setw(12) << "Bit Width"
             << std::setw(15) << "Operations"
             << std::setw(12) << "Memory"
             << std::setw(15) << "Energy (uJ)"
             << std::setw(12) << "vs FP32" << "\n";
        ostr << std::string(80, '-') << "\n";

        // Find FP32 baseline for comparison
        double fp32_energy = 0;
        for (const auto& p : profiles) {
            if (p.bit_width == 32) {
                fp32_energy = p.total_energy_pj;
                break;
            }
        }

        for (const auto& p : profiles) {
            double ratio = (fp32_energy > 0) ? p.total_energy_pj / fp32_energy : 1.0;
            std::stringstream mem_ss;
            mem_ss << std::fixed << std::setprecision(1);
            if (p.working_set_bytes >= 1024*1024) {
                mem_ss << (p.working_set_bytes / (1024.0*1024)) << " MB";
            } else {
                mem_ss << (p.working_set_bytes / 1024.0) << " KB";
            }

            ostr << std::left << std::setw(15) << p.precision
                 << std::right << std::setw(12) << p.bit_width
                 << std::setw(15) << p.total_ops
                 << std::setw(12) << mem_ss.str()
                 << std::fixed << std::setprecision(2)
                 << std::setw(15) << (p.total_energy_pj / 1e6)
                 << std::setw(11) << ratio << "x\n";
        }
    }

private:
    /// Estimate FMA energy based on bit width (Skylake-class)
    static double estimateFMAEnergy(int bit_width) {
        switch (bit_width) {
            case 8:  return 0.2;   // INT8/FP8
            case 16: return 0.47;  // FP16/BF16
            case 32: return 1.5;   // FP32
            case 64: return 5.3;   // FP64
            default: return 1.5;   // Default to FP32
        }
    }
};

}} // namespace sw::universal
