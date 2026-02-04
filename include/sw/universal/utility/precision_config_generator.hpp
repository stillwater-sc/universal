#pragma once
// precision_config_generator.hpp: generate precision configuration for mixed-precision algorithms
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// The precision_config_generator produces configuration code and type aliases
// for mixed-precision algorithm implementations based on Pareto analysis.
//
// Usage:
//   #include <universal/utility/precision_config_generator.hpp>
//
//   using namespace sw::universal;
//
//   PrecisionConfigGenerator gen;
//   gen.setAlgorithm("GEMM");
//   gen.setAccuracyRequirement(1e-4);
//   gen.setEnergyBudget(0.5);
//
//   std::string config = gen.generateConfigHeader();
//   std::cout << config;

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>

#include "pareto_explorer.hpp"
#include "algorithm_profiler.hpp"

namespace sw { namespace universal {

/// Configuration for a mixed-precision algorithm
struct MixedPrecisionConfig {
    std::string algorithm_name;
    std::string input_type;
    std::string compute_type;
    std::string accumulator_type;
    std::string output_type;
    double accuracy_requirement;
    double energy_factor;
    std::string rationale;

    MixedPrecisionConfig()
        : algorithm_name("unknown")
        , input_type("float"), compute_type("float")
        , accumulator_type("float"), output_type("float")
        , accuracy_requirement(1e-7), energy_factor(1.0)
        , rationale("Default FP32 configuration") {}
};

/// Generator for precision configuration code
class PrecisionConfigGenerator {
public:
    PrecisionConfigGenerator()
        : algorithm_("GEMM")
        , accuracy_(1e-7)
        , energy_budget_(1.0)
        , problem_size_("1024x1024") {}

    /// Set algorithm name
    void setAlgorithm(const std::string& name) { algorithm_ = name; }

    /// Set accuracy requirement (relative error)
    void setAccuracyRequirement(double acc) { accuracy_ = acc; }

    /// Set energy budget (relative to FP32)
    void setEnergyBudget(double budget) { energy_budget_ = budget; }

    /// Set problem size description
    void setProblemSize(const std::string& size) { problem_size_ = size; }

    /// Generate configuration based on requirements
    MixedPrecisionConfig generateConfig() const {
        MixedPrecisionConfig config;
        config.algorithm_name = algorithm_;
        config.accuracy_requirement = accuracy_;

        ParetoExplorer explorer;
        auto result = explorer.computeFrontier();

        // Find best type for each role
        // Output: must meet accuracy
        auto output_cfg = result.bestForAccuracy(accuracy_);
        config.output_type = mapToUniversalType(output_cfg.name);

        // Accumulator: needs higher precision for stability
        auto acc_cfg = result.bestForAccuracy(accuracy_ * 1e-3);
        config.accumulator_type = mapToUniversalType(acc_cfg.name);

        // Compute: balance accuracy and energy
        if (energy_budget_ < 0.5) {
            auto comp_cfg = result.bestForEnergy(energy_budget_);
            config.compute_type = mapToUniversalType(comp_cfg.name);
        } else {
            config.compute_type = config.output_type;
        }

        // Input: can often be lower precision
        auto input_cfg = result.bestForEnergy(energy_budget_ * 0.7);
        if (input_cfg.relative_accuracy > accuracy_ * 100) {
            config.input_type = config.compute_type;
        } else {
            config.input_type = mapToUniversalType(input_cfg.name);
        }

        // Estimate combined energy
        config.energy_factor = estimateEnergy(config);

        // Generate rationale
        std::stringstream ss;
        ss << "Optimized for " << algorithm_ << " with ";
        ss << std::scientific << std::setprecision(0) << accuracy_ << " accuracy, ";
        ss << std::fixed << std::setprecision(1) << (energy_budget_ * 100) << "% energy budget";
        config.rationale = ss.str();

        return config;
    }

    /// Generate C++ header with type aliases
    std::string generateConfigHeader() const {
        auto config = generateConfig();
        std::stringstream ss;

        ss << "// Auto-generated mixed-precision configuration\n";
        ss << "// Algorithm: " << config.algorithm_name << "\n";
        ss << "// Generated: " << getTimestamp() << "\n";
        ss << "//\n";
        ss << "// Requirements:\n";
        ss << "//   Accuracy:     " << std::scientific << std::setprecision(1)
           << config.accuracy_requirement << "\n";
        ss << "//   Energy budget: " << std::fixed << std::setprecision(0)
           << (energy_budget_ * 100) << "% of FP32\n";
        ss << "//\n";
        ss << "// Estimated energy: " << std::setprecision(1)
           << (config.energy_factor * 100) << "% of all-FP32\n";
        ss << "//\n";
        ss << "#pragma once\n\n";

        ss << "#include <universal/number/cfloat/cfloat.hpp>\n";
        ss << "#include <universal/number/posit/posit.hpp>\n";
        ss << "#include <universal/number/lns/lns.hpp>\n";
        ss << "#include <universal/number/fixpnt/fixpnt.hpp>\n\n";

        ss << "namespace " << sanitizeName(config.algorithm_name) << "_config {\n\n";

        ss << "// Input precision - for loading data\n";
        ss << "using InputType = " << config.input_type << ";\n\n";

        ss << "// Compute precision - for arithmetic operations\n";
        ss << "using ComputeType = " << config.compute_type << ";\n\n";

        ss << "// Accumulator precision - for reductions and dot products\n";
        ss << "using AccumulatorType = " << config.accumulator_type << ";\n\n";

        ss << "// Output precision - for storing results\n";
        ss << "using OutputType = " << config.output_type << ";\n\n";

        ss << "// Configuration metadata\n";
        ss << "constexpr double target_accuracy = " << std::scientific
           << config.accuracy_requirement << ";\n";
        ss << "constexpr double estimated_energy_factor = " << std::fixed
           << std::setprecision(2) << config.energy_factor << ";\n\n";

        ss << "} // namespace " << sanitizeName(config.algorithm_name) << "_config\n";

        return ss.str();
    }

    /// Generate example usage code
    std::string generateExampleCode() const {
        auto config = generateConfig();
        std::stringstream ss;

        ss << "// Example usage of mixed-precision " << config.algorithm_name << "\n";
        ss << "//\n";
        ss << "// Include the generated configuration:\n";
        ss << "// #include \"" << sanitizeName(config.algorithm_name) << "_precision_config.hpp\"\n\n";

        ss << "#include <vector>\n\n";

        ss << "template<typename InputT, typename ComputeT, typename AccumT, typename OutputT>\n";
        ss << "void mixed_precision_" << sanitizeName(config.algorithm_name)
           << "(const std::vector<InputT>& input,\n";
        ss << "                       std::vector<OutputT>& output) {\n";
        ss << "    // Convert input to compute precision\n";
        ss << "    std::vector<ComputeT> work(input.begin(), input.end());\n";
        ss << "    \n";
        ss << "    // Perform computation with accumulator precision for reductions\n";
        ss << "    AccumT accumulator = AccumT(0);\n";
        ss << "    for (const auto& val : work) {\n";
        ss << "        accumulator += AccumT(val);\n";
        ss << "    }\n";
        ss << "    \n";
        ss << "    // Store result in output precision\n";
        ss << "    output.push_back(OutputT(accumulator));\n";
        ss << "}\n\n";

        ss << "// Usage with generated config:\n";
        ss << "// using namespace " << sanitizeName(config.algorithm_name) << "_config;\n";
        ss << "// mixed_precision_" << sanitizeName(config.algorithm_name)
           << "<InputType, ComputeType, AccumulatorType, OutputType>(data, result);\n";

        return ss.str();
    }

    /// Generate comparison report
    std::string generateComparisonReport() const {
        std::stringstream ss;

        ss << "Mixed-Precision Configuration Report\n";
        ss << std::string(60, '=') << "\n\n";

        ss << "Algorithm: " << algorithm_ << "\n";
        ss << "Problem size: " << problem_size_ << "\n";
        ss << "Accuracy requirement: " << std::scientific << accuracy_ << "\n";
        ss << "Energy budget: " << std::fixed << std::setprecision(0)
           << (energy_budget_ * 100) << "% of FP32\n\n";

        // Generate configs at different accuracy levels
        std::vector<double> accuracy_levels = {1e-2, 1e-4, 1e-7, 1e-10};

        ss << "Configurations at different accuracy levels:\n";
        ss << std::string(60, '-') << "\n";
        ss << std::left << std::setw(12) << "Accuracy"
           << std::setw(12) << "Input"
           << std::setw(12) << "Compute"
           << std::setw(12) << "Accum"
           << std::setw(12) << "Energy" << "\n";
        ss << std::string(60, '-') << "\n";

        PrecisionConfigGenerator gen = *this;
        for (double acc : accuracy_levels) {
            gen.setAccuracyRequirement(acc);
            auto config = gen.generateConfig();

            ss << std::left << std::scientific << std::setprecision(0)
               << std::setw(12) << acc
               << std::fixed << std::setw(12) << abbreviateType(config.input_type)
               << std::setw(12) << abbreviateType(config.compute_type)
               << std::setw(12) << abbreviateType(config.accumulator_type)
               << std::setprecision(2) << std::setw(11) << config.energy_factor << "x\n";
        }

        return ss.str();
    }

    /// Print full analysis
    void printAnalysis(std::ostream& ostr) const {
        ostr << generateComparisonReport() << "\n";
        ostr << "Generated Configuration Header:\n";
        ostr << std::string(60, '-') << "\n";
        ostr << generateConfigHeader() << "\n";
        ostr << "Example Usage Code:\n";
        ostr << std::string(60, '-') << "\n";
        ostr << generateExampleCode();
    }

private:
    std::string algorithm_;
    double accuracy_;
    double energy_budget_;
    std::string problem_size_;

    /// Map friendly name to Universal type
    std::string mapToUniversalType(const std::string& name) const {
        static std::map<std::string, std::string> type_map = {
            {"FP64 (double)", "double"},
            {"FP32 (float)", "float"},
            {"FP16 (half)", "sw::universal::half"},
            {"BF16", "sw::universal::bfloat16"},
            {"posit<64,3>", "sw::universal::posit<64,3>"},
            {"posit<32,2>", "sw::universal::posit<32,2>"},
            {"posit<16,1>", "sw::universal::posit<16,1>"},
            {"posit<8,0>", "sw::universal::posit<8,0>"},
            {"INT8", "int8_t"},
            {"INT16", "int16_t"},
            {"lns<16,8>", "sw::universal::lns<16,8>"},
            {"lns<32,16>", "sw::universal::lns<32,16>"}
        };

        auto it = type_map.find(name);
        if (it != type_map.end()) {
            return it->second;
        }
        return "float";  // Default
    }

    /// Abbreviate type name for display
    std::string abbreviateType(const std::string& type) const {
        if (type == "double") return "FP64";
        if (type == "float") return "FP32";
        if (type == "sw::universal::half") return "FP16";
        if (type == "sw::universal::bfloat16") return "BF16";
        if (type.find("posit<64") != std::string::npos) return "P64";
        if (type.find("posit<32") != std::string::npos) return "P32";
        if (type.find("posit<16") != std::string::npos) return "P16";
        if (type.find("posit<8") != std::string::npos) return "P8";
        if (type == "int8_t") return "I8";
        if (type == "int16_t") return "I16";
        if (type.find("lns<16") != std::string::npos) return "LNS16";
        if (type.find("lns<32") != std::string::npos) return "LNS32";
        return type;
    }

    /// Estimate combined energy factor
    double estimateEnergy(const MixedPrecisionConfig& config) const {
        // Simplified model: weighted average
        double input_e = getEnergyFactor(config.input_type);
        double compute_e = getEnergyFactor(config.compute_type);
        double accum_e = getEnergyFactor(config.accumulator_type);
        double output_e = getEnergyFactor(config.output_type);

        // Weights: compute dominates, then memory (input/output)
        return 0.1 * input_e + 0.5 * compute_e + 0.3 * accum_e + 0.1 * output_e;
    }

    double getEnergyFactor(const std::string& type) const {
        if (type == "double") return 3.53;
        if (type == "float") return 1.0;
        if (type.find("half") != std::string::npos) return 0.31;
        if (type.find("bfloat") != std::string::npos) return 0.31;
        if (type.find("posit<64") != std::string::npos) return 1.73;
        if (type.find("posit<32") != std::string::npos) return 0.5;
        if (type.find("posit<16") != std::string::npos) return 0.15;
        if (type.find("posit<8") != std::string::npos) return 0.07;
        if (type == "int8_t") return 0.13;
        if (type == "int16_t") return 0.15;
        return 1.0;
    }

    std::string sanitizeName(const std::string& name) const {
        std::string result;
        for (char c : name) {
            if (std::isalnum(c)) {
                result += std::tolower(c);
            } else if (c == ' ' || c == '-') {
                result += '_';
            }
        }
        return result;
    }

    std::string getTimestamp() const {
        std::time_t now = std::time(nullptr);
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return buf;
    }
};

}} // namespace sw::universal
