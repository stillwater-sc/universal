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
// - Each bound is rounded outward whenever the operation was inexact, using an
//   error-free transformation rather than the FPU rounding mode (see #1544 and the
//   note above namespace detail below)
// - Error = interval width (hi - lo)
// - Guaranteed enclosure: true value always in [lo, hi]
// - An operation that rounds nothing does not widen
//
// Trade-offs:
// - More conservative than shadow (intervals can grow)
// - Provides mathematical guarantees (not just estimates)
// - Slightly slower: each bound costs an extra fma or TwoSum
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

#include <universal/utility/directives.hpp>   // UNIVERSAL_TRUSTED_FMA (#1578)
#include <cmath>
#include <iostream>
#include <iomanip>
#include <limits>
#include <type_traits>
#include <algorithm>

namespace sw { namespace universal {

// ============================================================================
// Directed rounding
// ============================================================================
//
// The bounds must enclose the true real result, which means each one has to be
// rounded outward whenever the operation was inexact. This was once done by
// switching the FPU rounding mode around each operation with fesetround. That is
// not sound in practice: without "#pragma STDC FENV_ACCESS ON" -- which GCC and
// Clang do not honour -- the compiler may evaluate the arithmetic under whatever
// mode it likes, including at compile time. Measured before this was replaced,
// Clang at -O2 returned 1/3 as the single point 0.33333333333333337034, the
// UPWARD-rounded value, which excludes the true 1/3 altogether (#1544).
//
// So the widening is done in arithmetic instead, the way interval<Scalar> does it
// (interval_detail in interval_impl.hpp): compute the result, recover the exact
// roundoff with an error-free transformation, and step one ulp outward only in the
// direction the roundoff says the true value lies. Nothing depends on the rounding
// mode, so nothing the optimizer does can invalidate it, and an operation that
// rounds nothing still yields a point interval.
//
// These are correct for the native floating-point types this class accepts; the
// EFTs they rest on are not sound for tapered or subnormal-flushing types, which
// is why TrackedBounded static_asserts on std::is_floating_point.

namespace detail {

template<typename T>
inline T next_up(T x) noexcept {
	return std::nextafter(x, std::numeric_limits<T>::infinity());
}

template<typename T>
inline T next_down(T x) noexcept {
	return std::nextafter(x, -std::numeric_limits<T>::infinity());
}

/// The exact roundoff of a + b, given the rounded sum s (Knuth's TwoSum).
/// Exact for every finite s, subnormal results included. The volatile
/// temporaries keep an aggressive optimizer from algebraically cancelling the
/// expression, which would defeat the whole point.
template<typename T>
inline T two_sum_roundoff(T a, T b, T s) noexcept {
	volatile T bb = s - a;
	volatile T lhs = a - (s - bb);
	volatile T rhs = b - bb;
	return T(lhs) + T(rhs);
}

/// a + b, rounded toward -infinity
template<typename T>
inline T add_down(T a, T b) noexcept {
	T s = a + b;
	if (!std::isfinite(s)) return s;
	return (two_sum_roundoff(a, b, s) < T(0)) ? next_down(s) : s;
}

/// a + b, rounded toward +infinity
template<typename T>
inline T add_up(T a, T b) noexcept {
	T s = a + b;
	if (!std::isfinite(s)) return s;
	return (two_sum_roundoff(a, b, s) > T(0)) ? next_up(s) : s;
}

template<typename T>
inline T sub_down(T a, T b) noexcept { return add_down(a, T(-b)); }

template<typename T>
inline T sub_up(T a, T b) noexcept { return add_up(a, T(-b)); }

/// The exact roundoff of a * b is fma(a, b, -p), which holds whenever the product
/// neither overflows nor falls into the subnormal range; below the smallest normal
/// the roundoff itself is no longer representable, so the sign cannot be trusted
/// and the bound is widened unconditionally. An exactly zero operand is exact.
template<typename T>
inline bool product_roundoff(T a, T b, T p, T& roundoff) noexcept {
	if (a == T(0) || b == T(0)) { roundoff = T(0); return true; }
	if (std::abs(p) < std::numeric_limits<T>::min()) return false;
#if UNIVERSAL_TRUSTED_FMA == 0
	// Without a correctly rounded fma the roundoff is not exact, so its sign cannot be
	// trusted and the bound is widened -- the same conservative answer this function
	// already gives for a subnormal product. Containment is the guarantee; tightness is
	// not (#1578).
	(void)p;
	return false;
#else
	roundoff = std::fma(a, b, -p);
	return std::isfinite(roundoff);
#endif
}

template<typename T>
inline T mul_down(T a, T b) noexcept {
	T p = a * b;
	if (!std::isfinite(p)) return p;
	T e;
	if (!product_roundoff(a, b, p, e)) return next_down(p);
	return (e < T(0)) ? next_down(p) : p;
}

template<typename T>
inline T mul_up(T a, T b) noexcept {
	T p = a * b;
	if (!std::isfinite(p)) return p;
	T e;
	if (!product_roundoff(a, b, p, e)) return next_up(p);
	return (e > T(0)) ? next_up(p) : p;
}

/// Where the true quotient sits relative to the rounded one.
enum class quotient_position { exact, above, below, unknown };

/// For a quotient q of a / b, fma(-q, b, a) is the exact residual a - q*b, and the
/// true quotient lies above q exactly when that residual has the same sign as b.
/// The residual is not to be trusted for a subnormal quotient, as for the product.
template<typename T>
inline quotient_position locate_quotient(T a, T b, T q) noexcept {
	if (a == T(0)) return quotient_position::exact;
	if (std::abs(q) < std::numeric_limits<T>::min()) return quotient_position::unknown;
#if UNIVERSAL_TRUSTED_FMA == 0
	// an inexact residual can carry the wrong sign, and the sign is the whole answer (#1578)
	return quotient_position::unknown;
#else
	T r = std::fma(-q, b, a);
	if (!std::isfinite(r)) return quotient_position::unknown;
	if (r == T(0)) return quotient_position::exact;
	return ((r > T(0)) == (b > T(0))) ? quotient_position::above : quotient_position::below;
#endif
}

template<typename T>
inline T div_down(T a, T b) noexcept {
	T q = a / b;
	if (!std::isfinite(q)) return q;
	const quotient_position where = locate_quotient(a, b, q);
	return (where == quotient_position::below || where == quotient_position::unknown)
		? next_down(q) : q;
}

template<typename T>
inline T div_up(T a, T b) noexcept {
	T q = a / b;
	if (!std::isfinite(q)) return q;
	const quotient_position where = locate_quotient(a, b, q);
	return (where == quotient_position::above || where == quotient_position::unknown)
		? next_up(q) : q;
}

/// sqrt(x) is exact when x - r*r is zero; otherwise the true root lies above r
/// when r*r undershoots x.
template<typename T>
inline T sqrt_down(T x) noexcept {
	T r = std::sqrt(x);
	if (!std::isfinite(r) || r == T(0)) return r;
#if UNIVERSAL_TRUSTED_FMA == 0
	return next_down(r);          // cannot prove r is the correct side, so widen (#1578)
#else
	T e = std::fma(-r, r, x);
	if (!std::isfinite(e)) return next_down(r);
	return (e < T(0)) ? next_down(r) : r;
#endif
}

template<typename T>
inline T sqrt_up(T x) noexcept {
	T r = std::sqrt(x);
	if (!std::isfinite(r) || r == T(0)) return r;
#if UNIVERSAL_TRUSTED_FMA == 0
	return next_up(r);            // cannot prove r is the correct side, so widen (#1578)
#else
	T e = std::fma(-r, r, x);
	if (!std::isfinite(e)) return next_up(r);
	return (e > T(0)) ? next_up(r) : r;
#endif
}

} // namespace detail

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

	/// Get the relative error bound.
	/// A zero-width interval is exact whatever it brackets, zero included: there is
	/// no relative error to report. Only a value that spans zero has a relative
	/// error with no meaning, and that is the case the infinity is for.
	T relative_error() const noexcept {
		T r = radius();
		if (r == T(0)) return T(0);
		T mid = value();
		if (mid == T(0)) return std::numeric_limits<T>::infinity();
		return r / std::abs(mid);
	}

	/// Estimate valid bits of precision, clamped to [0, the type's precision]
	double valid_bits() const noexcept {
		constexpr double type_precision = static_cast<double>(std::numeric_limits<T>::digits);
		T rel = relative_error();
		if (rel <= T(0)) return type_precision;
		if (!std::isfinite(rel)) return 0.0;
		// an interval wider than its own midpoint has nothing left to report
		return std::min(type_precision, std::max(0.0, -std::log2(static_cast<double>(rel))));
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

	/// Addition with outward rounding
	TrackedBounded operator+(const TrackedBounded& rhs) const {
		return TrackedBounded(detail::add_down(lo_, rhs.lo_),
		                      detail::add_up(hi_, rhs.hi_),
		                      ops_ + rhs.ops_ + 1);
	}

	/// Subtraction with outward rounding
	TrackedBounded operator-(const TrackedBounded& rhs) const {
		return TrackedBounded(detail::sub_down(lo_, rhs.hi_),   // lo - hi for the lower bound
		                      detail::sub_up(hi_, rhs.lo_),     // hi - lo for the upper bound
		                      ops_ + rhs.ops_ + 1);
	}

	/// Unary negation
	TrackedBounded operator-() const {
		return TrackedBounded(-hi_, -lo_, ops_);
	}

	/// Multiplication with outward rounding
	/// Must consider all four products to handle signs correctly
	TrackedBounded operator*(const TrackedBounded& rhs) const {
		// each corner product, rounded down for the lower bound and up for the upper
		const T products_lo[4] = { detail::mul_down(lo_, rhs.lo_), detail::mul_down(lo_, rhs.hi_),
		                           detail::mul_down(hi_, rhs.lo_), detail::mul_down(hi_, rhs.hi_) };
		const T products_hi[4] = { detail::mul_up(lo_, rhs.lo_), detail::mul_up(lo_, rhs.hi_),
		                           detail::mul_up(hi_, rhs.lo_), detail::mul_up(hi_, rhs.hi_) };

		T new_lo = products_lo[0];
		T new_hi = products_hi[0];
		for (int i = 1; i < 4; ++i) {
			new_lo = std::min(new_lo, products_lo[i]);
			new_hi = std::max(new_hi, products_hi[i]);
		}

		return TrackedBounded(new_lo, new_hi, ops_ + rhs.ops_ + 1);
	}

	/// Division with outward rounding
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

		// each corner quotient, rounded down for the lower bound and up for the upper
		const T quotients_lo[4] = { detail::div_down(lo_, rhs.lo_), detail::div_down(lo_, rhs.hi_),
		                            detail::div_down(hi_, rhs.lo_), detail::div_down(hi_, rhs.hi_) };
		const T quotients_hi[4] = { detail::div_up(lo_, rhs.lo_), detail::div_up(lo_, rhs.hi_),
		                            detail::div_up(hi_, rhs.lo_), detail::div_up(hi_, rhs.hi_) };

		T new_lo = quotients_lo[0];
		T new_hi = quotients_hi[0];
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
	// Comparison operators
	// ------------------------------------------------------------------------
	//
	// Intervals are only PARTIALLY ordered: two that overlap have no order between
	// them, because the true values they stand for may fall either way round. These
	// operators therefore answer on the bounds, and agree with operator==.
	//
	// They used to compare midpoints, which pretends to a total order and disagreed
	// with operator== -- [0,2] and [1,1] were neither less, nor greater, nor equal
	// (#1547). Reach for definitely_less / definitely_greater / overlaps when the
	// distinction matters at a call site.

	/// Every value in this interval is below every value in rhs
	bool operator<(const TrackedBounded& rhs) const {
		return hi_ < rhs.lo_;
	}

	/// Every value in this interval is above every value in rhs
	bool operator>(const TrackedBounded& rhs) const {
		return lo_ > rhs.hi_;
	}

	bool operator<=(const TrackedBounded& rhs) const {
		return hi_ <= rhs.lo_ || *this == rhs;
	}

	bool operator>=(const TrackedBounded& rhs) const {
		return lo_ >= rhs.hi_ || *this == rhs;
	}

	/// Same interval, not merely an overlapping one
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

	/// Square root with outward rounding
	friend TrackedBounded sqrt(const TrackedBounded& x) {
		if (x.hi_ < T(0)) {
			// Entirely negative - return NaN interval
			T nan = std::numeric_limits<T>::quiet_NaN();
			return TrackedBounded(nan, nan, x.ops_ + 1);
		}

		return TrackedBounded(detail::sqrt_down(std::max(x.lo_, T(0))),
		                      detail::sqrt_up(x.hi_),
		                      x.ops_ + 1);
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
	// rounded outward, so the interval is never narrower than the uncertainty asked for
	T delta = detail::mul_up(std::abs(value), relative_uncertainty);
	return TrackedBounded<T>(detail::sub_down(value, delta), detail::add_up(value, delta));
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
