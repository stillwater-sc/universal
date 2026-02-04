#pragma once
// pareto_explorer.hpp: explore accuracy/energy trade-offs for mixed-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// The pareto_explorer finds the Pareto-optimal frontier of precision
// configurations, balancing accuracy against energy consumption.
//
// Usage:
//   #include <universal/utility/pareto_explorer.hpp>
//
//   using namespace sw::universal;
//
//   ParetoExplorer explorer;
//   explorer.addConfiguration("FP32", 32, 1e-7, 1.5);   // precision, bits, accuracy, energy_factor
//   explorer.addConfiguration("FP16", 16, 1e-3, 0.47);
//   explorer.addConfiguration("INT8", 8, 1e-2, 0.2);
//
//   auto frontier = explorer.computeFrontier();
//   explorer.report(std::cout);

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

#include "algorithm_profiler.hpp"

namespace sw { namespace universal {

/// A single precision configuration with its characteristics
struct PrecisionConfig {
    std::string name;           // Configuration name (e.g., "FP16", "posit<16,1>")
    int bit_width;              // Bit width
    double relative_accuracy;   // Relative accuracy (machine epsilon or measured)
    double energy_factor;       // Energy relative to FP32 (FP32 = 1.0)
    double memory_factor;       // Memory relative to FP32 (FP32 = 1.0)

    // For Pareto analysis
    bool is_pareto_optimal;     // Is this on the Pareto frontier?
    double accuracy_rank;       // Rank by accuracy (lower = more accurate)
    double energy_rank;         // Rank by energy (lower = less energy)

    PrecisionConfig()
        : name("unknown"), bit_width(32)
        , relative_accuracy(1e-7), energy_factor(1.0), memory_factor(1.0)
        , is_pareto_optimal(false), accuracy_rank(0), energy_rank(0) {}

    PrecisionConfig(const std::string& n, int bits, double acc, double energy, double mem = 0.0)
        : name(n), bit_width(bits)
        , relative_accuracy(acc), energy_factor(energy)
        , memory_factor(mem > 0 ? mem : bits / 32.0)
        , is_pareto_optimal(false), accuracy_rank(0), energy_rank(0) {}
};

/// Result of Pareto analysis
struct ParetoResult {
    std::vector<PrecisionConfig> all_configs;       // All configurations
    std::vector<PrecisionConfig> frontier;          // Pareto-optimal configurations
    std::vector<PrecisionConfig> dominated;         // Dominated configurations

    /// Get the best config for a given accuracy requirement
    PrecisionConfig bestForAccuracy(double required_accuracy) const {
        PrecisionConfig best;
        best.energy_factor = 1e9;  // Start with worst

        for (const auto& cfg : frontier) {
            if (cfg.relative_accuracy <= required_accuracy) {
                if (cfg.energy_factor < best.energy_factor) {
                    best = cfg;
                }
            }
        }
        return best;
    }

    /// Get the best config for a given energy budget
    PrecisionConfig bestForEnergy(double max_energy_factor) const {
        PrecisionConfig best;
        best.relative_accuracy = 1.0;  // Start with worst accuracy

        for (const auto& cfg : frontier) {
            if (cfg.energy_factor <= max_energy_factor) {
                if (cfg.relative_accuracy < best.relative_accuracy) {
                    best = cfg;
                }
            }
        }
        return best;
    }
};

/// Explorer for Pareto-optimal precision configurations
class ParetoExplorer {
public:
    ParetoExplorer() {
        initializeStandardConfigs();
    }

    /// Add a custom configuration
    void addConfiguration(const PrecisionConfig& config) {
        configs_.push_back(config);
    }

    /// Add a configuration with parameters
    void addConfiguration(const std::string& name, int bits,
                          double accuracy, double energy_factor,
                          double memory_factor = 0.0) {
        configs_.emplace_back(name, bits, accuracy, energy_factor, memory_factor);
    }

    /// Clear all configurations
    void clear() {
        configs_.clear();
    }

    /// Reset to standard configurations
    void resetToStandard() {
        configs_.clear();
        initializeStandardConfigs();
    }

    /// Get all configurations
    const std::vector<PrecisionConfig>& configurations() const {
        return configs_;
    }

    /// Compute the Pareto frontier
    ParetoResult computeFrontier() const {
        ParetoResult result;
        result.all_configs = configs_;

        // A configuration is Pareto-optimal if no other configuration
        // is better in all objectives (lower accuracy error AND lower energy)
        for (auto& cfg : result.all_configs) {
            cfg.is_pareto_optimal = true;

            for (const auto& other : result.all_configs) {
                if (&cfg == &other) continue;

                // Check if 'other' dominates 'cfg'
                // Domination: other is <= in all objectives and < in at least one
                bool other_not_worse_acc = other.relative_accuracy <= cfg.relative_accuracy;
                bool other_not_worse_energy = other.energy_factor <= cfg.energy_factor;
                bool other_strictly_better = (other.relative_accuracy < cfg.relative_accuracy) ||
                                              (other.energy_factor < cfg.energy_factor);

                if (other_not_worse_acc && other_not_worse_energy && other_strictly_better) {
                    cfg.is_pareto_optimal = false;
                    break;
                }
            }

            if (cfg.is_pareto_optimal) {
                result.frontier.push_back(cfg);
            } else {
                result.dominated.push_back(cfg);
            }
        }

        // Sort frontier by energy (lowest first)
        std::sort(result.frontier.begin(), result.frontier.end(),
                  [](const PrecisionConfig& a, const PrecisionConfig& b) {
                      return a.energy_factor < b.energy_factor;
                  });

        return result;
    }

    /// Find best configuration for given accuracy requirement
    PrecisionConfig recommendForAccuracy(double required_accuracy) const {
        auto result = computeFrontier();
        return result.bestForAccuracy(required_accuracy);
    }

    /// Find best configuration for given energy budget
    PrecisionConfig recommendForEnergy(double max_energy_factor) const {
        auto result = computeFrontier();
        return result.bestForEnergy(max_energy_factor);
    }

    /// Generate report
    void report(std::ostream& ostr) const {
        auto result = computeFrontier();

        ostr << "Pareto Analysis: Accuracy vs Energy Trade-offs\n";
        ostr << std::string(70, '=') << "\n\n";

        ostr << "All Configurations:\n";
        ostr << std::string(70, '-') << "\n";
        ostr << std::left << std::setw(20) << "Configuration"
             << std::right << std::setw(10) << "Bits"
             << std::setw(15) << "Accuracy"
             << std::setw(12) << "Energy"
             << std::setw(12) << "Pareto?" << "\n";
        ostr << std::string(70, '-') << "\n";

        for (const auto& cfg : result.all_configs) {
            ostr << std::left << std::setw(20) << cfg.name
                 << std::right << std::setw(10) << cfg.bit_width
                 << std::scientific << std::setprecision(1)
                 << std::setw(15) << cfg.relative_accuracy
                 << std::fixed << std::setprecision(2)
                 << std::setw(11) << cfg.energy_factor << "x"
                 << std::setw(12) << (cfg.is_pareto_optimal ? "YES" : "no") << "\n";
        }

        ostr << "\nPareto Frontier (optimal trade-offs):\n";
        ostr << std::string(50, '-') << "\n";
        for (const auto& cfg : result.frontier) {
            ostr << "  " << cfg.name << ": accuracy=" << std::scientific
                 << cfg.relative_accuracy << ", energy=" << std::fixed
                 << std::setprecision(2) << cfg.energy_factor << "x\n";
        }

        ostr << "\nRecommendations:\n";
        ostr << std::string(50, '-') << "\n";

        // Recommendations for common accuracy requirements
        std::vector<std::pair<double, std::string>> accuracy_levels = {
            {1e-2, "Low (1e-2) - ML inference"},
            {1e-4, "Medium (1e-4) - Graphics"},
            {1e-7, "High (1e-7) - Scientific"},
            {1e-10, "Very High (1e-10) - Financial"}
        };

        for (const auto& level : accuracy_levels) {
            auto best = result.bestForAccuracy(level.first);
            if (!best.name.empty() && best.name != "unknown") {
                ostr << "  " << level.second << ": " << best.name
                     << " (energy=" << std::fixed << std::setprecision(2)
                     << best.energy_factor << "x)\n";
            } else {
                ostr << "  " << level.second << ": No suitable type\n";
            }
        }
    }

    /// Plot frontier as ASCII art
    void plotFrontier(std::ostream& ostr, int width = 60, int height = 20) const {
        auto result = computeFrontier();

        ostr << "\nPareto Frontier Plot (Accuracy vs Energy)\n";
        ostr << std::string(width + 5, '=') << "\n\n";

        // Find ranges
        double min_acc = 1e-16, max_acc = 1.0;
        double max_energy = 1.0;
        (void)min_acc; (void)max_acc;  // Used in normalization

        for (const auto& cfg : result.all_configs) {
            max_energy = std::max(max_energy, cfg.energy_factor);
        }
        max_energy *= 1.1;  // Add margin

        // Create plot grid
        std::vector<std::string> grid(height, std::string(width, ' '));

        // Plot points
        for (const auto& cfg : result.all_configs) {
            // Convert to log scale for accuracy
            double log_acc = std::log10(cfg.relative_accuracy);
            double norm_acc = (log_acc - std::log10(min_acc)) /
                              (std::log10(max_acc) - std::log10(min_acc));
            double norm_energy = cfg.energy_factor / max_energy;

            int x = static_cast<int>(norm_energy * (width - 1));
            int y = static_cast<int>((1.0 - norm_acc) * (height - 1));

            x = std::max(0, std::min(width - 1, x));
            y = std::max(0, std::min(height - 1, y));

            char marker = cfg.is_pareto_optimal ? '*' : 'o';
            grid[y][x] = marker;
        }

        // Draw axes and grid
        ostr << "  Accuracy\n";
        ostr << "  (better)\n";
        ostr << "     ^\n";
        for (int y = 0; y < height; ++y) {
            if (y == 0) ostr << "High |";
            else if (y == height - 1) ostr << "Low  |";
            else ostr << "     |";
            ostr << grid[y] << "\n";
        }
        ostr << "     +" << std::string(width, '-') << "> Energy (worse)\n";
        ostr << "     Low" << std::string(width - 12, ' ') << "High\n\n";
        ostr << "  Legend: * = Pareto-optimal, o = dominated\n";
    }

private:
    std::vector<PrecisionConfig> configs_;

    void initializeStandardConfigs() {
        // IEEE floating-point types
        configs_.emplace_back("FP64 (double)", 64, 2.2e-16, 3.53, 2.0);
        configs_.emplace_back("FP32 (float)", 32, 1.2e-7, 1.0, 1.0);
        configs_.emplace_back("FP16 (half)", 16, 9.8e-4, 0.31, 0.5);
        configs_.emplace_back("BF16", 16, 7.8e-3, 0.31, 0.5);

        // Posit types (approximate accuracy based on dynamic range utilization)
        configs_.emplace_back("posit<64,3>", 64, 3.5e-18, 1.73, 2.0);
        configs_.emplace_back("posit<32,2>", 32, 7.5e-9, 0.5, 1.0);
        configs_.emplace_back("posit<16,1>", 16, 2.4e-4, 0.15, 0.5);
        configs_.emplace_back("posit<8,0>", 8, 0.125, 0.07, 0.25);

        // Integer/fixed-point (accuracy depends heavily on scaling)
        configs_.emplace_back("INT8", 8, 3.9e-3, 0.13, 0.25);  // 1/256
        configs_.emplace_back("INT16", 16, 1.5e-5, 0.15, 0.5);  // 1/65536

        // LNS (logarithmic number system)
        configs_.emplace_back("lns<16,8>", 16, 7.8e-3, 0.2, 0.5);
        configs_.emplace_back("lns<32,16>", 32, 3.1e-5, 0.67, 1.0);
    }
};

/// Convenience function: find best precision for GEMM
inline PrecisionConfig recommendGEMMPrecision(
        uint64_t M, uint64_t N, uint64_t K,
        double required_accuracy,
        double energy_budget_factor = 1.0) {

    ParetoExplorer explorer;
    auto result = explorer.computeFrontier();

    // Filter by accuracy first
    std::vector<PrecisionConfig> candidates;
    for (const auto& cfg : result.frontier) {
        if (cfg.relative_accuracy <= required_accuracy) {
            candidates.push_back(cfg);
        }
    }

    if (candidates.empty()) {
        // No type meets accuracy, return highest accuracy available
        return result.frontier.back();
    }

    // Among candidates, find lowest energy
    auto best = std::min_element(candidates.begin(), candidates.end(),
        [](const PrecisionConfig& a, const PrecisionConfig& b) {
            return a.energy_factor < b.energy_factor;
        });

    return *best;
}

/// Generate mixed-precision recommendation for an algorithm
struct MixedPrecisionRecommendation {
    std::string algorithm;
    PrecisionConfig input_precision;
    PrecisionConfig compute_precision;
    PrecisionConfig accumulator_precision;
    PrecisionConfig output_precision;
    double estimated_energy_factor;  // Relative to all-FP32
    std::string rationale;
};

inline MixedPrecisionRecommendation recommendMixedPrecision(
        const std::string& algorithm,
        double required_output_accuracy,
        double energy_budget = 1.0) {

    MixedPrecisionRecommendation rec;
    rec.algorithm = algorithm;

    ParetoExplorer explorer;
    auto result = explorer.computeFrontier();

    // Heuristic: accumulator needs higher precision than inputs
    // Output precision matches required accuracy
    // Inputs can be lower precision

    // Find output precision
    rec.output_precision = result.bestForAccuracy(required_output_accuracy);

    // Accumulator: one level higher than output for numerical stability
    if (rec.output_precision.bit_width <= 16) {
        rec.accumulator_precision = result.bestForAccuracy(required_output_accuracy * 1e-3);
    } else {
        rec.accumulator_precision = rec.output_precision;
    }

    // Compute precision: same as output or lower if energy-constrained
    if (energy_budget < 0.5) {
        rec.compute_precision = result.bestForEnergy(energy_budget);
    } else {
        rec.compute_precision = rec.output_precision;
    }

    // Input precision: can often be lower
    rec.input_precision = result.bestForEnergy(energy_budget * 0.5);
    if (rec.input_precision.relative_accuracy > required_output_accuracy * 100) {
        // Input too inaccurate, bump up
        rec.input_precision = rec.compute_precision;
    }

    // Estimate combined energy factor
    rec.estimated_energy_factor =
        0.2 * rec.input_precision.energy_factor +    // Memory-bound
        0.5 * rec.compute_precision.energy_factor +   // Compute-bound
        0.2 * rec.accumulator_precision.energy_factor +
        0.1 * rec.output_precision.energy_factor;

    // Generate rationale
    std::stringstream ss;
    ss << "For " << algorithm << " with " << std::scientific << std::setprecision(0)
       << required_output_accuracy << " accuracy: ";
    ss << "Use " << rec.input_precision.name << " inputs, ";
    ss << rec.compute_precision.name << " compute, ";
    ss << rec.accumulator_precision.name << " accumulator";
    rec.rationale = ss.str();

    return rec;
}

}} // namespace sw::universal
