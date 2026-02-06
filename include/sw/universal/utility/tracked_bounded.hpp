#pragma once
// tracked_bounded.hpp: rigorous interval-based error tracking with directed rounding
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// TrackedBounded<T> provides mathematically rigorous error bounds using interval
// arithmetic with directed rounding. Unlike shadow computation which gives a
// point estimate of error, bounded tracking guarantees the true value lies
// within the computed interval.
//
// Key properties:
// - Uses IEEE directed rounding (fesetround) for rigorous bounds
// - Lower bound computed with round-toward-negative-infinity
// - Upper bound computed with round-toward-positive-infinity
// - Error = interval width (hi - lo)
// - Guaranteed enclosure: true value always in [lo, hi]
//
// Trade-offs:
// - More conservative than shadow (intervals can grow)
// - Provides mathematical guarantees (not just estimates)
// - Slightly slower due to rounding mode switches
//
// Usage:
//   #include <universal/utility/tracked_bounded.hpp>
//
//   using namespace sw::universal;
//
//   TrackedBounded<double> a = 1.0;
//   TrackedBounded<double> b = 3.0;
//   auto c = a / b;  // c bounds 1/3 rigorously
//   std::cout << "Value: " << c.value() << "\n";
//   std::cout << "Bounds: [" << c.lo() << ", " << c.hi() << "]\n";
//   std::cout << "Width: " << c.width() << "\n";
//   std::cout << "Valid bits: " << c.valid_bits() << "\n";

#include <cfenv>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <limits>
#include <type_traits>
#include <algorithm>

// Enable floating-point environment access for directed rounding
// MSVC uses a different pragma than the C99 standard
#if defined(_MSC_VER)
#pragma fenv_access (on)
#elif defined(__GNUC__) || defined(__clang__)
// GCC/Clang: STDC pragma often ignored, but fesetround() still works
#endif

namespace sw { namespace universal {

// ============================================================================
// RAII guard for directed rounding mode
// ============================================================================

/// RAII guard that saves and restores the floating-point rounding mode
class RoundingGuard {
public:
	explicit RoundingGuard(int mode) : saved_mode_(std::fegetround()) {
		std::fesetround(mode);
	}
	~RoundingGuard() {
		std::fesetround(saved_mode_);
	}
	// Non-copyable
	RoundingGuard(const RoundingGuard&) = delete;
	RoundingGuard& operator=(const RoundingGuard&) = delete;
private:
	int saved_mode_;
};

// ============================================================================
// TrackedBounded: Interval-based rigorous error tracking
// ============================================================================

/// TrackedBounded wraps a scalar type with rigorous interval bounds
///
/// @tparam T The underlying scalar type (float, double, long double)
///
/// The value is represented as an interval [lo, hi] that is guaranteed
/// to contain the true mathematical result. Arithmetic operations use
/// directed rounding to maintain this guarantee.
template<typename T>
class TrackedBounded {
	static_assert(std::is_floating_point_v<T>,
		"TrackedBounded requires IEEE floating-point type for directed rounding");

public:
	using value_type = T;

	// ------------------------------------------------------------------------
	// Constructors
	// ------------------------------------------------------------------------

	/// Default constructor: zero interval [0, 0]
	constexpr TrackedBounded() noexcept
		: lo_(T(0)), hi_(T(0)), ops_(0) {}

	/// Construct exact value [v, v]
	TrackedBounded(T v) noexcept
		: lo_(v), hi_(v), ops_(0) {}

	/// Construct from explicit bounds [lo, hi]
	TrackedBounded(T lo, T hi) noexcept
		: lo_(lo), hi_(hi), ops_(0) {
		if (lo_ > hi_) std::swap(lo_, hi_);
	}

	/// Construct from bounds with operation count
	TrackedBounded(T lo, T hi, size_t ops) noexcept
		: lo_(lo), hi_(hi), ops_(ops) {
		if (lo_ > hi_) std::swap(lo_, hi_);
	}

	// ------------------------------------------------------------------------
	// Accessors
	// ------------------------------------------------------------------------

	/// Get the midpoint value (best estimate)
	T value() const noexcept {
		return (lo_ + hi_) / T(2);
	}

	/// Get the lower bound
	T lo() const noexcept { return lo_; }

	/// Get the upper bound
	T hi() const noexcept { return hi_; }

	/// Get the interval width (absolute error bound)
	T width() const noexcept {
		return hi_ - lo_;
	}

	/// Get the radius (half-width)
	T radius() const noexcept {
		return width() / T(2);
	}

	/// Get the absolute error bound
	T error() const noexcept {
		return radius();
	}

	/// Get the relative error bound
	T relative_error() const noexcept {
		T mid = value();
		if (mid == T(0)) return std::numeric_limits<T>::infinity();
		return radius() / std::abs(mid);
	}

	/// Estimate valid bits of precision, capped at type precision
	double valid_bits() const noexcept {
		constexpr double type_precision = static_cast<double>(std::numeric_limits<T>::digits);
		T rel = relative_error();
		if (rel <= T(0)) return type_precision;
		if (!std::isfinite(rel)) return 0.0;
		return std::min(type_precision, -std::log2(static_cast<double>(rel)));
	}

	/// Check if the interval is exact (zero width)
	bool is_exact() const noexcept {
		return lo_ == hi_;
	}

	/// Check if value is definitely positive
	bool is_positive() const noexcept {
		return lo_ > T(0);
	}

	/// Check if value is definitely negative
	bool is_negative() const noexcept {
		return hi_ < T(0);
	}

	/// Check if interval contains zero
	bool contains_zero() const noexcept {
		return lo_ <= T(0) && hi_ >= T(0);
	}

	/// Get operation count
	size_t operations() const noexcept { return ops_; }

	// ------------------------------------------------------------------------
	// Arithmetic operators
	// ------------------------------------------------------------------------

	/// Addition with directed rounding
	TrackedBounded operator+(const TrackedBounded& rhs) const {
		T new_lo, new_hi;
		{
			RoundingGuard guard(FE_DOWNWARD);
			new_lo = lo_ + rhs.lo_;
		}
		{
			RoundingGuard guard(FE_UPWARD);
			new_hi = hi_ + rhs.hi_;
		}
		return TrackedBounded(new_lo, new_hi, ops_ + rhs.ops_ + 1);
	}

	/// Subtraction with directed rounding
	TrackedBounded operator-(const TrackedBounded& rhs) const {
		T new_lo, new_hi;
		{
			RoundingGuard guard(FE_DOWNWARD);
			new_lo = lo_ - rhs.hi_;  // lo - hi for lower bound
		}
		{
			RoundingGuard guard(FE_UPWARD);
			new_hi = hi_ - rhs.lo_;  // hi - lo for upper bound
		}
		return TrackedBounded(new_lo, new_hi, ops_ + rhs.ops_ + 1);
	}

	/// Unary negation
	TrackedBounded operator-() const {
		return TrackedBounded(-hi_, -lo_, ops_);
	}

	/// Multiplication with directed rounding
	/// Must consider all four products to handle signs correctly
	TrackedBounded operator*(const TrackedBounded& rhs) const {
		T new_lo, new_hi;

		// Compute all four possible products with appropriate rounding
		T products_lo[4], products_hi[4];
		{
			RoundingGuard guard(FE_DOWNWARD);
			products_lo[0] = lo_ * rhs.lo_;
			products_lo[1] = lo_ * rhs.hi_;
			products_lo[2] = hi_ * rhs.lo_;
			products_lo[3] = hi_ * rhs.hi_;
		}
		{
			RoundingGuard guard(FE_UPWARD);
			products_hi[0] = lo_ * rhs.lo_;
			products_hi[1] = lo_ * rhs.hi_;
			products_hi[2] = hi_ * rhs.lo_;
			products_hi[3] = hi_ * rhs.hi_;
		}

		// Find min of all lower-rounded products for new_lo
		new_lo = products_lo[0];
		for (int i = 1; i < 4; ++i) {
			new_lo = std::min(new_lo, products_lo[i]);
		}

		// Find max of all upper-rounded products for new_hi
		new_hi = products_hi[0];
		for (int i = 1; i < 4; ++i) {
			new_hi = std::max(new_hi, products_hi[i]);
		}

		return TrackedBounded(new_lo, new_hi, ops_ + rhs.ops_ + 1);
	}

	/// Division with directed rounding
	/// Requires that rhs does not contain zero
	TrackedBounded operator/(const TrackedBounded& rhs) const {
		// Check for division by interval containing zero
		if (rhs.contains_zero()) {
			// Result is unbounded - return [-inf, +inf]
			return TrackedBounded(
				-std::numeric_limits<T>::infinity(),
				std::numeric_limits<T>::infinity(),
				ops_ + rhs.ops_ + 1
			);
		}

		T new_lo, new_hi;

		// Compute all four possible quotients with appropriate rounding
		T quotients_lo[4], quotients_hi[4];
		{
			RoundingGuard guard(FE_DOWNWARD);
			quotients_lo[0] = lo_ / rhs.lo_;
			quotients_lo[1] = lo_ / rhs.hi_;
			quotients_lo[2] = hi_ / rhs.lo_;
			quotients_lo[3] = hi_ / rhs.hi_;
		}
		{
			RoundingGuard guard(FE_UPWARD);
			quotients_hi[0] = lo_ / rhs.lo_;
			quotients_hi[1] = lo_ / rhs.hi_;
			quotients_hi[2] = hi_ / rhs.lo_;
			quotients_hi[3] = hi_ / rhs.hi_;
		}

		// Find min/max
		new_lo = quotients_lo[0];
		new_hi = quotients_hi[0];
		for (int i = 1; i < 4; ++i) {
			new_lo = std::min(new_lo, quotients_lo[i]);
			new_hi = std::max(new_hi, quotients_hi[i]);
		}

		return TrackedBounded(new_lo, new_hi, ops_ + rhs.ops_ + 1);
	}

	/// Compound assignment operators
	TrackedBounded& operator+=(const TrackedBounded& rhs) {
		*this = *this + rhs;
		return *this;
	}

	TrackedBounded& operator-=(const TrackedBounded& rhs) {
		*this = *this - rhs;
		return *this;
	}

	TrackedBounded& operator*=(const TrackedBounded& rhs) {
		*this = *this * rhs;
		return *this;
	}

	TrackedBounded& operator/=(const TrackedBounded& rhs) {
		*this = *this / rhs;
		return *this;
	}

	// ------------------------------------------------------------------------
	// Comparison operators (for ordering, uses midpoint)
	// ------------------------------------------------------------------------

	bool operator<(const TrackedBounded& rhs) const {
		return value() < rhs.value();
	}

	bool operator>(const TrackedBounded& rhs) const {
		return value() > rhs.value();
	}

	bool operator<=(const TrackedBounded& rhs) const {
		return value() <= rhs.value();
	}

	bool operator>=(const TrackedBounded& rhs) const {
		return value() >= rhs.value();
	}

	bool operator==(const TrackedBounded& rhs) const {
		return lo_ == rhs.lo_ && hi_ == rhs.hi_;
	}

	bool operator!=(const TrackedBounded& rhs) const {
		return !(*this == rhs);
	}

	// ------------------------------------------------------------------------
	// Interval-specific comparisons
	// ------------------------------------------------------------------------

	/// Check if this interval definitely less than rhs (no overlap)
	bool definitely_less(const TrackedBounded& rhs) const {
		return hi_ < rhs.lo_;
	}

	/// Check if this interval definitely greater than rhs (no overlap)
	bool definitely_greater(const TrackedBounded& rhs) const {
		return lo_ > rhs.hi_;
	}

	/// Check if intervals overlap
	bool overlaps(const TrackedBounded& rhs) const {
		return lo_ <= rhs.hi_ && hi_ >= rhs.lo_;
	}

	/// Check if this interval contains rhs entirely
	bool contains(const TrackedBounded& rhs) const {
		return lo_ <= rhs.lo_ && hi_ >= rhs.hi_;
	}

	// ------------------------------------------------------------------------
	// Mathematical functions
	// ------------------------------------------------------------------------

	/// Square root with directed rounding
	friend TrackedBounded sqrt(const TrackedBounded& x) {
		if (x.hi_ < T(0)) {
			// Entirely negative - return NaN interval
			T nan = std::numeric_limits<T>::quiet_NaN();
			return TrackedBounded(nan, nan, x.ops_ + 1);
		}

		T new_lo, new_hi;
		{
			RoundingGuard guard(FE_DOWNWARD);
			new_lo = std::sqrt(std::max(x.lo_, T(0)));
		}
		{
			RoundingGuard guard(FE_UPWARD);
			new_hi = std::sqrt(x.hi_);
		}
		return TrackedBounded(new_lo, new_hi, x.ops_ + 1);
	}

	/// Absolute value
	friend TrackedBounded abs(const TrackedBounded& x) {
		if (x.lo_ >= T(0)) {
			// Entirely positive
			return TrackedBounded(x.lo_, x.hi_, x.ops_);
		} else if (x.hi_ <= T(0)) {
			// Entirely negative
			return TrackedBounded(-x.hi_, -x.lo_, x.ops_);
		} else {
			// Contains zero
			return TrackedBounded(T(0), std::max(-x.lo_, x.hi_), x.ops_);
		}
	}

	/// Integer power
	friend TrackedBounded pow(const TrackedBounded& base, int exp) {
		if (exp == 0) return TrackedBounded(T(1));
		if (exp == 1) return base;

		if (exp < 0) {
			return TrackedBounded(T(1)) / pow(base, -exp);
		}

		// Use binary exponentiation
		TrackedBounded result(T(1));
		TrackedBounded b = base;
		int e = exp;
		while (e > 0) {
			if (e & 1) result = result * b;
			b = b * b;
			e >>= 1;
		}
		return result;
	}

	// ------------------------------------------------------------------------
	// Output
	// ------------------------------------------------------------------------

	/// Stream output showing interval
	friend std::ostream& operator<<(std::ostream& os, const TrackedBounded& x) {
		os << "[" << x.lo_ << ", " << x.hi_ << "]";
		return os;
	}

	/// Detailed report
	void report(std::ostream& os) const {
		os << "TrackedBounded Report:\n";
		os << "  Interval:     [" << lo_ << ", " << hi_ << "]\n";
		os << "  Midpoint:     " << value() << "\n";
		os << "  Width:        " << std::scientific << width() << "\n";
		os << "  Radius:       " << radius() << "\n";
		os << "  Rel Error:    " << relative_error() << "\n";
		os << "  Valid bits:   " << std::fixed << std::setprecision(1) << valid_bits() << "\n";
		os << "  Operations:   " << ops_ << "\n";
		os << "  Is exact:     " << (is_exact() ? "yes" : "no") << "\n";
		os << "  Contains 0:   " << (contains_zero() ? "yes" : "no") << "\n";
	}

	/// Get strategy name
	static constexpr const char* strategy_name() {
		return "Bounded";
	}

private:
	T lo_;       ///< Lower bound of interval
	T hi_;       ///< Upper bound of interval
	size_t ops_; ///< Operation count
};

// ============================================================================
// Type aliases for common configurations
// ============================================================================

using TrackedBoundedFloat = TrackedBounded<float>;
using TrackedBoundedDouble = TrackedBounded<double>;

// ============================================================================
// Helper functions
// ============================================================================

/// Compute enclosing interval for a value with relative uncertainty
template<typename T>
TrackedBounded<T> make_uncertain(T value, T relative_uncertainty) {
	T delta = std::abs(value) * relative_uncertainty;
	return TrackedBounded<T>(value - delta, value + delta);
}

/// Compute intersection of two intervals (empty if disjoint)
template<typename T>
TrackedBounded<T> intersect(const TrackedBounded<T>& a, const TrackedBounded<T>& b) {
	T new_lo = std::max(a.lo(), b.lo());
	T new_hi = std::min(a.hi(), b.hi());
	if (new_lo > new_hi) {
		// Empty intersection - return NaN interval
		T nan = std::numeric_limits<T>::quiet_NaN();
		return TrackedBounded<T>(nan, nan);
	}
	return TrackedBounded<T>(new_lo, new_hi);
}

/// Compute hull (union) of two intervals
template<typename T>
TrackedBounded<T> hull(const TrackedBounded<T>& a, const TrackedBounded<T>& b) {
	return TrackedBounded<T>(
		std::min(a.lo(), b.lo()),
		std::max(a.hi(), b.hi())
	);
}

}} // namespace sw::universal
