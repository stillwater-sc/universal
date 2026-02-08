#pragma once
// type_advisor.hpp: recommend optimal number types for mixed-precision algorithms
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// The type_advisor recommends appropriate Universal number types based on:
//   - Observed value ranges (from range_analyzer)
//   - Required accuracy (relative error tolerance)
//   - Energy budget constraints
//   - Special value handling requirements
//
// Usage:
//   #include <universal/utility/type_advisor.hpp>
//
//   using namespace sw::universal;
//
//   // Analyze your data first
//   range_analyzer<double> analyzer;
//   for (auto v : data) analyzer.observe(v);
//
//   // Get type recommendations
//   TypeAdvisor advisor;
//   auto recommendations = advisor.recommend(analyzer, 1e-3);  // 0.1% accuracy
//
//   for (const auto& rec : recommendations) {
//       std::cout << rec.type_name << ": " << rec.rationale << "\n";
//   }

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>

#include "range_analyzer.hpp"

namespace sw { namespace universal {

/// Characteristics of a number system type
struct TypeCharacteristics {
    std::string name;           // Type name (e.g., "posit<32,2>")
    std::string family;         // Type family (posit, cfloat, fixpnt, lns)
    int total_bits;             // Total bit width
    int exponent_bits;          // Exponent/regime bits
    int fraction_bits;          // Fraction/mantissa bits
    double max_value;           // Maximum representable value
    double min_positive;        // Minimum positive value
    double epsilon;             // Machine epsilon (relative precision)
    double energy_per_fma;      // Energy per FMA in picojoules (estimate)
    bool has_subnormals;        // Supports gradual underflow
    bool has_inf;               // Has infinity representation
    bool has_nan;               // Has NaN representation
};

/// A single type recommendation with rationale
struct TypeRecommendation {
    TypeCharacteristics type;
    double suitability_score;   // 0-100, higher is better
    std::string rationale;      // Why this type is recommended
    bool meets_accuracy;        // Meets accuracy requirement
    bool meets_range;           // Covers required dynamic range
    double estimated_energy;    // Relative energy (1.0 = FP32 baseline)
};

/// Accuracy requirement specification
struct AccuracyRequirement {
    double relative_error;      // Maximum acceptable relative error
    double absolute_error;      // Maximum acceptable absolute error (0 = ignore)
    bool require_exact_zero;    // Must represent zero exactly
    bool require_inf;           // Must handle infinity
    bool require_nan;           // Must handle NaN

    AccuracyRequirement(double rel_err = 1e-3)
        : relative_error(rel_err), absolute_error(0.0),
          require_exact_zero(true), require_inf(false), require_nan(false) {}
};

/// Type advisor for mixed-precision algorithm design
class TypeAdvisor {
public:
    TypeAdvisor() {
        initializeTypeDatabase();
    }

    /// Get all known type characteristics
    const std::vector<TypeCharacteristics>& knownTypes() const {
        return types_;
    }

    /// Recommend types based on range analysis and accuracy requirements
    template<typename NumberSystem>
    std::vector<TypeRecommendation> recommend(
            const range_analyzer<NumberSystem>& analyzer,
            const AccuracyRequirement& accuracy = AccuracyRequirement()) const {

        std::vector<TypeRecommendation> recommendations;
        const auto& stats = analyzer.statistics();

        // Extract requirements from analyzer
        double required_max = std::abs(static_cast<double>(analyzer.maxValue()));
        double required_min_abs = static_cast<double>(analyzer.minAbsValue());
        int scale_span = analyzer.scaleRange();
        bool needs_subnormals = (stats.denormals > 0);
        bool has_special = (stats.infinities > 0 || stats.nans > 0);
        (void)required_max;  // Used in evaluateRange via type loop
        (void)required_min_abs;
        (void)scale_span;

        // Evaluate each type
        for (const auto& type : types_) {
            TypeRecommendation rec;
            rec.type = type;
            rec.meets_range = evaluateRange(type, required_max, required_min_abs, scale_span);
            rec.meets_accuracy = evaluateAccuracy(type, accuracy);
            rec.estimated_energy = type.energy_per_fma / 1.5;  // Normalize to FP32

            // Calculate suitability score
            rec.suitability_score = calculateScore(type, analyzer, accuracy,
                                                    needs_subnormals, has_special);

            // Generate rationale
            rec.rationale = generateRationale(type, rec, accuracy, scale_span,
                                               needs_subnormals, has_special);

            recommendations.push_back(rec);
        }

        // Sort by suitability score (highest first)
        std::sort(recommendations.begin(), recommendations.end(),
                  [](const TypeRecommendation& a, const TypeRecommendation& b) {
                      return a.suitability_score > b.suitability_score;
                  });

        return recommendations;
    }

    /// Recommend types with simple relative error requirement
    template<typename NumberSystem>
    std::vector<TypeRecommendation> recommend(
            const range_analyzer<NumberSystem>& analyzer,
            double relative_error) const {
        AccuracyRequirement acc(relative_error);
        return recommend(analyzer, acc);
    }

    /// Get the top recommendation
    template<typename NumberSystem>
    TypeRecommendation bestType(
            const range_analyzer<NumberSystem>& analyzer,
            const AccuracyRequirement& accuracy = AccuracyRequirement()) const {
        auto recs = recommend(analyzer, accuracy);
        if (recs.empty()) {
            TypeRecommendation empty;
            empty.type.name = "No suitable type found";
            empty.suitability_score = 0;
            return empty;
        }
        return recs[0];
    }

    /// Print recommendations report
    template<typename NumberSystem>
    void report(std::ostream& ostr,
                const range_analyzer<NumberSystem>& analyzer,
                const AccuracyRequirement& accuracy = AccuracyRequirement()) const {

        auto recs = recommend(analyzer, accuracy);

        ostr << "Type Advisor Recommendations\n";
        ostr << std::string(60, '=') << "\n\n";

        ostr << "Requirements:\n";
        ostr << std::scientific << std::setprecision(1);
        ostr << "  Relative error: <" << accuracy.relative_error << "\n";
        ostr << "  Scale span:     " << analyzer.scaleRange() << " decades\n";
        ostr << "  Subnormals:     " << (analyzer.statistics().denormals > 0 ? "needed" : "not needed") << "\n\n";

        ostr << std::fixed << std::setprecision(1);
        ostr << std::left << std::setw(20) << "Type"
             << std::right << std::setw(8) << "Score"
             << std::setw(8) << "Range"
             << std::setw(8) << "Acc"
             << std::setw(10) << "Energy"
             << "  Rationale\n";
        ostr << std::string(80, '-') << "\n";

        int shown = 0;
        for (const auto& rec : recs) {
            if (shown++ >= 10) break;  // Show top 10

            ostr << std::left << std::setw(20) << rec.type.name
                 << std::right << std::setw(7) << rec.suitability_score << "%"
                 << std::setw(8) << (rec.meets_range ? "OK" : "NO")
                 << std::setw(8) << (rec.meets_accuracy ? "OK" : "NO")
                 << std::setw(9) << rec.estimated_energy << "x"
                 << "  " << rec.rationale << "\n";
        }

        ostr << "\nBest recommendation: " << recs[0].type.name << "\n";
    }

private:
    std::vector<TypeCharacteristics> types_;

    void initializeTypeDatabase() {
        // IEEE-754 types
        types_.push_back({"half (cfloat<16,5>)", "cfloat", 16, 5, 10,
                          65504.0, 6.1e-5, 9.77e-4, 0.47, true, true, true});
        types_.push_back({"bfloat16 (cfloat<16,8>)", "cfloat", 16, 8, 7,
                          3.4e38, 1.2e-38, 7.81e-3, 0.47, true, true, true});
        types_.push_back({"float (cfloat<32,8>)", "cfloat", 32, 8, 23,
                          3.4e38, 1.2e-38, 1.19e-7, 1.5, true, true, true});
        types_.push_back({"double (cfloat<64,11>)", "cfloat", 64, 11, 52,
                          std::numeric_limits<double>::max(),
                          std::numeric_limits<double>::min(),
                          std::numeric_limits<double>::epsilon(), 5.3, true, true, true});

        // Posit types (no inf/nan, but excellent dynamic range)
        types_.push_back({"posit<8,0>", "posit", 8, 2, 5,
                          64.0, 0.015625, 0.125, 0.1, false, false, false});
        types_.push_back({"posit<16,1>", "posit", 16, 3, 12,
                          2.68e8, 3.7e-9, 2.44e-4, 0.22, false, false, false});
        types_.push_back({"posit<32,2>", "posit", 32, 4, 27,
                          7.2e34, 1.4e-35, 7.45e-9, 0.75, false, false, false});
        types_.push_back({"posit<64,3>", "posit", 64, 5, 58,
                          1e72, 1e-73, 3.47e-18, 2.6, false, false, false});

        // Fixed-point (very narrow range, high precision within range)
        types_.push_back({"fixpnt<16,8>", "fixpnt", 16, 0, 8,
                          127.99, 0.00390625, 0.00390625, 0.15, false, false, false});
        types_.push_back({"fixpnt<32,16>", "fixpnt", 32, 0, 16,
                          32767.99, 1.53e-5, 1.53e-5, 0.5, false, false, false});

        // LNS (logarithmic - excellent for multiply-heavy workloads)
        types_.push_back({"lns<16,8>", "lns", 16, 8, 7,
                          3.4e38, 1.2e-38, 7.81e-3, 0.3, false, true, false});
        // lns<32,16> has range beyond double, use double max as proxy
        types_.push_back({"lns<32,16>", "lns", 32, 16, 15,
                          std::numeric_limits<double>::max(),
                          std::numeric_limits<double>::min(),
                          3.05e-5, 1.0, false, true, false});
    }

    bool evaluateRange(const TypeCharacteristics& type,
                       double required_max, double required_min_abs,
                       int scale_span) const {
        if (required_max > type.max_value) return false;
        if (required_min_abs > 0 && required_min_abs < type.min_positive) {
            // Need subnormals or the value will underflow
            if (!type.has_subnormals) return false;
        }
        return true;
    }

    bool evaluateAccuracy(const TypeCharacteristics& type,
                          const AccuracyRequirement& accuracy) const {
        // Epsilon represents the relative precision
        return type.epsilon <= accuracy.relative_error;
    }

    template<typename NumberSystem>
    double calculateScore(const TypeCharacteristics& type,
                          const range_analyzer<NumberSystem>& analyzer,
                          const AccuracyRequirement& accuracy,
                          bool needs_subnormals, bool has_special) const {

        double score = 50.0;  // Base score

        // Range coverage (0-25 points)
        double max_val = std::abs(static_cast<double>(analyzer.maxValue()));
        double min_abs = static_cast<double>(analyzer.minAbsValue());
        if (max_val <= type.max_value && min_abs >= type.min_positive) {
            score += 25.0;
        } else if (max_val <= type.max_value) {
            score += 15.0;  // Partial credit
        }

        // Accuracy (0-25 points)
        if (type.epsilon <= accuracy.relative_error) {
            score += 25.0;
        } else if (type.epsilon <= accuracy.relative_error * 10) {
            score += 10.0;  // Close enough for some applications
        }

        // Energy efficiency (0-20 points)
        // Lower energy = higher score
        double energy_ratio = type.energy_per_fma / 1.5;  // Relative to FP32
        if (energy_ratio < 0.2) score += 20.0;
        else if (energy_ratio < 0.5) score += 15.0;
        else if (energy_ratio < 1.0) score += 10.0;
        else if (energy_ratio < 2.0) score += 5.0;

        // Bit width efficiency (0-10 points)
        // Smaller = better (assuming requirements are met)
        if (type.total_bits <= 8) score += 10.0;
        else if (type.total_bits <= 16) score += 7.0;
        else if (type.total_bits <= 32) score += 4.0;

        // Penalties
        if (needs_subnormals && !type.has_subnormals) score -= 20.0;
        if (has_special && accuracy.require_inf && !type.has_inf) score -= 15.0;
        if (has_special && accuracy.require_nan && !type.has_nan) score -= 15.0;

        return std::max(0.0, std::min(100.0, score));
    }

    std::string generateRationale(const TypeCharacteristics& type,
                                   const TypeRecommendation& rec,
                                   const AccuracyRequirement& accuracy,
                                   int scale_span,
                                   bool needs_subnormals, bool has_special) const {
        std::stringstream ss;

        if (!rec.meets_range) {
            ss << "Range insufficient";
        } else if (!rec.meets_accuracy) {
            ss << "Accuracy insufficient (eps=" << type.epsilon << ")";
        } else if (type.family == "posit" && !has_special) {
            ss << "Excellent for numerical algorithms";
        } else if (type.family == "cfloat" && type.total_bits == 16) {
            ss << "Good balance of range/precision";
        } else if (type.family == "fixpnt" && scale_span <= 4) {
            ss << "Ideal for narrow-range data";
        } else if (type.family == "lns") {
            ss << "Efficient for multiply-heavy code";
        } else {
            ss << "Meets requirements";
        }

        return ss.str();
    }
};

}} // namespace sw::universal
