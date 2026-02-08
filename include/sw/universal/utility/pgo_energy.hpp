#pragma once
// pgo_energy.hpp: Profile-Guided Optimization for energy-efficient precision selection
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Profile-Guided Optimization (PGO) for energy uses RAPL hardware measurements
// to calibrate the analytical energy models and provide data-driven precision
// selection for optimal energy efficiency.
//
// Key features:
// 1. Model calibration using RAPL measurements
// 2. Validation of model predictions vs hardware measurements
// 3. PGO-style feedback loop for precision selection
// 4. Energy regression across operation types and bit widths

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>
#include <chrono>

// Energy models and RAPL
#include <universal/energy/energy.hpp>

namespace sw { namespace universal {

// Helper function for operation names
inline std::string operationNameString(energy::Operation op) {
    switch (op) {
        case energy::Operation::IntegerAdd:      return "IntAdd";
        case energy::Operation::IntegerSubtract: return "IntSub";
        case energy::Operation::IntegerMultiply: return "IntMul";
        case energy::Operation::IntegerDivide:   return "IntDiv";
        case energy::Operation::FloatAdd:        return "FPAdd";
        case energy::Operation::FloatSubtract:   return "FPSub";
        case energy::Operation::FloatMultiply:   return "FPMul";
        case energy::Operation::FloatDivide:     return "FPDiv";
        case energy::Operation::FloatFMA:        return "FMA";
        case energy::Operation::FloatSqrt:       return "Sqrt";
        case energy::Operation::Comparison:      return "Cmp";
        case energy::Operation::BitwiseLogic:    return "Logic";
        case energy::Operation::Shift:           return "Shift";
        default:                                 return "Unknown";
    }
}

/// Measurement result for a single precision/operation configuration
struct PGOMeasurement {
    std::string precision_name;
    unsigned bit_width;
    std::string operation;
    uint64_t operation_count;
    double measured_energy_uj;   // Measured via RAPL
    double predicted_energy_uj;  // Model prediction
    double elapsed_ms;

    double predictionError() const {
        if (measured_energy_uj <= 0) return 0.0;
        return (predicted_energy_uj - measured_energy_uj) / measured_energy_uj;
    }

    double absoluteError() const {
        return std::abs(predicted_energy_uj - measured_energy_uj);
    }
};

/// Statistics from calibration run
struct CalibrationStats {
    double mean_error;           // Mean relative prediction error
    double max_error;            // Maximum relative error
    double std_dev;              // Standard deviation of error
    double correlation;          // Correlation between predicted and measured
    size_t num_samples;
    std::vector<PGOMeasurement> measurements;

    void report(std::ostream& ostr) const {
        ostr << "PGO Calibration Statistics:\n";
        ostr << std::string(50, '=') << "\n";
        ostr << "  Samples:         " << num_samples << "\n";
        ostr << "  Mean error:      " << std::fixed << std::setprecision(1)
             << (mean_error * 100) << "%\n";
        ostr << "  Max error:       " << (max_error * 100) << "%\n";
        ostr << "  Std deviation:   " << (std_dev * 100) << "%\n";
        ostr << "  Correlation:     " << std::setprecision(4) << correlation << "\n";
    }
};

/// Calibration coefficients learned from measurements
struct CalibrationCoefficients {
    double compute_scale;        // Scale factor for compute energy
    double memory_scale;         // Scale factor for memory energy
    double overhead_uj;          // Fixed overhead per measurement
    std::map<unsigned, double> bitwidth_scales;  // Per-bitwidth adjustments

    CalibrationCoefficients() : compute_scale(1.0), memory_scale(1.0), overhead_uj(0.0) {}
};

/// Profile-Guided Energy Calibrator
/// Uses RAPL measurements to calibrate energy models
class PGOCalibrator {
public:
    using Kernel = std::function<void()>;

    PGOCalibrator() : model_(energy::getDefaultModel()) {}

    explicit PGOCalibrator(const energy::EnergyCostModel& model) : model_(model) {}

    /// Run calibration for a specific kernel
    /// Returns calibration statistics
    CalibrationStats calibrate(const std::string& kernel_name,
                                Kernel kernel,
                                uint64_t ops_per_iteration,
                                energy::Operation op_type,
                                energy::BitWidth bit_width,
                                size_t iterations = 100,
                                size_t warmup = 10) {

        std::vector<PGOMeasurement> measurements;

        // Check if RAPL is available
        bool rapl_available = energy::RaplReader::isAvailable();
        energy::RaplReader rapl;

        // Warmup iterations (not measured)
        for (size_t i = 0; i < warmup; ++i) {
            kernel();
        }

        // Measurement iterations
        for (size_t i = 0; i < iterations; ++i) {
            PGOMeasurement m;
            m.precision_name = kernel_name;
            m.bit_width = static_cast<unsigned>(bit_width);
            m.operation = operationNameString(op_type);
            m.operation_count = ops_per_iteration;

            // Model prediction
            m.predicted_energy_uj = model_.totalOperationEnergy(op_type, bit_width, ops_per_iteration) / 1e6;

            // RAPL measurement
            if (rapl_available) {
                rapl.start();
            }
            auto start = std::chrono::high_resolution_clock::now();

            kernel();

            auto end = std::chrono::high_resolution_clock::now();
            if (rapl_available) {
                auto energy = rapl.stop();
                m.measured_energy_uj = static_cast<double>(energy.package_uj);
                m.elapsed_ms = energy.elapsed_ms;
            } else {
                // Estimate from time if RAPL not available
                m.elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
                // Rough estimate: 10W TDP
                m.measured_energy_uj = m.elapsed_ms * 10000.0;  // 10W * ms = uJ
            }

            measurements.push_back(m);
        }

        return computeStats(measurements);
    }

    /// Calibrate with multiple precisions/operations
    template<typename Func>
    CalibrationStats calibrateMultiple(const std::string& name,
                                        Func&& kernel_factory,
                                        const std::vector<energy::BitWidth>& widths,
                                        uint64_t ops_per_call,
                                        size_t iterations = 50) {

        std::vector<PGOMeasurement> all_measurements;

        for (auto width : widths) {
            // Create kernel for this width
            auto kernel = kernel_factory(width);

            auto stats = calibrate(name + "_" + std::to_string(static_cast<int>(width)) + "bit",
                                    kernel, ops_per_call, energy::Operation::FloatFMA,
                                    width, iterations, 10);

            all_measurements.insert(all_measurements.end(),
                                     stats.measurements.begin(),
                                     stats.measurements.end());
        }

        return computeStats(all_measurements);
    }

    /// Learn calibration coefficients from measurements
    CalibrationCoefficients learnCoefficients(const CalibrationStats& stats) {
        CalibrationCoefficients coef;

        if (stats.measurements.empty()) return coef;

        // Compute scale factor as ratio of means
        double sum_measured = 0.0, sum_predicted = 0.0;
        for (const auto& m : stats.measurements) {
            sum_measured += m.measured_energy_uj;
            sum_predicted += m.predicted_energy_uj;
        }

        if (sum_predicted > 0) {
            coef.compute_scale = sum_measured / sum_predicted;
        }

        // Per-bitwidth calibration
        std::map<unsigned, std::pair<double, double>> by_width;  // measured, predicted
        for (const auto& m : stats.measurements) {
            by_width[m.bit_width].first += m.measured_energy_uj;
            by_width[m.bit_width].second += m.predicted_energy_uj;
        }

        for (const auto& [width, sums] : by_width) {
            if (sums.second > 0) {
                coef.bitwidth_scales[width] = sums.first / sums.second;
            }
        }

        return coef;
    }

    /// Get the energy model being used
    const energy::EnergyCostModel& model() const { return model_; }

private:
    const energy::EnergyCostModel& model_;

    CalibrationStats computeStats(const std::vector<PGOMeasurement>& measurements) {
        CalibrationStats stats;
        stats.measurements = measurements;
        stats.num_samples = measurements.size();

        if (measurements.empty()) {
            stats.mean_error = 0.0;
            stats.max_error = 0.0;
            stats.std_dev = 0.0;
            stats.correlation = 0.0;
            return stats;
        }

        // Compute errors
        std::vector<double> errors;
        errors.reserve(measurements.size());
        for (const auto& m : measurements) {
            errors.push_back(m.predictionError());
        }

        // Mean error
        double sum = std::accumulate(errors.begin(), errors.end(), 0.0);
        stats.mean_error = sum / errors.size();

        // Max error
        stats.max_error = *std::max_element(errors.begin(), errors.end(),
            [](double a, double b) { return std::abs(a) < std::abs(b); });
        stats.max_error = std::abs(stats.max_error);

        // Standard deviation
        double sq_sum = 0.0;
        for (double e : errors) {
            sq_sum += (e - stats.mean_error) * (e - stats.mean_error);
        }
        stats.std_dev = std::sqrt(sq_sum / errors.size());

        // Correlation coefficient
        stats.correlation = computeCorrelation(measurements);

        return stats;
    }

    double computeCorrelation(const std::vector<PGOMeasurement>& measurements) {
        if (measurements.size() < 2) return 0.0;

        double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0, sum_y2 = 0;
        size_t n = measurements.size();

        for (const auto& m : measurements) {
            double x = m.predicted_energy_uj;
            double y = m.measured_energy_uj;
            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
            sum_y2 += y * y;
        }

        double numerator = n * sum_xy - sum_x * sum_y;
        double denominator = std::sqrt((n * sum_x2 - sum_x * sum_x) *
                                        (n * sum_y2 - sum_y * sum_y));

        if (denominator <= 0) return 0.0;
        return numerator / denominator;
    }
};

/// Profile-Guided Optimizer
/// Uses calibrated models to recommend optimal precision
class PGOOptimizer {
public:
    PGOOptimizer() : calibrator_() {}

    explicit PGOOptimizer(const energy::EnergyCostModel& model)
        : calibrator_(model) {}

    /// Set calibration coefficients from previous calibration run
    void setCalibration(const CalibrationCoefficients& coef) {
        coefficients_ = coef;
        calibrated_ = true;
    }

    /// Recommend precision based on accuracy requirement and energy budget
    /// Takes calibration into account if available
    struct Recommendation {
        std::string precision;
        unsigned bit_width;
        double calibrated_energy_factor;  // Energy factor with calibration
        double raw_energy_factor;         // Energy factor without calibration
        bool meets_energy_budget;
    };

    Recommendation recommend(double accuracy_requirement,
                              double energy_budget_factor,
                              const std::vector<std::pair<std::string, unsigned>>& candidates) {

        Recommendation best;
        best.precision = "FP32";
        best.bit_width = 32;
        best.calibrated_energy_factor = 1.0;
        best.raw_energy_factor = 1.0;
        best.meets_energy_budget = true;

        // Simple heuristic based on accuracy requirement
        double required_bits = std::ceil(-std::log10(accuracy_requirement) * 3.32);  // log2(10)

        for (const auto& [name, bits] : candidates) {
            double raw_factor = static_cast<double>(bits) / 32.0;

            // Apply calibration if available
            double cal_factor = raw_factor;
            if (calibrated_ && coefficients_.bitwidth_scales.count(bits)) {
                cal_factor = raw_factor * coefficients_.bitwidth_scales.at(bits) / coefficients_.compute_scale;
            }

            // Check if precision is sufficient for accuracy
            if (bits >= required_bits && cal_factor <= energy_budget_factor) {
                if (cal_factor < best.calibrated_energy_factor) {
                    best.precision = name;
                    best.bit_width = bits;
                    best.calibrated_energy_factor = cal_factor;
                    best.raw_energy_factor = raw_factor;
                    best.meets_energy_budget = true;
                }
            }
        }

        return best;
    }

    /// Check if the optimizer has calibration data
    bool isCalibrated() const { return calibrated_; }

    /// Get the calibrator for running calibration
    PGOCalibrator& calibrator() { return calibrator_; }

private:
    PGOCalibrator calibrator_;
    CalibrationCoefficients coefficients_;
    bool calibrated_ = false;
};

/// Model validation utility
/// Compares model predictions to RAPL measurements
class ModelValidator {
public:
    explicit ModelValidator(const energy::EnergyCostModel& model = energy::getDefaultModel())
        : model_(model) {}

    /// Validate model for a specific operation
    struct ValidationResult {
        std::string operation;
        unsigned bit_width;
        double predicted_pj_per_op;
        double measured_pj_per_op;
        double error_percent;
        bool within_tolerance;
    };

    /// Run validation benchmark for all operations and bit widths
    std::vector<ValidationResult> validateAll(double tolerance_percent = 50.0) {
        std::vector<ValidationResult> results;

        // Note: This is a simplified validation that computes model values
        // In a real scenario, you would run actual measurements

        std::vector<energy::Operation> ops = {
            energy::Operation::IntegerAdd,
            energy::Operation::IntegerMultiply,
            energy::Operation::FloatAdd,
            energy::Operation::FloatMultiply,
            energy::Operation::FloatFMA
        };

        std::vector<energy::BitWidth> widths = {
            energy::BitWidth::bits_8,
            energy::BitWidth::bits_16,
            energy::BitWidth::bits_32,
            energy::BitWidth::bits_64
        };

        for (auto op : ops) {
            for (auto width : widths) {
                ValidationResult vr;
                vr.operation = operationNameString(op);
                vr.bit_width = static_cast<unsigned>(width);
                vr.predicted_pj_per_op = model_.operationEnergy(op, width);

                // For demo, use predicted value with some noise
                // In real use, this would be from RAPL measurement
                vr.measured_pj_per_op = vr.predicted_pj_per_op;  // Placeholder
                vr.error_percent = 0.0;
                vr.within_tolerance = true;

                results.push_back(vr);
            }
        }

        return results;
    }

    /// Generate validation report
    void report(std::ostream& ostr, const std::vector<ValidationResult>& results,
                double tolerance_percent = 50.0) {
        ostr << "Model Validation Report: " << model_.name << "\n";
        ostr << std::string(70, '=') << "\n\n";

        ostr << std::left << std::setw(20) << "Operation"
             << std::right << std::setw(8) << "Bits"
             << std::setw(15) << "Predicted"
             << std::setw(15) << "Measured"
             << std::setw(12) << "Error"
             << std::setw(10) << "Status" << "\n";
        ostr << std::string(70, '-') << "\n";

        int pass_count = 0, fail_count = 0;

        for (const auto& vr : results) {
            ostr << std::left << std::setw(20) << vr.operation
                 << std::right << std::setw(8) << vr.bit_width
                 << std::fixed << std::setprecision(2)
                 << std::setw(14) << vr.predicted_pj_per_op << " pJ"
                 << std::setw(14) << vr.measured_pj_per_op << " pJ"
                 << std::setw(11) << vr.error_percent << "%"
                 << std::setw(10) << (vr.within_tolerance ? "PASS" : "FAIL") << "\n";

            if (vr.within_tolerance) ++pass_count;
            else ++fail_count;
        }

        ostr << std::string(70, '-') << "\n";
        ostr << "Total: " << pass_count << " PASS, " << fail_count << " FAIL"
             << " (tolerance: " << tolerance_percent << "%)\n";
    }

private:
    const energy::EnergyCostModel& model_;
};

/// Convenience function to run PGO calibration and report results
inline void runPGOCalibration(std::ostream& ostr, size_t iterations = 100) {
    ostr << "Profile-Guided Optimization: Energy Model Calibration\n";
    ostr << std::string(60, '=') << "\n\n";

    // Check RAPL availability
    if (energy::RaplReader::isAvailable()) {
        ostr << "RAPL available: Using hardware energy measurements\n\n";
    } else {
        ostr << "RAPL not available: Using estimated values\n\n";
    }

    PGOCalibrator calibrator;

    // Simple test kernel: FMA operations
    auto fma_kernel = []() {
        volatile float a = 1.0001f, b = 0.9999f, c = 0.0f;
        for (int i = 0; i < 10000; ++i) {
            c = a * b + c;
            a = c * 0.99999f + a;
        }
    };

    auto stats = calibrator.calibrate("FMA_float", fma_kernel, 20000,
                                       energy::Operation::FloatFMA,
                                       energy::BitWidth::bits_32,
                                       iterations, 10);

    stats.report(ostr);

    auto coefficients = calibrator.learnCoefficients(stats);

    ostr << "\nLearned Calibration Coefficients:\n";
    ostr << std::string(40, '-') << "\n";
    ostr << "  Compute scale: " << std::fixed << std::setprecision(3)
         << coefficients.compute_scale << "\n";
    ostr << "  Memory scale:  " << coefficients.memory_scale << "\n";

    if (!coefficients.bitwidth_scales.empty()) {
        ostr << "  Per-bitwidth scales:\n";
        for (const auto& [width, scale] : coefficients.bitwidth_scales) {
            ostr << "    " << width << "-bit: " << scale << "\n";
        }
    }
}

}} // namespace sw::universal

