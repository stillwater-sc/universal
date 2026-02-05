#pragma once
// tracked_exact.hpp: error tracker using exact two_sum/two_prod decomposition
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// TrackedExact<T> provides perfect error tracking for IEEE-like floating-point
// types by using two_sum and two_prod to compute exact rounding errors.
//
// For each operation, the rounding error is captured exactly:
//   a + b = s + r   where s is the floating-point sum and r is the exact error
//   a * b = p + r   where p is the floating-point product and r is the exact error
//
// Error propagation:
//   Addition: cumulative_error += |error_from_this_op|
//   Multiplication: err(a*b) = |a|*err(b) + |b|*err(a) + |rounding_error|
//
// Usage:
//   #include <universal/utility/tracked_exact.hpp>
//
//   using namespace sw::universal;
//
//   TrackedExact<double> a = 1.0;
//   TrackedExact<double> b = 1e-16;
//   auto c = a + b;
//
//   std::cout << "Value: " << c.value() << '\n';
//   std::cout << "Error: " << c.error() << '\n';
//   std::cout << "Valid bits: " << c.valid_bits() << '\n';

#include <cmath>
#include <limits>
#include <type_traits>
#include <ostream>
#include <iomanip>

#include "error_tracking_traits.hpp"

namespace sw { namespace universal {

// ============================================================================
// Error-Free Operations
// Inline versions for use within TrackedExact (avoid dependency issues)
// ============================================================================

namespace detail {

/// two_sum: compute s + r = a + b exactly
/// Returns the floating-point sum s, and stores the exact error in r
template<typename T>
inline T tracked_two_sum(T a, T b, T& r) {
	volatile T s = a + b;
	if (std::isfinite(s)) {
		volatile T bb = s - a;
		r = (a - (s - bb)) + (b - bb);
	} else {
		r = T(0);
	}
	return s;
}

/// two_diff: compute s + r = a - b exactly
template<typename T>
inline T tracked_two_diff(T a, T b, T& r) {
	volatile T s = a - b;
	if (std::isfinite(s)) {
		volatile T bb = s - a;
		r = (a - (s - bb)) - (b + bb);
	} else {
		r = T(0);
	}
	return s;
}

/// split: split a into high and low parts for two_prod
template<typename T>
inline void tracked_split(T a, T& hi, T& lo) {
	// For double: 27 bits in high part
	// For float: 12 bits in high part
	constexpr int BITS = std::is_same_v<T, float> ? 12 : 27;
	const T SPLITTER = T(1ull << BITS) + T(1);
	const T SPLIT_THRESHOLD = std::ldexp(std::numeric_limits<T>::max(), -BITS - 1);

	volatile T temp;
	if (std::abs(a) > SPLIT_THRESHOLD) {
		a = std::ldexp(a, -BITS - 1);
		temp = SPLITTER * a;
		hi = temp - (temp - a);
		lo = a - hi;
		hi = std::ldexp(hi, BITS + 1);
		lo = std::ldexp(lo, BITS + 1);
	} else {
		temp = SPLITTER * a;
		hi = temp - (temp - a);
		lo = a - hi;
	}
}

/// two_prod: compute p + r = a * b exactly
template<typename T>
inline T tracked_two_prod(T a, T b, T& r) {
	volatile T p = a * b;
	if (std::isfinite(p)) {
#if defined(__FMA__) || defined(__AVX2__)
		// Use FMA if available
		r = std::fma(a, b, -p);
#else
		T a_hi, a_lo, b_hi, b_lo;
		tracked_split(a, a_hi, a_lo);
		tracked_split(b, b_hi, b_lo);
		r = ((a_hi * b_hi - p) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo;
#endif
	} else {
		r = T(0);
	}
	return p;
}

/// two_sqr: compute p + r = a * a exactly (optimized for squaring)
template<typename T>
inline T tracked_two_sqr(T a, T& r) {
	volatile T p = a * a;
	if (std::isfinite(p)) {
#if defined(__FMA__) || defined(__AVX2__)
		r = std::fma(a, a, -p);
#else
		T hi, lo;
		tracked_split(a, hi, lo);
		r = ((hi * hi - p) + T(2) * hi * lo) + lo * lo;
#endif
	} else {
		r = T(0);
	}
	return p;
}

} // namespace detail

// ============================================================================
// TrackedExact Class
// ============================================================================

/// TrackedExact provides perfect error tracking for IEEE-like types
/// using two_sum/two_prod decomposition
template<typename T>
class TrackedExact {
	static_assert(error_tracking_traits<T>::has_exact_errors || std::is_floating_point_v<T>,
	              "TrackedExact requires type with exact error computation (IEEE-like floats)");

public:
	using value_type = T;

private:
	T value_;                 ///< The computed value
	double cumulative_error_; ///< Running sum of absolute errors
	uint64_t op_count_;       ///< Number of operations performed

public:
	// ========================================================================
	// Constructors
	// ========================================================================

	/// Default constructor: zero value with no error
	constexpr TrackedExact() noexcept
		: value_(T(0)), cumulative_error_(0.0), op_count_(0) {}

	/// Construct from a value (no error initially)
	constexpr TrackedExact(T v) noexcept
		: value_(v), cumulative_error_(0.0), op_count_(0) {}

	/// Construct with explicit error and op count (internal use)
	constexpr TrackedExact(T v, double err, uint64_t ops) noexcept
		: value_(v), cumulative_error_(err), op_count_(ops) {}

	// Copy and move
	constexpr TrackedExact(const TrackedExact&) noexcept = default;
	constexpr TrackedExact(TrackedExact&&) noexcept = default;
	constexpr TrackedExact& operator=(const TrackedExact&) noexcept = default;
	constexpr TrackedExact& operator=(TrackedExact&&) noexcept = default;

	/// Assign from raw value (resets error)
	constexpr TrackedExact& operator=(T v) noexcept {
		value_ = v;
		cumulative_error_ = 0.0;
		op_count_ = 0;
		return *this;
	}

	// ========================================================================
	// Accessors
	// ========================================================================

	/// Get the computed value
	constexpr T value() const noexcept { return value_; }

	/// Get cumulative absolute error
	constexpr double error() const noexcept { return cumulative_error_; }

	/// Get operation count
	constexpr uint64_t operations() const noexcept { return op_count_; }

	/// Implicit conversion to underlying type
	constexpr operator T() const noexcept { return value_; }

	// ========================================================================
	// Error Metrics
	// ========================================================================

	/// Relative error (error / |value|)
	double relative_error() const noexcept {
		double v = static_cast<double>(value_);
		if (std::abs(v) < std::numeric_limits<double>::min()) return 0.0;
		return cumulative_error_ / std::abs(v);
	}

	/// Estimate of valid bits remaining
	/// -log2(relative_error) gives bits of accuracy
	double valid_bits() const noexcept {
		double rel_err = relative_error();
		if (rel_err <= 0.0) return 53.0;  // Full double precision
		return std::max(0.0, -std::log2(rel_err));
	}

	/// ULPs of error
	double ulps_error() const noexcept {
		if (value_ == T(0)) return 0.0;
		T ulp = std::abs(std::nextafter(value_, std::numeric_limits<T>::infinity()) - value_);
		return cumulative_error_ / static_cast<double>(ulp);
	}

	/// Is the result exact (no accumulated error)?
	bool is_exact() const noexcept {
		return cumulative_error_ == 0.0;
	}

	// ========================================================================
	// Arithmetic Operators
	// ========================================================================

	/// Addition with exact error tracking
	TrackedExact operator+(const TrackedExact& rhs) const {
		T err;
		T sum = detail::tracked_two_sum(value_, rhs.value_, err);
		double total_error = cumulative_error_ + rhs.cumulative_error_ + std::abs(static_cast<double>(err));
		return TrackedExact(sum, total_error, op_count_ + rhs.op_count_ + 1);
	}

	/// Addition with scalar
	TrackedExact operator+(T rhs) const {
		return *this + TrackedExact(rhs);
	}

	/// Subtraction with exact error tracking
	TrackedExact operator-(const TrackedExact& rhs) const {
		T err;
		T diff = detail::tracked_two_diff(value_, rhs.value_, err);
		double total_error = cumulative_error_ + rhs.cumulative_error_ + std::abs(static_cast<double>(err));
		return TrackedExact(diff, total_error, op_count_ + rhs.op_count_ + 1);
	}

	/// Subtraction with scalar
	TrackedExact operator-(T rhs) const {
		return *this - TrackedExact(rhs);
	}

	/// Unary minus
	TrackedExact operator-() const {
		return TrackedExact(-value_, cumulative_error_, op_count_);
	}

	/// Multiplication with exact error tracking
	/// Error propagation: (a + ea) * (b + eb) = ab + a*eb + b*ea + ea*eb
	/// We track: |a|*err(b) + |b|*err(a) + |rounding_error|
	TrackedExact operator*(const TrackedExact& rhs) const {
		T err;
		T prod = detail::tracked_two_prod(value_, rhs.value_, err);

		// Error propagation in multiplication
		double a = static_cast<double>(value_);
		double b = static_cast<double>(rhs.value_);
		double prop_error = std::abs(a) * rhs.cumulative_error_ +
		                    std::abs(b) * cumulative_error_;
		double total_error = prop_error + std::abs(static_cast<double>(err));

		return TrackedExact(prod, total_error, op_count_ + rhs.op_count_ + 1);
	}

	/// Multiplication with scalar
	TrackedExact operator*(T rhs) const {
		return *this * TrackedExact(rhs);
	}

	/// Division with exact error tracking
	/// Division is computed as a * (1/b), tracking the error in reciprocal
	TrackedExact operator/(const TrackedExact& rhs) const {
		if (rhs.value_ == T(0)) {
			// Division by zero - return infinity with max error
			return TrackedExact(value_ / rhs.value_,
			                    std::numeric_limits<double>::infinity(),
			                    op_count_ + rhs.op_count_ + 1);
		}

		// Compute reciprocal error
		// 1/b has relative error approximately equal to relative error of b
		T recip = T(1) / rhs.value_;
		double recip_rel_error = rhs.relative_error();
		double recip_abs_error = std::abs(static_cast<double>(recip)) * recip_rel_error;

		// Now multiply a by reciprocal
		T err;
		T quot = detail::tracked_two_prod(value_, recip, err);

		double a = static_cast<double>(value_);
		double prop_error = std::abs(a) * recip_abs_error +
		                    std::abs(static_cast<double>(recip)) * cumulative_error_;
		double total_error = prop_error + std::abs(static_cast<double>(err));

		return TrackedExact(quot, total_error, op_count_ + rhs.op_count_ + 1);
	}

	/// Division with scalar
	TrackedExact operator/(T rhs) const {
		return *this / TrackedExact(rhs);
	}

	// ========================================================================
	// Compound Assignment Operators
	// ========================================================================

	TrackedExact& operator+=(const TrackedExact& rhs) {
		*this = *this + rhs;
		return *this;
	}

	TrackedExact& operator+=(T rhs) {
		*this = *this + TrackedExact(rhs);
		return *this;
	}

	TrackedExact& operator-=(const TrackedExact& rhs) {
		*this = *this - rhs;
		return *this;
	}

	TrackedExact& operator-=(T rhs) {
		*this = *this - TrackedExact(rhs);
		return *this;
	}

	TrackedExact& operator*=(const TrackedExact& rhs) {
		*this = *this * rhs;
		return *this;
	}

	TrackedExact& operator*=(T rhs) {
		*this = *this * TrackedExact(rhs);
		return *this;
	}

	TrackedExact& operator/=(const TrackedExact& rhs) {
		*this = *this / rhs;
		return *this;
	}

	TrackedExact& operator/=(T rhs) {
		*this = *this / TrackedExact(rhs);
		return *this;
	}

	// ========================================================================
	// Comparison Operators
	// ========================================================================

	bool operator==(const TrackedExact& rhs) const noexcept {
		return value_ == rhs.value_;
	}

	bool operator!=(const TrackedExact& rhs) const noexcept {
		return value_ != rhs.value_;
	}

	bool operator<(const TrackedExact& rhs) const noexcept {
		return value_ < rhs.value_;
	}

	bool operator<=(const TrackedExact& rhs) const noexcept {
		return value_ <= rhs.value_;
	}

	bool operator>(const TrackedExact& rhs) const noexcept {
		return value_ > rhs.value_;
	}

	bool operator>=(const TrackedExact& rhs) const noexcept {
		return value_ >= rhs.value_;
	}

	// ========================================================================
	// Mathematical Functions
	// ========================================================================

	/// Square with optimized error tracking
	TrackedExact sqr() const {
		T err;
		T sq = detail::tracked_two_sqr(value_, err);

		// Error propagation: (a + ea)^2 = a^2 + 2*a*ea + ea^2
		double a = static_cast<double>(value_);
		double prop_error = 2.0 * std::abs(a) * cumulative_error_;
		double total_error = prop_error + std::abs(static_cast<double>(err));

		return TrackedExact(sq, total_error, op_count_ + 1);
	}

	// ========================================================================
	// Reporting
	// ========================================================================

	/// Output a detailed report of the tracked value
	void report(std::ostream& os) const {
		os << "TrackedExact Report:\n";
		os << "  Value:          " << std::setprecision(17) << value_ << '\n';
		os << "  Abs Error:      " << std::scientific << cumulative_error_ << '\n';
		os << "  Rel Error:      " << relative_error() << '\n';
		os << "  Valid bits:     " << std::fixed << std::setprecision(1) << valid_bits() << '\n';
		os << "  ULPs error:     " << std::scientific << ulps_error() << '\n';
		os << "  Operations:     " << op_count_ << '\n';
		os << "  Is exact:       " << (is_exact() ? "yes" : "no") << '\n';
	}
};

// ============================================================================
// Stream Operators
// ============================================================================

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const TrackedExact<T>& v) {
	return os << v.value();
}

// ============================================================================
// Free Function Mathematical Operations
// ============================================================================

/// Absolute value
template<typename T>
TrackedExact<T> abs(const TrackedExact<T>& v) {
	return TrackedExact<T>(std::abs(v.value()), v.error(), v.operations());
}

/// Square root with error propagation
/// sqrt(a + ea) ≈ sqrt(a) + ea/(2*sqrt(a))
template<typename T>
TrackedExact<T> sqrt(const TrackedExact<T>& v) {
	T result = std::sqrt(v.value());
	// Error propagation: d/da sqrt(a) = 1/(2*sqrt(a))
	double derivative = 0.5 / static_cast<double>(result);
	double prop_error = std::abs(derivative) * v.error();
	// Add rounding error estimate (0.5 ULP for sqrt)
	T ulp = std::abs(std::nextafter(result, std::numeric_limits<T>::infinity()) - result);
	double rounding_error = 0.5 * static_cast<double>(ulp);
	return TrackedExact<T>(result, prop_error + rounding_error, v.operations() + 1);
}

/// Square
template<typename T>
TrackedExact<T> sqr(const TrackedExact<T>& v) {
	return v.sqr();
}

// ============================================================================
// Scalar + TrackedExact operations (for commutativity)
// ============================================================================

template<typename T>
TrackedExact<T> operator+(T lhs, const TrackedExact<T>& rhs) {
	return TrackedExact<T>(lhs) + rhs;
}

template<typename T>
TrackedExact<T> operator-(T lhs, const TrackedExact<T>& rhs) {
	return TrackedExact<T>(lhs) - rhs;
}

template<typename T>
TrackedExact<T> operator*(T lhs, const TrackedExact<T>& rhs) {
	return TrackedExact<T>(lhs) * rhs;
}

template<typename T>
TrackedExact<T> operator/(T lhs, const TrackedExact<T>& rhs) {
	return TrackedExact<T>(lhs) / rhs;
}

// ============================================================================
// Type tag for TrackedExact
// ============================================================================

template<typename T>
inline std::string type_tag(const TrackedExact<T>& = {}) {
	return "TrackedExact<" + std::string(typeid(T).name()) + ">";
}

}} // namespace sw::universal
