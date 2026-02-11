// error_tracker_sketch.hpp - Design sketch for review
// NOT production code - for design discussion
//
// This sketch explores how to create a unified error propagation tracker
// that works across multiple number systems with different error characteristics:
//
// 1. cfloat/IEEE floats: Support two_sum/two_prod for exact error computation
// 2. posit: Tapered precision, no clean error separation, use shadow computation
// 3. lns: Multiplication is EXACT (log domain), only addition introduces error
// 4. areal: Interval type with uncertainty bit - inherently tracks uncertainty
// 5. interval: Rigorous bounds via interval arithmetic
// 6. valid: Posit-based interval arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#pragma once
#include <cmath>
#include <limits>
#include <type_traits>
#include <ostream>

namespace sw { namespace universal {

// ============================================================================
// Error Tracking Strategies
// ============================================================================

enum class ErrorStrategy {
    Exact,       // two_sum/two_prod - IEEE floats only
    Shadow,      // Higher precision shadow computation
    Bounded,     // Interval arithmetic
    Statistical, // ULP-based statistical model
    Inherent     // Type natively tracks error (areal, interval, valid)
};

// ============================================================================
// Type Traits for Error Tracking Capabilities
// ============================================================================

template<typename T>
struct error_tracking_traits {
    // Does this type support exact error computation?
    static constexpr bool has_exact_errors = false;

    // Does this type support directed rounding?
    static constexpr bool has_directed_rounding = false;

    // Is multiplication exact in this representation?
    static constexpr bool exact_multiplication = false;

    // Does this type natively track uncertainty?
    static constexpr bool tracks_uncertainty = false;

    // Is this an interval type?
    static constexpr bool is_interval_type = false;

    // Recommended default strategy
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Shadow;

    // What type to use for shadow computation?
    using shadow_type = long double;
};

// IEEE float specialization
template<>
struct error_tracking_traits<float> {
    static constexpr bool has_exact_errors = true;
    static constexpr bool has_directed_rounding = true;
    static constexpr bool exact_multiplication = false;
    static constexpr bool tracks_uncertainty = false;
    static constexpr bool is_interval_type = false;
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Exact;
    using shadow_type = double;
};

template<>
struct error_tracking_traits<double> {
    static constexpr bool has_exact_errors = true;
    static constexpr bool has_directed_rounding = true;
    static constexpr bool exact_multiplication = false;
    static constexpr bool tracks_uncertainty = false;
    static constexpr bool is_interval_type = false;
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Exact;
    using shadow_type = long double;
};

// ============================================================================
// Posit Specialization
// Posits have tapered precision - no clean error separation possible
// ============================================================================

/*
template<unsigned nbits, unsigned es>
struct error_tracking_traits<posit<nbits, es>> {
    static constexpr bool has_exact_errors = false;  // No two_sum for posits
    static constexpr bool has_directed_rounding = false;
    static constexpr bool exact_multiplication = false;
    static constexpr bool tracks_uncertainty = false;
    static constexpr bool is_interval_type = false;
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Shadow;
    // Use larger posit or double for shadow
    using shadow_type = std::conditional_t<(nbits <= 32), double, long double>;
};
*/

// ============================================================================
// LNS Specialization
// KEY INSIGHT: In LNS, multiplication IS exact (it's addition in log domain)
// Only addition/subtraction introduces error in LNS
// ============================================================================

/*
template<unsigned nbits, unsigned rbits, typename bt, auto...x>
struct error_tracking_traits<lns<nbits, rbits, bt, x...>> {
    static constexpr bool has_exact_errors = false;
    static constexpr bool has_directed_rounding = false;
    static constexpr bool exact_multiplication = true;  // KEY DIFFERENCE!
    static constexpr bool tracks_uncertainty = false;
    static constexpr bool is_interval_type = false;
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Shadow;
    using shadow_type = double;
};
*/

// ============================================================================
// Areal Specialization
// Areal is a "faithful" floating-point with an uncertainty bit (ubit)
// The ubit indicates whether the value is exact (ubit=0) or represents
// the interval (value, next_encoding) when ubit=1
//
// Key properties:
// - When ubit=0: value is exactly representable
// - When ubit=1: true value lies in open interval (value, next_value)
// - The ubit propagates through operations automatically
// - This provides built-in error tracking at the type level
// ============================================================================

/*
template<unsigned nbits, unsigned es, typename bt>
struct error_tracking_traits<areal<nbits, es, bt>> {
    static constexpr bool has_exact_errors = false;
    static constexpr bool has_directed_rounding = false;
    static constexpr bool exact_multiplication = false;
    static constexpr bool tracks_uncertainty = true;   // KEY: Inherent tracking!
    static constexpr bool is_interval_type = true;     // It's an interval type
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Inherent;
    using shadow_type = double;  // Not typically needed
};
*/

// ============================================================================
// Valid Specialization (posit-based interval)
// Valid numbers use two posit bounds with open/closed indicators
// ============================================================================

/*
template<unsigned nbits, unsigned es>
struct error_tracking_traits<valid<nbits, es>> {
    static constexpr bool has_exact_errors = false;
    static constexpr bool has_directed_rounding = false;
    static constexpr bool exact_multiplication = false;
    static constexpr bool tracks_uncertainty = true;     // Has open/closed bounds
    static constexpr bool is_interval_type = true;
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Inherent;
    using shadow_type = valid<nbits, es>;  // Self-shadowing
};
*/

// ============================================================================
// Forward declaration for classical interval type (proposed for Universal)
// ============================================================================

template<typename Real> class interval;

/*
template<typename Real>
struct error_tracking_traits<interval<Real>> {
    static constexpr bool has_exact_errors = false;
    static constexpr bool has_directed_rounding = true;  // Uses directed rounding
    static constexpr bool exact_multiplication = false;
    static constexpr bool tracks_uncertainty = true;     // KEY: Inherent tracking!
    static constexpr bool is_interval_type = true;
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Inherent;
    using shadow_type = interval<Real>;  // Self-shadowing
};
*/

// ============================================================================
// Error-Free Operations (IEEE floats only)
// ============================================================================

// two_sum: compute s + e = a + b exactly
inline std::pair<double, double> two_sum(double a, double b) {
    double s = a + b;
    double a_prime = s - b;
    double b_prime = s - a_prime;
    double delta_a = a - a_prime;
    double delta_b = b - b_prime;
    double e = delta_a + delta_b;
    return {s, e};
}

// two_prod: compute p + e = a * b exactly (requires FMA)
inline std::pair<double, double> two_prod(double a, double b) {
    double p = a * b;
    double e = std::fma(a, b, -p);
    return {p, e};
}

// ============================================================================
// Tracked Value - Shadow Strategy Implementation
// ============================================================================

template<typename T>
class TrackedShadow {
public:
    using value_type = T;
    using shadow_type = typename error_tracking_traits<T>::shadow_type;

private:
    T value_;
    shadow_type shadow_;
    uint64_t op_count_;

public:
    // Constructors
    TrackedShadow() : value_(T(0)), shadow_(0), op_count_(0) {}

    TrackedShadow(T v)
        : value_(v)
        , shadow_(static_cast<shadow_type>(v))
        , op_count_(0) {}

    TrackedShadow(T v, shadow_type s, uint64_t ops = 0)
        : value_(v), shadow_(s), op_count_(ops) {}

    // Accessors
    T value() const { return value_; }
    shadow_type shadow() const { return shadow_; }
    uint64_t operations() const { return op_count_; }

    // Error metrics
    double error() const {
        return std::abs(static_cast<double>(shadow_) - static_cast<double>(value_));
    }

    double relative_error() const {
        double s = static_cast<double>(shadow_);
        if (std::abs(s) < std::numeric_limits<double>::min()) return 0.0;
        return error() / std::abs(s);
    }

    double valid_bits() const {
        double rel_err = relative_error();
        if (rel_err <= 0) return 53.0;  // Full precision
        return std::max(0.0, -std::log2(rel_err));
    }

    // Arithmetic operators
    TrackedShadow operator+(const TrackedShadow& rhs) const {
        T result = value_ + rhs.value_;
        shadow_type exact = shadow_ + rhs.shadow_;
        return TrackedShadow(result, exact, op_count_ + rhs.op_count_ + 1);
    }

    TrackedShadow operator-(const TrackedShadow& rhs) const {
        T result = value_ - rhs.value_;
        shadow_type exact = shadow_ - rhs.shadow_;
        return TrackedShadow(result, exact, op_count_ + rhs.op_count_ + 1);
    }

    TrackedShadow operator*(const TrackedShadow& rhs) const {
        T result = value_ * rhs.value_;
        shadow_type exact = shadow_ * rhs.shadow_;
        return TrackedShadow(result, exact, op_count_ + rhs.op_count_ + 1);
    }

    TrackedShadow operator/(const TrackedShadow& rhs) const {
        T result = value_ / rhs.value_;
        shadow_type exact = shadow_ / rhs.shadow_;
        return TrackedShadow(result, exact, op_count_ + rhs.op_count_ + 1);
    }

    // Compound assignment
    TrackedShadow& operator+=(const TrackedShadow& rhs) {
        value_ += rhs.value_;
        shadow_ += rhs.shadow_;
        op_count_ += rhs.op_count_ + 1;
        return *this;
    }

    TrackedShadow& operator*=(const TrackedShadow& rhs) {
        value_ *= rhs.value_;
        shadow_ *= rhs.shadow_;
        op_count_ += rhs.op_count_ + 1;
        return *this;
    }
};

// ============================================================================
// Tracked Value - Exact Strategy Implementation (IEEE floats)
// ============================================================================

template<typename T>
class TrackedExact {
    static_assert(error_tracking_traits<T>::has_exact_errors,
                  "TrackedExact requires type with exact error computation");

public:
    using value_type = T;

private:
    T value_;
    double cumulative_error_;  // Running sum of absolute errors
    uint64_t op_count_;

public:
    TrackedExact() : value_(T(0)), cumulative_error_(0), op_count_(0) {}
    TrackedExact(T v) : value_(v), cumulative_error_(0), op_count_(0) {}
    TrackedExact(T v, double e, uint64_t ops)
        : value_(v), cumulative_error_(e), op_count_(ops) {}

    T value() const { return value_; }
    double error() const { return cumulative_error_; }
    uint64_t operations() const { return op_count_; }

    double relative_error() const {
        double v = static_cast<double>(value_);
        if (std::abs(v) < std::numeric_limits<double>::min()) return 0.0;
        return cumulative_error_ / std::abs(v);
    }

    // Addition with exact error tracking
    TrackedExact operator+(const TrackedExact& rhs) const {
        auto [sum, err] = two_sum(static_cast<double>(value_),
                                   static_cast<double>(rhs.value_));
        double total_error = cumulative_error_ + rhs.cumulative_error_ + std::abs(err);
        return TrackedExact(static_cast<T>(sum), total_error,
                            op_count_ + rhs.op_count_ + 1);
    }

    // Multiplication with exact error tracking
    TrackedExact operator*(const TrackedExact& rhs) const {
        auto [prod, err] = two_prod(static_cast<double>(value_),
                                     static_cast<double>(rhs.value_));
        // Error propagation in multiplication is more complex:
        // (a + ea) * (b + eb) = ab + a*eb + b*ea + ea*eb
        double a = static_cast<double>(value_);
        double b = static_cast<double>(rhs.value_);
        double prop_error = std::abs(a) * rhs.cumulative_error_ +
                            std::abs(b) * cumulative_error_;
        double total_error = prop_error + std::abs(err);
        return TrackedExact(static_cast<T>(prod), total_error,
                            op_count_ + rhs.op_count_ + 1);
    }
};

// ============================================================================
// Tracked Value - Statistical Strategy (Fast, Approximate)
// ============================================================================

template<typename T>
class TrackedStatistical {
public:
    using value_type = T;

private:
    T value_;
    double error_bound_;  // Estimated error bound (not exact)
    uint64_t op_count_;

    // ULP value for current result
    static double ulp_of(T v) {
        double d = static_cast<double>(v);
        if (d == 0.0) return std::numeric_limits<T>::min();
        int exp;
        std::frexp(d, &exp);
        return std::ldexp(std::numeric_limits<T>::epsilon(), exp - 1);
    }

public:
    TrackedStatistical() : value_(T(0)), error_bound_(0), op_count_(0) {}
    TrackedStatistical(T v) : value_(v), error_bound_(0), op_count_(0) {}
    TrackedStatistical(T v, double e, uint64_t ops)
        : value_(v), error_bound_(e), op_count_(ops) {}

    T value() const { return value_; }
    double error() const { return error_bound_; }
    uint64_t operations() const { return op_count_; }

    // Addition: error accumulates
    // |err(a+b)| <= |err(a)| + |err(b)| + 0.5*ulp(a+b)
    TrackedStatistical operator+(const TrackedStatistical& rhs) const {
        T result = value_ + rhs.value_;
        double new_error = error_bound_ + rhs.error_bound_ + 0.5 * ulp_of(result);
        return TrackedStatistical(result, new_error, op_count_ + rhs.op_count_ + 1);
    }

    // Multiplication: relative error accumulates
    // rel_err(a*b) <= rel_err(a) + rel_err(b) + 0.5*ulp
    TrackedStatistical operator*(const TrackedStatistical& rhs) const {
        T result = value_ * rhs.value_;
        double abs_result = std::abs(static_cast<double>(result));
        // Error propagation: |a|*err(b) + |b|*err(a) + 0.5*ulp(result)
        double new_error = std::abs(static_cast<double>(value_)) * rhs.error_bound_ +
                           std::abs(static_cast<double>(rhs.value_)) * error_bound_ +
                           0.5 * ulp_of(result);
        return TrackedStatistical(result, new_error, op_count_ + rhs.op_count_ + 1);
    }
};

// ============================================================================
// LNS-Specific Error Model
// ============================================================================

// For LNS, we need special handling because:
// 1. Multiplication is EXACT (it's addition in log domain)
// 2. Addition has error that depends on the operand ratio

template<typename LNSType>
class TrackedLNS {
public:
    using value_type = LNSType;

private:
    LNSType value_;
    double shadow_;       // Double-precision shadow
    double add_errors_;   // Cumulative addition errors only
    uint64_t adds_;
    uint64_t mults_;      // Tracked separately since mult is exact

public:
    TrackedLNS() : value_(LNSType(0)), shadow_(0), add_errors_(0), adds_(0), mults_(0) {}
    TrackedLNS(LNSType v) : value_(v), shadow_(double(v)), add_errors_(0), adds_(0), mults_(0) {}

    LNSType value() const { return value_; }
    double error() const { return std::abs(shadow_ - double(value_)); }

    // LNS multiplication is EXACT - no error introduced
    TrackedLNS operator*(const TrackedLNS& rhs) const {
        LNSType result = value_ * rhs.value_;
        double exact = shadow_ * rhs.shadow_;
        // Multiplication adds NO new error in LNS
        return TrackedLNS(result, exact, add_errors_ + rhs.add_errors_,
                          adds_ + rhs.adds_, mults_ + rhs.mults_ + 1);
    }

    // LNS addition introduces error - the only source of error
    TrackedLNS operator+(const TrackedLNS& rhs) const {
        LNSType result = value_ + rhs.value_;
        double exact = shadow_ + rhs.shadow_;

        // Error in LNS addition depends on the ratio of operands
        // When a ≈ -b, we have catastrophic cancellation
        double ratio = shadow_ / rhs.shadow_;
        double cancellation_factor = 1.0;
        if (ratio < 0 && std::abs(1.0 + ratio) < 0.1) {
            // Near-cancellation: error is amplified
            cancellation_factor = 1.0 / std::abs(1.0 + ratio);
        }

        double this_op_error = std::abs(exact - double(result));
        double total_add_error = add_errors_ + rhs.add_errors_ + this_op_error;

        return TrackedLNS(result, exact, total_add_error,
                          adds_ + rhs.adds_ + 1, mults_ + rhs.mults_);
    }

    // Report LNS-specific metrics
    void report(std::ostream& os) const {
        os << "LNS Tracked Value:\n";
        os << "  Value: " << double(value_) << "\n";
        os << "  Shadow: " << shadow_ << "\n";
        os << "  Total error: " << error() << "\n";
        os << "  Additions: " << adds_ << " (error source)\n";
        os << "  Multiplications: " << mults_ << " (exact)\n";
    }

private:
    TrackedLNS(LNSType v, double s, double e, uint64_t a, uint64_t m)
        : value_(v), shadow_(s), add_errors_(e), adds_(a), mults_(m) {}
};

// ============================================================================
// Areal-Specific Wrapper
// Areal natively tracks uncertainty via its uncertainty bit (ubit)
// This wrapper provides a unified interface for error tracking
//
// The areal type encoding:
//   [sign | exponent | fraction | ubit]
//
// When ubit=0: The value is exact
// When ubit=1: The true value lies in the open interval (v, next(v))
// ============================================================================

template<typename ArealType>
class TrackedAreal {
public:
    using value_type = ArealType;

private:
    ArealType value_;
    uint64_t op_count_;

public:
    TrackedAreal() : value_(), op_count_(0) {}
    TrackedAreal(ArealType v) : value_(v), op_count_(0) {}

    ArealType value() const { return value_; }
    uint64_t operations() const { return op_count_; }

    // Areal uncertainty check - the type itself knows if it's exact
    bool is_exact() const {
        // ubit is at bit 0 in areal encoding
        return !value_.at(0);  // ubit=0 means exact
    }

    // Error bounds - if ubit=1, value is in (v, next(v))
    // Returns 0 if exact, otherwise the interval width
    double error_bound() const {
        if (is_exact()) return 0.0;
        // When ubit=1, the true value is in (value, value+ulp)
        ArealType next_val = value_;
        ++next_val;  // Move to next encoding
        return std::abs(double(next_val) - double(value_));
    }

    // For API compatibility with other tracked types
    double error() const { return error_bound(); }

    double relative_error() const {
        double v = double(value_);
        if (std::abs(v) < std::numeric_limits<double>::min()) return 0.0;
        return error_bound() / std::abs(v);
    }

    // Arithmetic operators - areal handles uncertainty propagation internally
    TrackedAreal operator+(const TrackedAreal& rhs) const {
        ArealType result = value_ + rhs.value_;
        return TrackedAreal(result, op_count_ + rhs.op_count_ + 1);
    }

    TrackedAreal operator-(const TrackedAreal& rhs) const {
        ArealType result = value_ - rhs.value_;
        return TrackedAreal(result, op_count_ + rhs.op_count_ + 1);
    }

    TrackedAreal operator*(const TrackedAreal& rhs) const {
        ArealType result = value_ * rhs.value_;
        return TrackedAreal(result, op_count_ + rhs.op_count_ + 1);
    }

    TrackedAreal operator/(const TrackedAreal& rhs) const {
        ArealType result = value_ / rhs.value_;
        return TrackedAreal(result, op_count_ + rhs.op_count_ + 1);
    }

    void report(std::ostream& os) const {
        os << "Areal Tracked Value:\n";
        os << "  Value: " << value_ << "\n";
        os << "  Exact: " << (is_exact() ? "yes" : "no") << "\n";
        os << "  Error bound: " << error_bound() << "\n";
        os << "  Operations: " << op_count_ << "\n";
    }

private:
    TrackedAreal(ArealType v, uint64_t ops) : value_(v), op_count_(ops) {}
};

// ============================================================================
// Classical Interval Arithmetic (proposed new Universal type)
// Provides rigorous bounds via directed rounding
//
// An interval [a,b] represents all real numbers x such that a <= x <= b
// Operations are defined to guarantee containment of the true result
// ============================================================================

template<typename Real>
class interval {
public:
    using value_type = Real;

private:
    Real lo_;  // Lower bound
    Real hi_;  // Upper bound

public:
    interval() : lo_(0), hi_(0) {}
    interval(Real v) : lo_(v), hi_(v) {}
    interval(Real lo, Real hi) : lo_(lo), hi_(hi) {}

    Real lower() const { return lo_; }
    Real upper() const { return hi_; }
    Real midpoint() const { return (lo_ + hi_) / Real(2); }
    Real width() const { return hi_ - lo_; }
    Real radius() const { return width() / Real(2); }

    bool contains(Real v) const { return lo_ <= v && v <= hi_; }
    bool is_singleton() const { return lo_ == hi_; }

    // Interval addition: [a,b] + [c,d] = [a+c, b+d]
    // With proper directed rounding for rigor
    interval operator+(const interval& rhs) const {
        // TODO: Use directed rounding for rigorous bounds
        // round_down(lo_ + rhs.lo_), round_up(hi_ + rhs.hi_)
        return interval(lo_ + rhs.lo_, hi_ + rhs.hi_);
    }

    // Interval subtraction: [a,b] - [c,d] = [a-d, b-c]
    interval operator-(const interval& rhs) const {
        return interval(lo_ - rhs.hi_, hi_ - rhs.lo_);
    }

    // Interval multiplication: [a,b] * [c,d]
    // Need to consider all four products and take min/max
    interval operator*(const interval& rhs) const {
        Real products[4] = {
            lo_ * rhs.lo_,
            lo_ * rhs.hi_,
            hi_ * rhs.lo_,
            hi_ * rhs.hi_
        };
        Real min_p = products[0], max_p = products[0];
        for (int i = 1; i < 4; ++i) {
            if (products[i] < min_p) min_p = products[i];
            if (products[i] > max_p) max_p = products[i];
        }
        return interval(min_p, max_p);
    }

    // Interval division (assumes rhs doesn't contain zero)
    interval operator/(const interval& rhs) const {
        // Simple case: both bounds of divisor have same sign
        if (rhs.lo_ > 0 || rhs.hi_ < 0) {
            return *this * interval(Real(1)/rhs.hi_, Real(1)/rhs.lo_);
        }
        // Division by interval containing zero - undefined/extended
        return interval(-std::numeric_limits<Real>::infinity(),
                        std::numeric_limits<Real>::infinity());
    }

    // Comparison for exact intervals (singletons)
    bool operator==(const interval& rhs) const {
        return lo_ == rhs.lo_ && hi_ == rhs.hi_;
    }

    // Output
    friend std::ostream& operator<<(std::ostream& os, const interval& iv) {
        return os << '[' << iv.lo_ << ", " << iv.hi_ << ']';
    }
};

// ============================================================================
// Tracked wrapper for interval arithmetic
// ============================================================================

template<typename Real>
class TrackedInterval {
public:
    using value_type = interval<Real>;

private:
    interval<Real> value_;
    uint64_t op_count_;

public:
    TrackedInterval() : value_(), op_count_(0) {}
    TrackedInterval(Real v) : value_(v), op_count_(0) {}
    TrackedInterval(interval<Real> v) : value_(v), op_count_(0) {}
    TrackedInterval(Real lo, Real hi) : value_(lo, hi), op_count_(0) {}

    interval<Real> value() const { return value_; }
    uint64_t operations() const { return op_count_; }

    // Error is the interval width (enclosure of all possible values)
    double error() const { return static_cast<double>(value_.width()); }

    // Relative error based on midpoint
    double relative_error() const {
        double mid = static_cast<double>(value_.midpoint());
        if (std::abs(mid) < std::numeric_limits<double>::min()) return 0.0;
        return error() / std::abs(mid);
    }

    // Is this an exact (singleton) interval?
    bool is_exact() const { return value_.is_singleton(); }

    TrackedInterval operator+(const TrackedInterval& rhs) const {
        return TrackedInterval(value_ + rhs.value_, op_count_ + rhs.op_count_ + 1);
    }

    TrackedInterval operator-(const TrackedInterval& rhs) const {
        return TrackedInterval(value_ - rhs.value_, op_count_ + rhs.op_count_ + 1);
    }

    TrackedInterval operator*(const TrackedInterval& rhs) const {
        return TrackedInterval(value_ * rhs.value_, op_count_ + rhs.op_count_ + 1);
    }

    TrackedInterval operator/(const TrackedInterval& rhs) const {
        return TrackedInterval(value_ / rhs.value_, op_count_ + rhs.op_count_ + 1);
    }

    void report(std::ostream& os) const {
        os << "Interval Tracked Value:\n";
        os << "  Interval: " << value_ << "\n";
        os << "  Midpoint: " << value_.midpoint() << "\n";
        os << "  Width: " << value_.width() << "\n";
        os << "  Exact: " << (is_exact() ? "yes" : "no") << "\n";
        os << "  Operations: " << op_count_ << "\n";
    }

private:
    TrackedInterval(interval<Real> v, uint64_t ops) : value_(v), op_count_(ops) {}
};

// ============================================================================
// Unified Interface with Automatic Strategy Selection
// ============================================================================

template<typename T,
         ErrorStrategy Strategy = error_tracking_traits<T>::default_strategy>
class Tracked;

// Specialization for Exact strategy
template<typename T>
class Tracked<T, ErrorStrategy::Exact> : public TrackedExact<T> {
    using TrackedExact<T>::TrackedExact;
};

// Specialization for Shadow strategy
template<typename T>
class Tracked<T, ErrorStrategy::Shadow> : public TrackedShadow<T> {
    using TrackedShadow<T>::TrackedShadow;
};

// Specialization for Statistical strategy
template<typename T>
class Tracked<T, ErrorStrategy::Statistical> : public TrackedStatistical<T> {
    using TrackedStatistical<T>::TrackedStatistical;
};

// Specialization for Bounded (Interval) strategy
template<typename T>
class Tracked<T, ErrorStrategy::Bounded> : public TrackedInterval<T> {
    using TrackedInterval<T>::TrackedInterval;
};

// Note: For ErrorStrategy::Inherent, use the type-specific wrappers directly:
// - TrackedAreal<areal<nbits,es,bt>> for areal types
// - TrackedInterval<Real> for interval types
// These types natively track uncertainty, so no separate wrapper logic needed

// ============================================================================
// Error Tracking Strategy Comparison Table
// ============================================================================
//
// | Type     | Strategy   | Accuracy    | Performance | Notes                    |
// |----------|------------|-------------|-------------|--------------------------|
// | float    | Exact      | Perfect     | Fast        | two_sum/two_prod         |
// | double   | Exact      | Perfect     | Fast        | two_sum/two_prod         |
// | posit    | Shadow     | High        | 2x slower   | Higher precision shadow  |
// | lns      | LNS-aware  | High        | 2x slower   | Mult exact, add tracked  |
// | areal    | Inherent   | Bounded     | Native      | Uncertainty bit          |
// | valid    | Inherent   | Rigorous    | 2x+ slower  | Posit-based intervals    |
// | interval | Inherent   | Rigorous    | 4x slower   | Classical IA             |
//
// ============================================================================

// ============================================================================
// Usage Examples (in comments)
// ============================================================================

/*
// For IEEE floats - uses exact two_sum/two_prod
Tracked<float> a = 1.0f;
Tracked<float> b = 1e-8f;
auto c = a + b;
std::cout << "Error: " << c.error() << "\n";  // Exact error

// For posits - uses shadow computation
Tracked<posit<32,2>> x = 1.0;
Tracked<posit<32,2>> y = 1e-8;
auto z = x + y;
std::cout << "Error: " << z.error() << "\n";  // Shadow-based error

// For LNS - uses LNS-specific model
// KEY: Multiplication is EXACT in LNS (it's just addition in log domain)
TrackedLNS<lns<32,8>> p = 1.0;
TrackedLNS<lns<32,8>> q = 2.0;
auto r = p * q;  // EXACT - no error introduced!
auto s = p + q;  // Error tracked - only addition introduces error in LNS
s.report(std::cout);

// For areal - uses inherent uncertainty tracking via ubit
// The areal type's uncertainty bit (ubit) automatically tracks whether
// the value is exact or represents an interval (v, next(v))
TrackedAreal<areal<32,8>> av = 1.0;
TrackedAreal<areal<32,8>> bv = 1e-8;
auto cv = av + bv;
cv.report(std::cout);  // Shows if result is exact or interval
// Output shows whether ubit=0 (exact) or ubit=1 (interval)

// For interval arithmetic - rigorous mathematical bounds
// Classical interval arithmetic guarantees containment of true result
TrackedInterval<double> ia(1.0);
TrackedInterval<double> ib(1e-8);
auto ic = ia + ib;
ic.report(std::cout);  // Shows interval width as error measure

// Create interval with explicit bounds for uncertain input
TrackedInterval<double> uncertain(0.99, 1.01);  // Value known to be in [0.99, 1.01]
auto result = uncertain * ia;
result.report(std::cout);  // Propagates uncertainty through computation

// Explicit strategy override
Tracked<float, ErrorStrategy::Shadow> f = 1.0f;  // Force shadow for float
Tracked<double, ErrorStrategy::Bounded> g = 1.0;  // Use interval arithmetic for double

// Type selection guide:
// - Use cfloat/float/double with Exact for IEEE-specific exact error tracking
// - Use posit with Shadow for tapered precision applications
// - Use lns with TrackedLNS for multiplication-heavy DSP algorithms
// - Use areal for faithful arithmetic with automatic uncertainty tracking
// - Use interval for rigorous numerical analysis with guaranteed bounds
*/

}} // namespace sw::universal
