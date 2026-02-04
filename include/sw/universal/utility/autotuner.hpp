#pragma once
// autotuner.hpp: automatic precision selection through runtime profiling
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Autotuning enables automatic precision selection by running a kernel
// at multiple precisions and comparing accuracy, energy, and performance.

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <type_traits>
#include <limits>

// Universal number types
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>

// Energy estimation
#include <universal/utility/occurrence.hpp>
#include <universal/energy/energy.hpp>

namespace sw { namespace universal {

/// Result of tuning a single precision configuration
struct TuningPoint {
    std::string precision_name;
    unsigned bit_width;
    double relative_error;     // Max relative error vs reference
    double mean_ulp_error;     // Mean ULP error
    double estimated_energy_factor;  // Energy relative to FP32
    double estimated_bandwidth_factor;  // Bandwidth relative to FP32
    double execution_time_ns;  // Actual execution time
    uint64_t operations;       // Total operations performed
    bool meets_accuracy;       // Does it meet accuracy requirement?
    bool meets_energy;         // Does it meet energy budget?
};

/// Complete result of autotuning a kernel
struct AutotuneResult {
    std::string kernel_name;
    std::vector<TuningPoint> all_points;
    TuningPoint best_accuracy;      // Best accuracy regardless of energy
    TuningPoint best_energy;        // Best energy that meets accuracy
    TuningPoint recommended;        // Overall recommendation
    double accuracy_requirement;
    double energy_budget;           // Max energy factor vs FP32

    void report(std::ostream& ostr) const {
        ostr << "Autotuning Results: " << kernel_name << "\n";
        ostr << std::string(70, '=') << "\n\n";

        ostr << "Accuracy requirement: " << std::scientific << std::setprecision(1)
             << accuracy_requirement << "\n";
        ostr << "Energy budget: " << std::fixed << std::setprecision(2)
             << (energy_budget * 100) << "% of FP32\n\n";

        ostr << std::left << std::setw(18) << "Precision"
             << std::right << std::setw(8) << "Bits"
             << std::setw(12) << "RelError"
             << std::setw(12) << "ULP"
             << std::setw(10) << "Energy"
             << std::setw(10) << "BW"
             << std::setw(8) << "Acc"
             << std::setw(8) << "Eng" << "\n";
        ostr << std::string(70, '-') << "\n";

        for (const auto& pt : all_points) {
            ostr << std::left << std::setw(18) << pt.precision_name
                 << std::right << std::setw(8) << pt.bit_width
                 << std::scientific << std::setprecision(1) << std::setw(12) << pt.relative_error
                 << std::fixed << std::setprecision(2) << std::setw(12) << pt.mean_ulp_error
                 << std::setw(9) << pt.estimated_energy_factor << "x"
                 << std::setw(9) << pt.estimated_bandwidth_factor << "x"
                 << std::setw(8) << (pt.meets_accuracy ? "YES" : "-")
                 << std::setw(8) << (pt.meets_energy ? "YES" : "-") << "\n";
        }

        ostr << std::string(70, '-') << "\n\n";
        ostr << "Recommendations:\n";
        ostr << "  Best accuracy:  " << best_accuracy.precision_name
             << " (error=" << std::scientific << best_accuracy.relative_error << ")\n";
        ostr << "  Best energy:    " << best_energy.precision_name
             << " (energy=" << std::fixed << std::setprecision(2)
             << best_energy.estimated_energy_factor << "x)\n";
        ostr << "  * Recommended:  " << recommended.precision_name
             << " (error=" << std::scientific << recommended.relative_error
             << ", energy=" << std::fixed << recommended.estimated_energy_factor << "x)\n";
    }
};

/// Autotuner for precision selection
class Autotuner {
public:
    /// Default constructor with reasonable defaults
    Autotuner()
        : accuracy_req_(1e-4)
        , energy_budget_(0.5)
        , enable_timing_(true)
        , iterations_(100) {}

    /// Set accuracy requirement (max relative error)
    Autotuner& setAccuracyRequirement(double acc) { accuracy_req_ = acc; return *this; }

    /// Set energy budget (factor vs FP32, e.g., 0.5 = 50% of FP32 energy)
    Autotuner& setEnergyBudget(double budget) { energy_budget_ = budget; return *this; }

    /// Enable/disable timing measurements
    Autotuner& enableTiming(bool enable) { enable_timing_ = enable; return *this; }

    /// Set number of iterations for timing
    Autotuner& setIterations(size_t iters) { iterations_ = iters; return *this; }

    /// Tune a unary function (e.g., sqrt, sin, exp)
    /// Reference is computed using double precision
    template<typename Func>
    AutotuneResult tuneUnaryFunction(const std::string& name, Func&& func,
                                      const std::vector<double>& test_inputs) {
        AutotuneResult result;
        result.kernel_name = name;
        result.accuracy_requirement = accuracy_req_;
        result.energy_budget = energy_budget_;

        // Compute reference results with double precision
        std::vector<double> reference(test_inputs.size());
        for (size_t i = 0; i < test_inputs.size(); ++i) {
            reference[i] = func(test_inputs[i]);
        }

        // Test each precision
        testPrecision<double>(result, "FP64", 64, func, test_inputs, reference);
        testPrecision<float>(result, "FP32", 32, func, test_inputs, reference);
        testPrecision<half>(result, "FP16", 16, func, test_inputs, reference);
        testPrecision<posit<32,2>>(result, "posit<32,2>", 32, func, test_inputs, reference);
        testPrecision<posit<16,1>>(result, "posit<16,1>", 16, func, test_inputs, reference);
        testPrecision<posit<8,0>>(result, "posit<8,0>", 8, func, test_inputs, reference);

        // Find best configurations
        selectBest(result);
        return result;
    }

    /// Tune a binary function (e.g., add, multiply, pow)
    template<typename Func>
    AutotuneResult tuneBinaryFunction(const std::string& name, Func&& func,
                                       const std::vector<std::pair<double, double>>& test_inputs) {
        AutotuneResult result;
        result.kernel_name = name;
        result.accuracy_requirement = accuracy_req_;
        result.energy_budget = energy_budget_;

        // Compute reference results with double precision
        std::vector<double> reference(test_inputs.size());
        for (size_t i = 0; i < test_inputs.size(); ++i) {
            reference[i] = func(test_inputs[i].first, test_inputs[i].second);
        }

        // Test each precision
        testBinaryPrecision<double>(result, "FP64", 64, func, test_inputs, reference);
        testBinaryPrecision<float>(result, "FP32", 32, func, test_inputs, reference);
        testBinaryPrecision<half>(result, "FP16", 16, func, test_inputs, reference);
        testBinaryPrecision<posit<32,2>>(result, "posit<32,2>", 32, func, test_inputs, reference);
        testBinaryPrecision<posit<16,1>>(result, "posit<16,1>", 16, func, test_inputs, reference);
        testBinaryPrecision<posit<8,0>>(result, "posit<8,0>", 8, func, test_inputs, reference);

        // Find best configurations
        selectBest(result);
        return result;
    }

    /// Tune a reduction function (e.g., dot product, sum)
    template<typename Func>
    AutotuneResult tuneReduction(const std::string& name, Func&& func,
                                  const std::vector<std::vector<double>>& test_vectors) {
        AutotuneResult result;
        result.kernel_name = name;
        result.accuracy_requirement = accuracy_req_;
        result.energy_budget = energy_budget_;

        // Compute reference results with double precision
        std::vector<double> reference(test_vectors.size());
        for (size_t i = 0; i < test_vectors.size(); ++i) {
            reference[i] = func(test_vectors[i]);
        }

        // Test each precision
        testReductionPrecision<double>(result, "FP64", 64, func, test_vectors, reference);
        testReductionPrecision<float>(result, "FP32", 32, func, test_vectors, reference);
        testReductionPrecision<half>(result, "FP16", 16, func, test_vectors, reference);
        testReductionPrecision<posit<32,2>>(result, "posit<32,2>", 32, func, test_vectors, reference);
        testReductionPrecision<posit<16,1>>(result, "posit<16,1>", 16, func, test_vectors, reference);
        testReductionPrecision<posit<8,0>>(result, "posit<8,0>", 8, func, test_vectors, reference);

        // Find best configurations
        selectBest(result);
        return result;
    }

    /// Generate test inputs for common ranges
    static std::vector<double> generateTestInputs(double min_val, double max_val, size_t count) {
        std::vector<double> inputs(count);
        double step = (max_val - min_val) / static_cast<double>(count - 1);
        for (size_t i = 0; i < count; ++i) {
            inputs[i] = min_val + static_cast<double>(i) * step;
        }
        return inputs;
    }

    /// Generate logarithmically spaced test inputs
    static std::vector<double> generateLogTestInputs(double min_val, double max_val, size_t count) {
        std::vector<double> inputs(count);
        double log_min = std::log10(min_val);
        double log_max = std::log10(max_val);
        double step = (log_max - log_min) / static_cast<double>(count - 1);
        for (size_t i = 0; i < count; ++i) {
            inputs[i] = std::pow(10.0, log_min + static_cast<double>(i) * step);
        }
        return inputs;
    }

    /// Generate random test vectors for reduction
    static std::vector<std::vector<double>> generateTestVectors(size_t num_vectors,
                                                                  size_t vector_size,
                                                                  double min_val = -1.0,
                                                                  double max_val = 1.0) {
        std::vector<std::vector<double>> vectors(num_vectors);
        for (size_t i = 0; i < num_vectors; ++i) {
            vectors[i].resize(vector_size);
            for (size_t j = 0; j < vector_size; ++j) {
                // Simple pseudo-random using linear congruential generator
                static uint64_t seed = 12345;
                seed = seed * 1103515245 + 12345;
                double t = static_cast<double>((seed >> 16) & 0x7FFF) / 32767.0;
                vectors[i][j] = min_val + t * (max_val - min_val);
            }
        }
        return vectors;
    }

private:
    double accuracy_req_;
    double energy_budget_;
    bool enable_timing_;
    size_t iterations_;

    /// Test a unary function at a specific precision
    template<typename Real, typename Func>
    void testPrecision(AutotuneResult& result, const std::string& name, unsigned bits,
                       Func&& func, const std::vector<double>& inputs,
                       const std::vector<double>& reference) {
        TuningPoint pt;
        pt.precision_name = name;
        pt.bit_width = bits;

        double max_rel_error = 0.0;
        double sum_ulp_error = 0.0;

        for (size_t i = 0; i < inputs.size(); ++i) {
            Real x = static_cast<Real>(inputs[i]);
            Real y = func(x);
            double computed = static_cast<double>(y);
            double ref = reference[i];

            // Relative error
            if (std::abs(ref) > std::numeric_limits<double>::min()) {
                double rel_err = std::abs((computed - ref) / ref);
                max_rel_error = std::max(max_rel_error, rel_err);
            }

            // ULP error estimation (simplified)
            double eps = static_cast<double>(std::numeric_limits<Real>::epsilon());
            double ulp_err = (eps > 0) ? (std::abs(computed - ref) / eps) : 0.0;
            sum_ulp_error += ulp_err;
        }

        pt.relative_error = max_rel_error;
        pt.mean_ulp_error = sum_ulp_error / static_cast<double>(inputs.size());
        pt.operations = inputs.size();

        // Energy and bandwidth estimation
        estimateResources(pt, inputs.size());

        // Check constraints
        pt.meets_accuracy = (pt.relative_error <= accuracy_req_);
        pt.meets_energy = (pt.estimated_energy_factor <= energy_budget_);

        // Optional timing
        if (enable_timing_) {
            auto start = std::chrono::high_resolution_clock::now();
            for (size_t iter = 0; iter < iterations_; ++iter) {
                for (size_t i = 0; i < inputs.size(); ++i) {
                    Real x = static_cast<Real>(inputs[i]);
                    volatile Real y = func(x);
                    (void)y;
                }
            }
            auto end = std::chrono::high_resolution_clock::now();
            pt.execution_time_ns = std::chrono::duration<double, std::nano>(end - start).count() /
                                   static_cast<double>(iterations_ * inputs.size());
        }

        result.all_points.push_back(pt);
    }

    /// Test a binary function at a specific precision
    template<typename Real, typename Func>
    void testBinaryPrecision(AutotuneResult& result, const std::string& name, unsigned bits,
                              Func&& func, const std::vector<std::pair<double, double>>& inputs,
                              const std::vector<double>& reference) {
        TuningPoint pt;
        pt.precision_name = name;
        pt.bit_width = bits;

        double max_rel_error = 0.0;
        double sum_ulp_error = 0.0;

        for (size_t i = 0; i < inputs.size(); ++i) {
            Real a = static_cast<Real>(inputs[i].first);
            Real b = static_cast<Real>(inputs[i].second);
            Real y = func(a, b);
            double computed = static_cast<double>(y);
            double ref = reference[i];

            if (std::abs(ref) > std::numeric_limits<double>::min()) {
                double rel_err = std::abs((computed - ref) / ref);
                max_rel_error = std::max(max_rel_error, rel_err);
            }

            double eps = static_cast<double>(std::numeric_limits<Real>::epsilon());
            double ulp_err = (eps > 0) ? (std::abs(computed - ref) / eps) : 0.0;
            sum_ulp_error += ulp_err;
        }

        pt.relative_error = max_rel_error;
        pt.mean_ulp_error = sum_ulp_error / static_cast<double>(inputs.size());
        pt.operations = inputs.size();

        estimateResources(pt, inputs.size());

        pt.meets_accuracy = (pt.relative_error <= accuracy_req_);
        pt.meets_energy = (pt.estimated_energy_factor <= energy_budget_);

        result.all_points.push_back(pt);
    }

    /// Test a reduction function at a specific precision
    template<typename Real, typename Func>
    void testReductionPrecision(AutotuneResult& result, const std::string& name, unsigned bits,
                                 Func&& func, const std::vector<std::vector<double>>& inputs,
                                 const std::vector<double>& reference) {
        TuningPoint pt;
        pt.precision_name = name;
        pt.bit_width = bits;

        double max_rel_error = 0.0;
        double sum_ulp_error = 0.0;
        uint64_t total_ops = 0;

        for (size_t i = 0; i < inputs.size(); ++i) {
            // Convert input vector to Real type
            std::vector<Real> typed_input(inputs[i].size());
            for (size_t j = 0; j < inputs[i].size(); ++j) {
                typed_input[j] = static_cast<Real>(inputs[i][j]);
            }

            // Compute using lambda that works with vector<Real>
            Real y{0};
            for (const auto& v : typed_input) {
                y += v;
            }
            double computed = static_cast<double>(y);
            double ref = reference[i];

            if (std::abs(ref) > std::numeric_limits<double>::min()) {
                double rel_err = std::abs((computed - ref) / ref);
                max_rel_error = std::max(max_rel_error, rel_err);
            }

            double eps = static_cast<double>(std::numeric_limits<Real>::epsilon());
            double ulp_err = (eps > 0) ? (std::abs(computed - ref) / eps) : 0.0;
            sum_ulp_error += ulp_err;
            total_ops += inputs[i].size();
        }

        pt.relative_error = max_rel_error;
        pt.mean_ulp_error = sum_ulp_error / static_cast<double>(inputs.size());
        pt.operations = total_ops;

        estimateResources(pt, total_ops);

        pt.meets_accuracy = (pt.relative_error <= accuracy_req_);
        pt.meets_energy = (pt.estimated_energy_factor <= energy_budget_);

        result.all_points.push_back(pt);
    }

    /// Estimate energy and bandwidth based on bit width
    void estimateResources(TuningPoint& pt, uint64_t ops) {
        // Energy factor relative to FP32 (approximate based on bit width)
        switch (pt.bit_width) {
            case 64: pt.estimated_energy_factor = 3.5; break;
            case 32: pt.estimated_energy_factor = 1.0; break;
            case 16: pt.estimated_energy_factor = 0.3; break;
            case 8:  pt.estimated_energy_factor = 0.13; break;
            default: pt.estimated_energy_factor = static_cast<double>(pt.bit_width) / 32.0;
        }

        // Bandwidth factor relative to FP32
        pt.estimated_bandwidth_factor = static_cast<double>(pt.bit_width) / 32.0;
    }

    /// Select best configurations from results
    void selectBest(AutotuneResult& result) {
        if (result.all_points.empty()) return;

        // Best accuracy (lowest error)
        result.best_accuracy = *std::min_element(result.all_points.begin(), result.all_points.end(),
            [](const TuningPoint& a, const TuningPoint& b) {
                return a.relative_error < b.relative_error;
            });

        // Best energy that meets accuracy requirement
        auto energy_candidates = result.all_points;
        std::sort(energy_candidates.begin(), energy_candidates.end(),
            [](const TuningPoint& a, const TuningPoint& b) {
                return a.estimated_energy_factor < b.estimated_energy_factor;
            });

        result.best_energy = energy_candidates[0]; // Default to lowest energy
        for (const auto& pt : energy_candidates) {
            if (pt.meets_accuracy) {
                result.best_energy = pt;
                break;
            }
        }

        // Recommended: best energy that meets both constraints
        result.recommended = result.all_points[0]; // Default
        double best_score = std::numeric_limits<double>::max();
        for (const auto& pt : result.all_points) {
            if (pt.meets_accuracy && pt.meets_energy) {
                // Score by energy (lower is better)
                double score = pt.estimated_energy_factor;
                if (score < best_score) {
                    best_score = score;
                    result.recommended = pt;
                }
            }
        }

        // If nothing meets both constraints, recommend best accuracy that meets energy
        if (best_score == std::numeric_limits<double>::max()) {
            for (const auto& pt : result.all_points) {
                if (pt.meets_energy) {
                    double score = pt.relative_error;
                    if (score < best_score) {
                        best_score = score;
                        result.recommended = pt;
                    }
                }
            }
        }

        // If still nothing, recommend best accuracy overall
        if (best_score == std::numeric_limits<double>::max()) {
            result.recommended = result.best_accuracy;
        }
    }
};

/// Convenience function to autotune a sqrt-like unary function
inline AutotuneResult autotuneSqrt(double accuracy_req = 1e-4, double energy_budget = 0.5) {
    Autotuner tuner;
    tuner.setAccuracyRequirement(accuracy_req);
    tuner.setEnergyBudget(energy_budget);

    auto inputs = Autotuner::generateLogTestInputs(0.001, 1000.0, 100);
    return tuner.tuneUnaryFunction("sqrt", [](auto x) { using std::sqrt; return sqrt(x); }, inputs);
}

/// Convenience function to autotune an exp-like unary function
inline AutotuneResult autotuneExp(double accuracy_req = 1e-4, double energy_budget = 0.5) {
    Autotuner tuner;
    tuner.setAccuracyRequirement(accuracy_req);
    tuner.setEnergyBudget(energy_budget);

    auto inputs = Autotuner::generateTestInputs(-5.0, 5.0, 100);
    return tuner.tuneUnaryFunction("exp", [](auto x) { using std::exp; return exp(x); }, inputs);
}

/// Convenience function to autotune a log-like unary function
inline AutotuneResult autotuneLog(double accuracy_req = 1e-4, double energy_budget = 0.5) {
    Autotuner tuner;
    tuner.setAccuracyRequirement(accuracy_req);
    tuner.setEnergyBudget(energy_budget);

    auto inputs = Autotuner::generateLogTestInputs(0.001, 1000.0, 100);
    return tuner.tuneUnaryFunction("log", [](auto x) { using std::log; return log(x); }, inputs);
}

/// Convenience function to autotune a sum reduction
inline AutotuneResult autotuneSum(size_t vector_size = 1000,
                                   double accuracy_req = 1e-4,
                                   double energy_budget = 0.5) {
    Autotuner tuner;
    tuner.setAccuracyRequirement(accuracy_req);
    tuner.setEnergyBudget(energy_budget);

    auto vectors = Autotuner::generateTestVectors(10, vector_size, -1.0, 1.0);
    return tuner.tuneReduction("sum", [](const std::vector<double>& v) {
        double sum = 0.0;
        for (double x : v) sum += x;
        return sum;
    }, vectors);
}

}} // namespace sw::universal

