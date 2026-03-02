#pragma once
// range_analyzer.hpp: utility to track value ranges during computation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// The range_analyzer tracks observed value ranges during computation to help
// determine appropriate precision for mixed-precision algorithm design.
//
// Usage:
//   #include <universal/utility/range_analyzer.hpp>
//
//   using namespace sw::universal;
//
//   range_analyzer<double> analyzer;
//   for (auto& result : computation_results) {
//       analyzer.observe(result);
//   }
//   analyzer.report(std::cout);
//
//   // Get recommendations
//   auto recommendation = analyzer.recommendPrecision();

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <limits>
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace sw { namespace universal {

/// Statistics collected during range analysis
struct RangeStatistics {
    uint64_t observations;      // Total values observed
    uint64_t zeros;             // Count of exact zeros
    uint64_t denormals;         // Count of subnormal values
    uint64_t normals;           // Count of normal values
    uint64_t infinities;        // Count of +/- infinity
    uint64_t nans;              // Count of NaN values
    uint64_t positive;          // Count of positive values
    uint64_t negative;          // Count of negative values
    uint64_t overflows;         // Values that would overflow a target type
    uint64_t underflows;        // Values that would underflow a target type

    RangeStatistics() : observations(0), zeros(0), denormals(0), normals(0),
                        infinities(0), nans(0), positive(0), negative(0),
                        overflows(0), underflows(0) {}

    void reset() {
        observations = zeros = denormals = normals = 0;
        infinities = nans = positive = negative = 0;
        overflows = underflows = 0;
    }
};

/// Precision recommendation based on range analysis
struct PrecisionRecommendation {
    int min_exponent_bits;      // Minimum exponent bits needed
    int min_fraction_bits;      // Minimum fraction bits for observed precision
    int recommended_bits;       // Total recommended bit-width
    std::string type_suggestion; // Suggested Universal type
    bool needs_subnormals;      // Whether subnormal support is needed
    double utilization;         // Fraction of dynamic range utilized (0-1)

    PrecisionRecommendation() : min_exponent_bits(0), min_fraction_bits(0),
                                 recommended_bits(0), type_suggestion("unknown"),
                                 needs_subnormals(false), utilization(0.0) {}
};

/// Range analyzer for tracking value distributions during computation
template<typename NumberSystem>
class range_analyzer {
public:
    using value_type = NumberSystem;

    range_analyzer() { reset(); }

    /// Reset all statistics
    void reset() {
        stats_.reset();
        min_value_ = std::numeric_limits<NumberSystem>::max();
        max_value_ = std::numeric_limits<NumberSystem>::lowest();
        min_abs_value_ = std::numeric_limits<NumberSystem>::max();
        max_abs_value_ = NumberSystem(0);
        min_scale_ = std::numeric_limits<int>::max();
        max_scale_ = std::numeric_limits<int>::min();
    }

    /// Observe a single value
    void observe(const NumberSystem& value) {
        ++stats_.observations;

        // Check for special values
        if (std::isnan(static_cast<double>(value))) {
            ++stats_.nans;
            return;  // Don't update min/max for NaN
        }

        if (std::isinf(static_cast<double>(value))) {
            ++stats_.infinities;
            if (static_cast<double>(value) > 0) ++stats_.positive;
            else ++stats_.negative;
            return;  // Don't update min/max for infinity
        }

        double dval = static_cast<double>(value);

        // Track sign
        if (dval > 0) ++stats_.positive;
        else if (dval < 0) ++stats_.negative;
        else ++stats_.zeros;

        // Track value ranges
        if (value < min_value_) min_value_ = value;
        if (value > max_value_) max_value_ = value;

        // Track absolute value range (excluding zero)
        if (dval != 0.0) {
            NumberSystem abs_val = (dval > 0) ? value : -value;
            if (abs_val < min_abs_value_) min_abs_value_ = abs_val;
            if (abs_val > max_abs_value_) max_abs_value_ = abs_val;

            // Track scale (exponent)
            int scale = extractScale(dval);
            if (scale < min_scale_) min_scale_ = scale;
            if (scale > max_scale_) max_scale_ = scale;

            // Check for denormals
            double abs_dval = std::abs(dval);
            if (abs_dval > 0 && abs_dval < static_cast<double>(std::numeric_limits<NumberSystem>::min())) {
                ++stats_.denormals;
            } else {
                ++stats_.normals;
            }
        }
    }

    /// Observe multiple values
    template<typename Iterator>
    void observe(Iterator begin, Iterator end) {
        for (auto it = begin; it != end; ++it) {
            observe(*it);
        }
    }

    /// Check if a value would overflow/underflow a target type
    template<typename TargetType>
    void checkBounds(const NumberSystem& value) {
        double dval = static_cast<double>(value);
        if (std::isnan(dval) || std::isinf(dval)) return;

        double abs_val = std::abs(dval);
        if (abs_val > static_cast<double>(std::numeric_limits<TargetType>::max())) {
            ++stats_.overflows;
        }
        if (abs_val > 0 && abs_val < static_cast<double>(std::numeric_limits<TargetType>::min())) {
            ++stats_.underflows;
        }
    }

    /// Get collected statistics
    const RangeStatistics& statistics() const { return stats_; }

    /// Get observed value range
    NumberSystem minValue() const { return min_value_; }
    NumberSystem maxValue() const { return max_value_; }
    NumberSystem minAbsValue() const { return min_abs_value_; }
    NumberSystem maxAbsValue() const { return max_abs_value_; }

    /// Get observed scale (exponent) range
    int minScale() const { return min_scale_; }
    int maxScale() const { return max_scale_; }
    /// Convenience alias for POP integration: ufp of the largest observed value
    int ufp() const { return maxScale(); }
    int scaleRange() const {
        if (min_scale_ == std::numeric_limits<int>::max()) return 0;
        return max_scale_ - min_scale_ + 1;
    }

    /// Calculate dynamic range utilization
    double dynamicRangeUtilization() const {
        if (stats_.observations == 0 || stats_.observations == stats_.zeros + stats_.nans) {
            return 0.0;
        }

        // Compare observed scale range to type's full scale range
        int type_min_exp = std::numeric_limits<NumberSystem>::min_exponent;
        int type_max_exp = std::numeric_limits<NumberSystem>::max_exponent;
        int type_range = type_max_exp - type_min_exp;

        if (type_range <= 0) return 1.0;

        int observed_range = scaleRange();
        return static_cast<double>(observed_range) / static_cast<double>(type_range);
    }

    /// Generate precision recommendation
    PrecisionRecommendation recommendPrecision() const {
        PrecisionRecommendation rec;

        if (stats_.observations == 0) {
            rec.type_suggestion = "No data observed";
            return rec;
        }

        // Calculate required exponent bits
        int scale_span = scaleRange();
        if (scale_span > 0) {
            // Need enough bits to represent the scale range
            // exponent bits = ceil(log2(scale_range)) + 1 for sign
            rec.min_exponent_bits = static_cast<int>(std::ceil(std::log2(scale_span + 1))) + 1;
            rec.min_exponent_bits = std::max(rec.min_exponent_bits, 2);  // Minimum 2 bits
        } else {
            rec.min_exponent_bits = 2;
        }

        // Estimate fraction bits needed (heuristic based on observed precision)
        // For now, use a simple heuristic: larger scale range needs fewer fraction bits
        if (scale_span <= 4) {
            rec.min_fraction_bits = 23;  // FP32-like precision
        } else if (scale_span <= 16) {
            rec.min_fraction_bits = 10;  // FP16-like precision
        } else if (scale_span <= 64) {
            rec.min_fraction_bits = 7;   // bfloat16-like precision
        } else {
            rec.min_fraction_bits = 3;   // Minimal precision
        }

        rec.needs_subnormals = (stats_.denormals > 0);
        rec.utilization = dynamicRangeUtilization();

        // Calculate total bits: 1 sign + exponent + fraction
        rec.recommended_bits = 1 + rec.min_exponent_bits + rec.min_fraction_bits;

        // Round to standard sizes and suggest type
        if (rec.recommended_bits <= 8) {
            rec.recommended_bits = 8;
            rec.type_suggestion = "cfloat<8,2> or posit<8,0>";
        } else if (rec.recommended_bits <= 16) {
            rec.recommended_bits = 16;
            if (rec.min_exponent_bits <= 5) {
                rec.type_suggestion = "half (cfloat<16,5>) or posit<16,1>";
            } else {
                rec.type_suggestion = "bfloat16 (cfloat<16,8>) or posit<16,2>";
            }
        } else if (rec.recommended_bits <= 32) {
            rec.recommended_bits = 32;
            rec.type_suggestion = "float (cfloat<32,8>) or posit<32,2>";
        } else {
            rec.recommended_bits = 64;
            rec.type_suggestion = "double (cfloat<64,11>) or posit<64,3>";
        }

        return rec;
    }

    /// Report range analysis to stream
    void report(std::ostream& ostr) const {
        ostr << "Range Analysis Report\n";
        ostr << std::string(50, '=') << "\n\n";

        ostr << "Observations: " << stats_.observations << "\n\n";

        ostr << "Value Classification:\n";
        ostr << "  Zeros:      " << stats_.zeros << "\n";
        ostr << "  Normals:    " << stats_.normals << "\n";
        ostr << "  Denormals:  " << stats_.denormals << "\n";
        ostr << "  Infinities: " << stats_.infinities << "\n";
        ostr << "  NaNs:       " << stats_.nans << "\n";
        ostr << "  Positive:   " << stats_.positive << "\n";
        ostr << "  Negative:   " << stats_.negative << "\n\n";

        if (stats_.observations > stats_.zeros + stats_.nans + stats_.infinities) {
            ostr << std::scientific << std::setprecision(6);
            ostr << "Value Range:\n";
            ostr << "  Min value:     " << static_cast<double>(min_value_) << "\n";
            ostr << "  Max value:     " << static_cast<double>(max_value_) << "\n";
            ostr << "  Min |value|:   " << static_cast<double>(min_abs_value_) << "\n";
            ostr << "  Max |value|:   " << static_cast<double>(max_abs_value_) << "\n\n";

            ostr << std::fixed << std::setprecision(2);
            ostr << "Scale (Exponent) Range:\n";
            ostr << "  Min scale:     " << min_scale_ << "\n";
            ostr << "  Max scale:     " << max_scale_ << "\n";
            ostr << "  Scale span:    " << scaleRange() << " decades\n";
            ostr << "  DR utilization: " << (dynamicRangeUtilization() * 100) << "%\n\n";
        }

        if (stats_.overflows > 0 || stats_.underflows > 0) {
            ostr << "Boundary Violations:\n";
            ostr << "  Overflows:  " << stats_.overflows << "\n";
            ostr << "  Underflows: " << stats_.underflows << "\n\n";
        }

        auto rec = recommendPrecision();
        ostr << "Precision Recommendation:\n";
        ostr << "  Min exponent bits: " << rec.min_exponent_bits << "\n";
        ostr << "  Min fraction bits: " << rec.min_fraction_bits << "\n";
        ostr << "  Recommended bits:  " << rec.recommended_bits << "\n";
        ostr << "  Needs subnormals:  " << (rec.needs_subnormals ? "yes" : "no") << "\n";
        ostr << "  Suggested type:    " << rec.type_suggestion << "\n";
    }

    /// Get a one-line summary
    std::string summary() const {
        std::stringstream ss;
        ss << stats_.observations << " obs, ";
        ss << "scale [" << min_scale_ << "," << max_scale_ << "], ";
        ss << std::scientific << std::setprecision(2);
        ss << "range [" << static_cast<double>(min_value_) << ","
           << static_cast<double>(max_value_) << "]";
        return ss.str();
    }

private:
    RangeStatistics stats_;
    NumberSystem min_value_;
    NumberSystem max_value_;
    NumberSystem min_abs_value_;
    NumberSystem max_abs_value_;
    int min_scale_;
    int max_scale_;

    /// Extract scale (exponent) from a double value
    static int extractScale(double value) {
        if (value == 0.0) return 0;
        int exp;
        std::frexp(std::abs(value), &exp);
        return exp - 1;  // frexp returns mantissa in [0.5, 1), so adjust
    }
};

/// Convenience function: analyze a container of values
template<typename Container>
range_analyzer<typename Container::value_type> analyzeRange(const Container& values) {
    range_analyzer<typename Container::value_type> analyzer;
    analyzer.observe(values.begin(), values.end());
    return analyzer;
}

/// Convenience function: compare ranges between source and target types
template<typename SourceType, typename TargetType>
void compareRanges(const range_analyzer<SourceType>& analyzer, std::ostream& ostr) {
    ostr << "Range Compatibility Analysis\n";
    ostr << std::string(40, '-') << "\n";

    double src_min = static_cast<double>(analyzer.minAbsValue());
    double src_max = static_cast<double>(analyzer.maxAbsValue());
    double tgt_min = static_cast<double>(std::numeric_limits<TargetType>::min());
    double tgt_max = static_cast<double>(std::numeric_limits<TargetType>::max());

    ostr << std::scientific << std::setprecision(3);
    ostr << "Source range:  [" << src_min << ", " << src_max << "]\n";
    ostr << "Target range:  [" << tgt_min << ", " << tgt_max << "]\n";

    bool fits = (src_min >= tgt_min) && (src_max <= tgt_max);
    ostr << "Fits in target: " << (fits ? "YES" : "NO") << "\n";

    if (!fits) {
        if (src_max > tgt_max) {
            ostr << "  WARNING: Values exceed target maximum (overflow risk)\n";
        }
        if (src_min < tgt_min && src_min > 0) {
            ostr << "  WARNING: Values below target minimum (underflow risk)\n";
        }
    }
}

}} // namespace sw::universal
