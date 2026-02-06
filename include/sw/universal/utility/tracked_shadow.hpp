#pragma once
// tracked_shadow.hpp: error tracker using higher-precision shadow computation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// TrackedShadow<T> provides error tracking for types that don't support exact
// error decomposition (like posits) by maintaining a higher-precision "shadow"
// value that tracks the mathematically correct result.
//
// For each operation, both the target type and shadow type compute the result.
// Error is then: |shadow_value - double(computed_value)|
//
// This approach works for any arithmetic type, making it suitable for:
// - posit (tapered precision, no clean error separation)
// - lns (though multiplication is exact, additions need tracking)
// - Any custom number type
//
// The shadow type is determined by error_tracking_traits<T>::shadow_type,
// defaulting to double for small types and long double for larger ones.
//
// Usage:
//   #include <universal/utility/tracked_shadow.hpp>
//   #include <universal/number/posit/posit.hpp>
//
//   using namespace sw::universal;
//
//   TrackedShadow<posit<32,2>> a = 1.0;
//   TrackedShadow<posit<32,2>> b = 1e-8;
//   auto c = a + b;
//
//   std::cout << "Value: " << double(c.value()) << '\n';
//   std::cout << "Shadow: " << c.shadow() << '\n';
//   std::cout << "Error: " << c.error() << '\n';

#include <cmath>
#include <limits>
#include <type_traits>
#include <ostream>
#include <iomanip>

#include "error_tracking_traits.hpp"

namespace sw { namespace universal {

// ============================================================================
// TrackedShadow Class
// ============================================================================

/// TrackedShadow provides error tracking via higher-precision shadow computation
/// Suitable for posits and other types without exact error decomposition
template<typename T, typename ShadowType = typename error_tracking_traits<T>::shadow_type>
class TrackedShadow {
public:
	using value_type = T;
	using shadow_type = ShadowType;

private:
	T value_;               ///< The computed value in target type
	ShadowType shadow_;     ///< Higher-precision shadow for reference
	uint64_t op_count_;     ///< Number of operations performed

public:
	// ========================================================================
	// Constructors
	// ========================================================================

	/// Default constructor: zero value
	constexpr TrackedShadow() noexcept
		: value_(T(0)), shadow_(ShadowType(0)), op_count_(0) {}

	/// Construct from a value (same value for both representations)
	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> || std::is_same_v<U, T>>>
	constexpr TrackedShadow(U v) noexcept
		: value_(static_cast<T>(v))
		, shadow_(static_cast<ShadowType>(v))
		, op_count_(0) {}

	/// Construct with explicit shadow and op count (internal use)
	constexpr TrackedShadow(T v, ShadowType s, uint64_t ops) noexcept
		: value_(v), shadow_(s), op_count_(ops) {}

	// Copy and move
	constexpr TrackedShadow(const TrackedShadow&) noexcept = default;
	constexpr TrackedShadow(TrackedShadow&&) noexcept = default;
	constexpr TrackedShadow& operator=(const TrackedShadow&) noexcept = default;
	constexpr TrackedShadow& operator=(TrackedShadow&&) noexcept = default;

	/// Assign from raw value (resets shadow to match)
	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> || std::is_same_v<U, T>>>
	constexpr TrackedShadow& operator=(U v) noexcept {
		value_ = static_cast<T>(v);
		shadow_ = static_cast<ShadowType>(v);
		op_count_ = 0;
		return *this;
	}

	// ========================================================================
	// Accessors
	// ========================================================================

	/// Get the computed value in target type
	constexpr T value() const noexcept { return value_; }

	/// Get the shadow value (higher precision reference)
	constexpr ShadowType shadow() const noexcept { return shadow_; }

	/// Get operation count
	constexpr uint64_t operations() const noexcept { return op_count_; }

	/// Implicit conversion to underlying type
	constexpr operator T() const noexcept { return value_; }

	// ========================================================================
	// Error Metrics
	// ========================================================================

	/// Absolute error: |shadow - double(value)|
	double error() const noexcept {
		return std::abs(static_cast<double>(shadow_) - static_cast<double>(value_));
	}

	/// Relative error: error / |shadow|
	double relative_error() const noexcept {
		double s = static_cast<double>(shadow_);
		if (std::abs(s) < std::numeric_limits<double>::min()) return 0.0;
		return error() / std::abs(s);
	}

	/// Estimate of valid bits remaining: -log2(relative_error)
	double valid_bits() const noexcept {
		double rel_err = relative_error();
		if (rel_err <= 0.0) return 53.0;  // Full precision
		return std::max(0.0, -std::log2(rel_err));
	}

	/// Is the result exact (shadow matches value)?
	bool is_exact() const noexcept {
		return static_cast<double>(shadow_) == static_cast<double>(value_);
	}

	/// Error in ULPs (approximate, based on target type)
	double ulps_error() const noexcept {
		double v = static_cast<double>(value_);
		if (v == 0.0) return 0.0;
		// Estimate ULP based on value magnitude
		double ulp = std::abs(v) * std::numeric_limits<double>::epsilon();
		return error() / ulp;
	}

	// ========================================================================
	// Arithmetic Operators
	// ========================================================================

	/// Addition: compute in both types
	TrackedShadow operator+(const TrackedShadow& rhs) const {
		T result = value_ + rhs.value_;
		ShadowType exact = shadow_ + rhs.shadow_;
		return TrackedShadow(result, exact, op_count_ + rhs.op_count_ + 1);
	}

	/// Addition with scalar
	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedShadow operator+(U rhs) const {
		return *this + TrackedShadow(rhs);
	}

	/// Subtraction: compute in both types
	TrackedShadow operator-(const TrackedShadow& rhs) const {
		T result = value_ - rhs.value_;
		ShadowType exact = shadow_ - rhs.shadow_;
		return TrackedShadow(result, exact, op_count_ + rhs.op_count_ + 1);
	}

	/// Subtraction with scalar
	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedShadow operator-(U rhs) const {
		return *this - TrackedShadow(rhs);
	}

	/// Unary minus
	TrackedShadow operator-() const {
		return TrackedShadow(-value_, -shadow_, op_count_);
	}

	/// Multiplication: compute in both types
	TrackedShadow operator*(const TrackedShadow& rhs) const {
		T result = value_ * rhs.value_;
		ShadowType exact = shadow_ * rhs.shadow_;
		return TrackedShadow(result, exact, op_count_ + rhs.op_count_ + 1);
	}

	/// Multiplication with scalar
	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedShadow operator*(U rhs) const {
		return *this * TrackedShadow(rhs);
	}

	/// Division: compute in both types
	TrackedShadow operator/(const TrackedShadow& rhs) const {
		T result = value_ / rhs.value_;
		ShadowType exact = shadow_ / rhs.shadow_;
		return TrackedShadow(result, exact, op_count_ + rhs.op_count_ + 1);
	}

	/// Division with scalar
	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedShadow operator/(U rhs) const {
		return *this / TrackedShadow(rhs);
	}

	// ========================================================================
	// Compound Assignment Operators
	// ========================================================================

	TrackedShadow& operator+=(const TrackedShadow& rhs) {
		*this = *this + rhs;
		return *this;
	}

	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedShadow& operator+=(U rhs) {
		*this = *this + TrackedShadow(rhs);
		return *this;
	}

	TrackedShadow& operator-=(const TrackedShadow& rhs) {
		*this = *this - rhs;
		return *this;
	}

	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedShadow& operator-=(U rhs) {
		*this = *this - TrackedShadow(rhs);
		return *this;
	}

	TrackedShadow& operator*=(const TrackedShadow& rhs) {
		*this = *this * rhs;
		return *this;
	}

	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedShadow& operator*=(U rhs) {
		*this = *this * TrackedShadow(rhs);
		return *this;
	}

	TrackedShadow& operator/=(const TrackedShadow& rhs) {
		*this = *this / rhs;
		return *this;
	}

	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedShadow& operator/=(U rhs) {
		*this = *this / TrackedShadow(rhs);
		return *this;
	}

	// ========================================================================
	// Comparison Operators
	// ========================================================================

	bool operator==(const TrackedShadow& rhs) const noexcept {
		return value_ == rhs.value_;
	}

	bool operator!=(const TrackedShadow& rhs) const noexcept {
		return value_ != rhs.value_;
	}

	bool operator<(const TrackedShadow& rhs) const noexcept {
		return value_ < rhs.value_;
	}

	bool operator<=(const TrackedShadow& rhs) const noexcept {
		return value_ <= rhs.value_;
	}

	bool operator>(const TrackedShadow& rhs) const noexcept {
		return value_ > rhs.value_;
	}

	bool operator>=(const TrackedShadow& rhs) const noexcept {
		return value_ >= rhs.value_;
	}

	// ========================================================================
	// Reporting
	// ========================================================================

	/// Output a detailed report of the tracked value
	void report(std::ostream& os) const {
		os << "TrackedShadow Report:\n";
		os << "  Value:          " << std::setprecision(17) << static_cast<double>(value_) << '\n';
		os << "  Shadow:         " << static_cast<double>(shadow_) << '\n';
		os << "  Abs Error:      " << std::scientific << error() << '\n';
		os << "  Rel Error:      " << relative_error() << '\n';
		os << "  Valid bits:     " << std::fixed << std::setprecision(1) << valid_bits() << '\n';
		os << "  Operations:     " << op_count_ << '\n';
		os << "  Is exact:       " << (is_exact() ? "yes" : "no") << '\n';
	}
};

// ============================================================================
// Stream Operators
// ============================================================================

template<typename T, typename S>
inline std::ostream& operator<<(std::ostream& os, const TrackedShadow<T, S>& v) {
	return os << static_cast<double>(v.value());
}

// ============================================================================
// Free Function Mathematical Operations
// ============================================================================

/// Absolute value
template<typename T, typename S>
TrackedShadow<T, S> abs(const TrackedShadow<T, S>& v) {
	using std::abs;
	T val = v.value();
	S shad = v.shadow();
	// Handle abs properly for types that might not have std::abs
	T abs_val = (val < T(0)) ? -val : val;
	S abs_shad = (shad < S(0)) ? -shad : shad;
	return TrackedShadow<T, S>(abs_val, abs_shad, v.operations());
}

/// Square root
template<typename T, typename S>
TrackedShadow<T, S> sqrt(const TrackedShadow<T, S>& v) {
	using std::sqrt;
	T result = sqrt(v.value());
	S exact = sqrt(v.shadow());
	return TrackedShadow<T, S>(result, exact, v.operations() + 1);
}

/// Square
template<typename T, typename S>
TrackedShadow<T, S> sqr(const TrackedShadow<T, S>& v) {
	return v * v;
}

/// Power
template<typename T, typename S>
TrackedShadow<T, S> pow(const TrackedShadow<T, S>& base, int exp) {
	using std::pow;
	T result = pow(base.value(), exp);
	S exact = pow(base.shadow(), exp);
	return TrackedShadow<T, S>(result, exact, base.operations() + 1);
}

/// Exponential
template<typename T, typename S>
TrackedShadow<T, S> exp(const TrackedShadow<T, S>& v) {
	using std::exp;
	T result = exp(v.value());
	S exact = exp(v.shadow());
	return TrackedShadow<T, S>(result, exact, v.operations() + 1);
}

/// Natural logarithm
template<typename T, typename S>
TrackedShadow<T, S> log(const TrackedShadow<T, S>& v) {
	using std::log;
	T result = log(v.value());
	S exact = log(v.shadow());
	return TrackedShadow<T, S>(result, exact, v.operations() + 1);
}

/// Sine
template<typename T, typename S>
TrackedShadow<T, S> sin(const TrackedShadow<T, S>& v) {
	using std::sin;
	T result = sin(v.value());
	S exact = sin(v.shadow());
	return TrackedShadow<T, S>(result, exact, v.operations() + 1);
}

/// Cosine
template<typename T, typename S>
TrackedShadow<T, S> cos(const TrackedShadow<T, S>& v) {
	using std::cos;
	T result = cos(v.value());
	S exact = cos(v.shadow());
	return TrackedShadow<T, S>(result, exact, v.operations() + 1);
}

// ============================================================================
// Scalar + TrackedShadow operations (for commutativity)
// ============================================================================

template<typename U, typename T, typename S,
         typename = std::enable_if_t<std::is_arithmetic_v<U>>>
TrackedShadow<T, S> operator+(U lhs, const TrackedShadow<T, S>& rhs) {
	return TrackedShadow<T, S>(lhs) + rhs;
}

template<typename U, typename T, typename S,
         typename = std::enable_if_t<std::is_arithmetic_v<U>>>
TrackedShadow<T, S> operator-(U lhs, const TrackedShadow<T, S>& rhs) {
	return TrackedShadow<T, S>(lhs) - rhs;
}

template<typename U, typename T, typename S,
         typename = std::enable_if_t<std::is_arithmetic_v<U>>>
TrackedShadow<T, S> operator*(U lhs, const TrackedShadow<T, S>& rhs) {
	return TrackedShadow<T, S>(lhs) * rhs;
}

template<typename U, typename T, typename S,
         typename = std::enable_if_t<std::is_arithmetic_v<U>>>
TrackedShadow<T, S> operator/(U lhs, const TrackedShadow<T, S>& rhs) {
	return TrackedShadow<T, S>(lhs) / rhs;
}

// ============================================================================
// Type tag for TrackedShadow
// ============================================================================

template<typename T, typename S>
inline std::string type_tag(const TrackedShadow<T, S>& = {}) {
	return "TrackedShadow<" + std::string(typeid(T).name()) + ">";
}

// ============================================================================
// Convenience alias for posit tracking
// ============================================================================

// Forward declaration
template<unsigned nbits, unsigned es> class posit;

/// TrackedPosit is a convenience alias for TrackedShadow<posit<nbits,es>>
template<unsigned nbits, unsigned es>
using TrackedPosit = TrackedShadow<posit<nbits, es>>;

}} // namespace sw::universal
