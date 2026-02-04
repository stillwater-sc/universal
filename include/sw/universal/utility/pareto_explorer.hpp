#pragma once
// pareto_explorer.hpp: explore accuracy/energy/bandwidth trade-offs for mixed-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// The pareto_explorer finds the Pareto-optimal frontier of precision
// configurations, balancing three objectives:
// 1. Accuracy (relative error tolerance)
// 2. Energy consumption (compute energy)
// 3. Memory bandwidth (data transfer requirements)
//
// Usage:
//   #include <universal/utility/pareto_explorer.hpp>
//
//   using namespace sw::universal;
//
//   ParetoExplorer explorer;
//   explorer.addConfiguration("FP32", 32, 1e-7, 1.0, 1.0);  // name, bits, accuracy, energy, bandwidth
//   explorer.addConfiguration("FP16", 16, 1e-3, 0.31, 0.5);
//   explorer.addConfiguration("INT8", 8, 1e-2, 0.13, 0.25);
//
//   // 2D analysis (accuracy vs energy)
//   auto frontier2d = explorer.computeFrontier();
//
//   // 3D analysis (accuracy vs energy vs bandwidth)
//   auto frontier3d = explorer.computeFrontier3D();
//
//   // Query with memory bandwidth constraint
//   auto best = explorer.recommendForAlgorithm(1e-4, 0.5, 100.0);  // accuracy, energy, GB/s available
//
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
    double energy_factor;       // Compute energy relative to FP32 (FP32 = 1.0)
    double bandwidth_factor;    // Memory bandwidth relative to FP32 (FP32 = 1.0)
    double memory_factor;       // Memory footprint relative to FP32 (same as bandwidth for most types)

    // For Pareto analysis
    bool is_pareto_optimal;     // Is this on the 2D Pareto frontier?
    bool is_pareto_optimal_3d;  // Is this on the 3D Pareto frontier?
    double accuracy_rank;       // Rank by accuracy (lower = more accurate)
    double energy_rank;         // Rank by energy (lower = less energy)
    double bandwidth_rank;      // Rank by bandwidth (lower = less bandwidth)

    // Derived metrics for algorithm selection
    double ops_per_byte;        // Arithmetic intensity threshold where this type excels
    double roofline_crossover;  // AI where compute and memory energy are equal

    PrecisionConfig()
        : name("unknown"), bit_width(32)
        , relative_accuracy(1e-7), energy_factor(1.0), bandwidth_factor(1.0), memory_factor(1.0)
        , is_pareto_optimal(false), is_pareto_optimal_3d(false)
        , accuracy_rank(0), energy_rank(0), bandwidth_rank(0)
        , ops_per_byte(0), roofline_crossover(0) {}

    PrecisionConfig(const std::string& n, int bits, double acc, double energy, double bandwidth = 0.0)
        : name(n), bit_width(bits)
        , relative_accuracy(acc), energy_factor(energy)
        , bandwidth_factor(bandwidth > 0 ? bandwidth : bits / 32.0)
        , memory_factor(bandwidth > 0 ? bandwidth : bits / 32.0)
        , is_pareto_optimal(false), is_pareto_optimal_3d(false)
        , accuracy_rank(0), energy_rank(0), bandwidth_rank(0)
        , ops_per_byte(0), roofline_crossover(0) {
        // Compute roofline crossover point
        // At this arithmetic intensity, compute energy = memory energy
        // Using typical DRAM energy of ~20 pJ/byte and FP32 FMA of ~1.5 pJ
        double compute_energy_per_op = energy_factor * 1.5;  // pJ
        double memory_energy_per_byte = bandwidth_factor * 5.0;  // pJ (L2/L3 level)
        if (memory_energy_per_byte > 0) {
            roofline_crossover = memory_energy_per_byte / compute_energy_per_op;
        }
    }
};

/// Algorithm characteristics for memory-aware precision selection
/// This is a lightweight struct for Pareto analysis, distinct from
/// the more comprehensive AlgorithmProfile in algorithm_profiler.hpp
struct AlgorithmCharacteristics {
    std::string name;
    double arithmetic_intensity;   // Ops per byte (higher = compute-bound)
    double working_set_bytes;      // Total memory footprint
    double available_bandwidth_gbps;  // System memory bandwidth in GB/s
    bool is_memory_bound;          // True if bandwidth-limited

    AlgorithmCharacteristics()
        : name("unknown"), arithmetic_intensity(1.0)
        , working_set_bytes(0), available_bandwidth_gbps(100.0)
        , is_memory_bound(false) {}

    AlgorithmCharacteristics(const std::string& n, double ai, double ws_bytes = 0, double bw_gbps = 100.0)
        : name(n), arithmetic_intensity(ai)
        , working_set_bytes(ws_bytes), available_bandwidth_gbps(bw_gbps)
        , is_memory_bound(ai < 10.0) {}  // Rough threshold: AI < 10 is memory-bound
};

/// Result of Pareto analysis
struct ParetoResult {
    std::vector<PrecisionConfig> all_configs;       // All configurations
    std::vector<PrecisionConfig> frontier;          // 2D Pareto-optimal (accuracy vs energy)
    std::vector<PrecisionConfig> frontier_3d;       // 3D Pareto-optimal (accuracy vs energy vs bandwidth)
    std::vector<PrecisionConfig> dominated;         // Dominated configurations

    /// Get the best config for a given accuracy requirement (2D: minimize energy)
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

    /// Get the best config for a given energy budget (2D: minimize error)
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

    /// Get the best config for a given bandwidth constraint
    PrecisionConfig bestForBandwidth(double max_bandwidth_factor) const {
        PrecisionConfig best;
        best.relative_accuracy = 1.0;

        for (const auto& cfg : frontier_3d) {
            if (cfg.bandwidth_factor <= max_bandwidth_factor) {
                if (cfg.relative_accuracy < best.relative_accuracy) {
                    best = cfg;
                }
            }
        }
        return best;
    }

    /// Get best config with combined constraints (3D)
    PrecisionConfig bestForConstraints(double required_accuracy,
                                        double max_energy_factor,
                                        double max_bandwidth_factor) const {
        PrecisionConfig best;
        best.energy_factor = 1e9;

        for (const auto& cfg : frontier_3d) {
            if (cfg.relative_accuracy <= required_accuracy &&
                cfg.energy_factor <= max_energy_factor &&
                cfg.bandwidth_factor <= max_bandwidth_factor) {
                // Among feasible, pick lowest combined cost
                double cost = cfg.energy_factor + cfg.bandwidth_factor;
                double best_cost = best.energy_factor + best.bandwidth_factor;
                if (cost < best_cost) {
                    best = cfg;
                }
            }
        }
        return best;
    }

    /// Get best config for a specific algorithm profile
    /// Balances compute and memory energy based on arithmetic intensity
    PrecisionConfig bestForAlgorithm(double required_accuracy,
                                      const AlgorithmCharacteristics& algo) const {
        PrecisionConfig best;
        double best_total_energy = 1e9;

        for (const auto& cfg : frontier_3d) {
            if (cfg.relative_accuracy <= required_accuracy) {
                // Total energy = compute_energy + memory_energy
                // compute_energy proportional to energy_factor
                // memory_energy proportional to bandwidth_factor / arithmetic_intensity
                double compute_cost = cfg.energy_factor;
                double memory_cost = cfg.bandwidth_factor / std::max(0.1, algo.arithmetic_intensity);
                double total_cost = compute_cost + memory_cost;

                if (total_cost < best_total_energy) {
                    best_total_energy = total_cost;
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

    /// Compute the 2D Pareto frontier (accuracy vs energy)
    ParetoResult computeFrontier() const {
        ParetoResult result;
        result.all_configs = configs_;

        // A configuration is Pareto-optimal if no other configuration
        // is better in all objectives (lower accuracy error AND lower energy)
        for (auto& cfg : result.all_configs) {
            cfg.is_pareto_optimal = true;

            for (const auto& other : result.all_configs) {
                if (&cfg == &other) continue;

                // Check if 'other' dominates 'cfg' in 2D
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

    /// Compute the 3D Pareto frontier (accuracy vs energy vs bandwidth)
    ParetoResult computeFrontier3D() const {
        ParetoResult result;
        result.all_configs = configs_;

        // First compute 2D frontier
        for (auto& cfg : result.all_configs) {
            cfg.is_pareto_optimal = true;

            for (const auto& other : result.all_configs) {
                if (&cfg == &other) continue;

                bool other_not_worse_acc = other.relative_accuracy <= cfg.relative_accuracy;
                bool other_not_worse_energy = other.energy_factor <= cfg.energy_factor;
                bool other_strictly_better_2d = (other.relative_accuracy < cfg.relative_accuracy) ||
                                                 (other.energy_factor < cfg.energy_factor);

                if (other_not_worse_acc && other_not_worse_energy && other_strictly_better_2d) {
                    cfg.is_pareto_optimal = false;
                    break;
                }
            }

            if (cfg.is_pareto_optimal) {
                result.frontier.push_back(cfg);
            }
        }

        // Compute 3D frontier
        for (auto& cfg : result.all_configs) {
            cfg.is_pareto_optimal_3d = true;

            for (const auto& other : result.all_configs) {
                if (&cfg == &other) continue;

                // Check if 'other' dominates 'cfg' in 3D
                bool other_not_worse_acc = other.relative_accuracy <= cfg.relative_accuracy;
                bool other_not_worse_energy = other.energy_factor <= cfg.energy_factor;
                bool other_not_worse_bw = other.bandwidth_factor <= cfg.bandwidth_factor;
                bool other_strictly_better = (other.relative_accuracy < cfg.relative_accuracy) ||
                                              (other.energy_factor < cfg.energy_factor) ||
                                              (other.bandwidth_factor < cfg.bandwidth_factor);

                if (other_not_worse_acc && other_not_worse_energy &&
                    other_not_worse_bw && other_strictly_better) {
                    cfg.is_pareto_optimal_3d = false;
                    break;
                }
            }

            if (cfg.is_pareto_optimal_3d) {
                result.frontier_3d.push_back(cfg);
            } else if (!cfg.is_pareto_optimal) {
                result.dominated.push_back(cfg);
            }
        }

        // Sort frontiers
        std::sort(result.frontier.begin(), result.frontier.end(),
                  [](const PrecisionConfig& a, const PrecisionConfig& b) {
                      return a.energy_factor < b.energy_factor;
                  });

        std::sort(result.frontier_3d.begin(), result.frontier_3d.end(),
                  [](const PrecisionConfig& a, const PrecisionConfig& b) {
                      // Sort by combined metric: energy + bandwidth
                      return (a.energy_factor + a.bandwidth_factor) <
                             (b.energy_factor + b.bandwidth_factor);
                  });

        return result;
    }

    /// Find best configuration for given accuracy requirement (2D)
    PrecisionConfig recommendForAccuracy(double required_accuracy) const {
        auto result = computeFrontier();
        return result.bestForAccuracy(required_accuracy);
    }

    /// Find best configuration for given energy budget (2D)
    PrecisionConfig recommendForEnergy(double max_energy_factor) const {
        auto result = computeFrontier();
        return result.bestForEnergy(max_energy_factor);
    }

    /// Find best configuration for given bandwidth constraint (3D)
    PrecisionConfig recommendForBandwidth(double max_bandwidth_factor) const {
        auto result = computeFrontier3D();
        return result.bestForBandwidth(max_bandwidth_factor);
    }

    /// Find best configuration with combined constraints (3D)
    PrecisionConfig recommendWithConstraints(double required_accuracy,
                                              double max_energy_factor,
                                              double max_bandwidth_factor) const {
        auto result = computeFrontier3D();
        return result.bestForConstraints(required_accuracy, max_energy_factor, max_bandwidth_factor);
    }

    /// Find best configuration for a specific algorithm profile
    /// This is the recommended method for memory-aware selection
    PrecisionConfig recommendForAlgorithm(double required_accuracy,
                                           const AlgorithmCharacteristics& algo) const {
        auto result = computeFrontier3D();
        return result.bestForAlgorithm(required_accuracy, algo);
    }

    /// Create algorithm profile from characteristics
    static AlgorithmCharacteristics profileAlgorithm(const std::string& name,
                                              uint64_t total_ops,
                                              uint64_t total_bytes,
                                              double bandwidth_gbps = 100.0) {
        double ai = (total_bytes > 0) ? static_cast<double>(total_ops) / total_bytes : 1.0;
        return AlgorithmCharacteristics(name, ai, static_cast<double>(total_bytes), bandwidth_gbps);
    }

    /// Common algorithm profiles
    static AlgorithmCharacteristics profileDotProduct(uint64_t n, int elem_bytes = 4) {
        // Dot product: 2n ops (n muls + n adds), 2n elements read
        uint64_t ops = 2 * n;
        uint64_t bytes = 2 * n * static_cast<uint64_t>(elem_bytes);
        return AlgorithmCharacteristics("dot_product", static_cast<double>(ops) / bytes, static_cast<double>(bytes));
    }

    static AlgorithmCharacteristics profileGEMM(uint64_t M, uint64_t N, uint64_t K, int elem_bytes = 4) {
        // GEMM: 2*M*N*K ops, (M*K + K*N + M*N) * elem_bytes
        uint64_t ops = 2 * M * N * K;
        uint64_t bytes = (M * K + K * N + M * N) * static_cast<uint64_t>(elem_bytes);
        double ai = static_cast<double>(ops) / bytes;
        return AlgorithmCharacteristics("GEMM", ai, static_cast<double>(bytes));
    }

    static AlgorithmCharacteristics profileConv2D(uint64_t H, uint64_t W, uint64_t C_in,
                                           uint64_t C_out, uint64_t K, int elem_bytes = 4) {
        // Conv2D (naive): 2*H*W*C_in*C_out*K*K ops
        uint64_t ops = 2 * H * W * C_in * C_out * K * K;
        uint64_t input_bytes = H * W * C_in * static_cast<uint64_t>(elem_bytes);
        uint64_t kernel_bytes = K * K * C_in * C_out * static_cast<uint64_t>(elem_bytes);
        uint64_t output_bytes = H * W * C_out * static_cast<uint64_t>(elem_bytes);
        uint64_t total_bytes = input_bytes + kernel_bytes + output_bytes;
        double ai = static_cast<double>(ops) / total_bytes;
        return AlgorithmCharacteristics("Conv2D", ai, static_cast<double>(total_bytes));
    }

    /// Generate report
    void report(std::ostream& ostr) const {
        auto result = computeFrontier3D();

        ostr << "Pareto Analysis: Accuracy vs Energy vs Bandwidth Trade-offs\n";
        ostr << std::string(85, '=') << "\n\n";

        ostr << "All Configurations:\n";
        ostr << std::string(85, '-') << "\n";
        ostr << std::left << std::setw(18) << "Configuration"
             << std::right << std::setw(6) << "Bits"
             << std::setw(12) << "Accuracy"
             << std::setw(10) << "Energy"
             << std::setw(10) << "BW"
             << std::setw(10) << "2D"
             << std::setw(10) << "3D" << "\n";
        ostr << std::string(85, '-') << "\n";

        for (const auto& cfg : result.all_configs) {
            ostr << std::left << std::setw(18) << cfg.name
                 << std::right << std::setw(6) << cfg.bit_width
                 << std::scientific << std::setprecision(1)
                 << std::setw(12) << cfg.relative_accuracy
                 << std::fixed << std::setprecision(2)
                 << std::setw(9) << cfg.energy_factor << "x"
                 << std::setw(9) << cfg.bandwidth_factor << "x"
                 << std::setw(10) << (cfg.is_pareto_optimal ? "YES" : "no")
                 << std::setw(10) << (cfg.is_pareto_optimal_3d ? "YES" : "no") << "\n";
        }

        ostr << "\n2D Pareto Frontier (accuracy vs energy):\n";
        ostr << std::string(60, '-') << "\n";
        for (const auto& cfg : result.frontier) {
            ostr << "  " << std::left << std::setw(16) << cfg.name
                 << ": acc=" << std::scientific << std::setprecision(1) << cfg.relative_accuracy
                 << ", energy=" << std::fixed << std::setprecision(2) << cfg.energy_factor << "x\n";
        }

        ostr << "\n3D Pareto Frontier (accuracy vs energy vs bandwidth):\n";
        ostr << std::string(60, '-') << "\n";
        for (const auto& cfg : result.frontier_3d) {
            ostr << "  " << std::left << std::setw(16) << cfg.name
                 << ": acc=" << std::scientific << std::setprecision(1) << cfg.relative_accuracy
                 << ", energy=" << std::fixed << std::setprecision(2) << cfg.energy_factor << "x"
                 << ", bw=" << cfg.bandwidth_factor << "x\n";
        }

        ostr << "\nRecommendations by Accuracy:\n";
        ostr << std::string(60, '-') << "\n";

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
                     << best.energy_factor << "x, bw=" << best.bandwidth_factor << "x)\n";
            } else {
                ostr << "  " << level.second << ": No suitable type\n";
            }
        }

        ostr << "\nRecommendations by Algorithm Type:\n";
        ostr << std::string(60, '-') << "\n";

        // Compute-bound vs memory-bound recommendations
        auto compute_bound = AlgorithmCharacteristics("compute_bound", 100.0);  // High AI
        auto memory_bound = AlgorithmCharacteristics("memory_bound", 1.0);       // Low AI

        auto best_compute = result.bestForAlgorithm(1e-4, compute_bound);
        auto best_memory = result.bestForAlgorithm(1e-4, memory_bound);

        ostr << "  Compute-bound (AI>10): " << best_compute.name
             << " (energy=" << std::fixed << std::setprecision(2)
             << best_compute.energy_factor << "x)\n";
        ostr << "  Memory-bound (AI<10):  " << best_memory.name
             << " (bw=" << best_memory.bandwidth_factor << "x)\n";
    }

    /// Plot frontier as ASCII art (2D: accuracy vs energy)
    void plotFrontier(std::ostream& ostr, int width = 60, int height = 20) const {
        auto result = computeFrontier3D();

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

            // Use different markers for 2D vs 3D Pareto-optimal
            char marker;
            if (cfg.is_pareto_optimal && cfg.is_pareto_optimal_3d) marker = '#';
            else if (cfg.is_pareto_optimal) marker = '*';
            else if (cfg.is_pareto_optimal_3d) marker = '+';
            else marker = 'o';
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
        ostr << "  Legend: # = both 2D+3D optimal, * = 2D optimal, + = 3D optimal, o = dominated\n";
    }

    /// Plot bandwidth dimension (bandwidth vs energy)
    void plotBandwidth(std::ostream& ostr, int width = 60, int height = 15) const {
        auto result = computeFrontier3D();

        ostr << "\nBandwidth vs Energy Plot\n";
        ostr << std::string(width + 5, '=') << "\n\n";

        double max_energy = 1.0, max_bw = 1.0;
        for (const auto& cfg : result.all_configs) {
            max_energy = std::max(max_energy, cfg.energy_factor);
            max_bw = std::max(max_bw, cfg.bandwidth_factor);
        }
        max_energy *= 1.1;
        max_bw *= 1.1;

        std::vector<std::string> grid(height, std::string(width, ' '));

        for (const auto& cfg : result.all_configs) {
            double norm_energy = cfg.energy_factor / max_energy;
            double norm_bw = cfg.bandwidth_factor / max_bw;

            int x = static_cast<int>(norm_energy * (width - 1));
            int y = static_cast<int>((1.0 - norm_bw) * (height - 1));

            x = std::max(0, std::min(width - 1, x));
            y = std::max(0, std::min(height - 1, y));

            char marker = cfg.is_pareto_optimal_3d ? '*' : 'o';
            grid[y][x] = marker;
        }

        ostr << "  Bandwidth\n";
        ostr << "  (lower=better)\n";
        ostr << "     ^\n";
        for (int y = 0; y < height; ++y) {
            if (y == 0) ostr << "High |";
            else if (y == height - 1) ostr << "Low  |";
            else ostr << "     |";
            ostr << grid[y] << "\n";
        }
        ostr << "     +" << std::string(width, '-') << "> Energy (higher=worse)\n";
        ostr << "     Low" << std::string(width - 12, ' ') << "High\n\n";
        ostr << "  Note: Low bandwidth + Low energy = optimal for memory-bound algorithms\n";
    }

    /// Generate roofline-style analysis for algorithm selection
    void rooflineAnalysis(std::ostream& ostr, double system_bandwidth_gbps = 100.0) const {
        auto result = computeFrontier3D();

        ostr << "\nRoofline Analysis for Algorithm Selection\n";
        ostr << std::string(70, '=') << "\n\n";
        ostr << "System memory bandwidth: " << system_bandwidth_gbps << " GB/s\n\n";

        // Common algorithm profiles
        std::vector<AlgorithmCharacteristics> algos = {
            {"Dot product (n=1M)", 1.0, 8e6},
            {"GEMM (1024x1024)", 341.0, 12e6},
            {"GEMM (256x256)", 85.0, 0.75e6},
            {"Conv2D (224x224, 3->64)", 6.0, 37e6},
            {"Stencil (3D, 27-pt)", 3.4, 100e6}
        };

        ostr << std::left << std::setw(28) << "Algorithm"
             << std::right << std::setw(8) << "AI"
             << std::setw(12) << "Type"
             << std::setw(22) << "Best Precision (1e-4)" << "\n";
        ostr << std::string(70, '-') << "\n";

        for (const auto& algo : algos) {
            auto best = result.bestForAlgorithm(1e-4, algo);
            std::string bound_type = algo.is_memory_bound ? "mem-bound" : "compute";

            ostr << std::left << std::setw(28) << algo.name
                 << std::right << std::fixed << std::setprecision(1)
                 << std::setw(8) << algo.arithmetic_intensity
                 << std::setw(12) << bound_type
                 << std::setw(22) << best.name << "\n";
        }

        ostr << "\nAI = Arithmetic Intensity (ops/byte). Higher AI = more compute-bound.\n";
        ostr << "Memory-bound algorithms benefit more from lower bandwidth types.\n";
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
