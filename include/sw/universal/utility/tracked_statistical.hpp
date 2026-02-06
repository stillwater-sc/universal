#pragma once
// tracked_statistical.hpp: fast ULP-based statistical error tracking
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// TrackedStatistical<T> provides fast, approximate error tracking using
// a statistical model based on ULPs (Units in Last Place). Unlike exact
// or shadow tracking, this approach doesn't compute the actual error -
// instead, it estimates error bounds using statistical assumptions about
// floating-point rounding.
//
// Key advantages:
// - Very fast: No shadow computation, minimal overhead
// - Simple: Just tracks operation count and applies model
// - Configurable: Random walk vs linear error growth
//
// Error Models:
// - Random Walk: Error grows as sqrt(n) * 0.5 ULP (optimistic, random errors)
// - Linear:      Error grows as n * 0.5 ULP (pessimistic, correlated errors)
//
// Limitations:
// - Approximate: Only provides estimates, not actual error
// - Assumes typical IEEE rounding behavior
// - Doesn't detect catastrophic cancellation
//
// Usage:
//   #include <universal/utility/tracked_statistical.hpp>
//
//   using namespace sw::universal;
//
//   TrackedStatistical<double> a = 1.0;
//   TrackedStatistical<double> b = 1e-15;
//   auto c = a + b;
//   std::cout << "Estimated error: " << c.error() << " ULPs\n";
//   std::cout << "Valid bits: " << c.valid_bits() << "\n";
//
//   // Use linear model for worst-case analysis
//   TrackedStatistical<double, ErrorModel::Linear> x = 1.0;

#include <cmath>
#include <cstdint>
#include <limits>
#include <iostream>
#include <iomanip>
#include <type_traits>

namespace sw { namespace universal {

// ============================================================================
// Error accumulation models
// ============================================================================

/// How errors accumulate across operations
enum class ErrorModel {
	RandomWalk,  ///< sqrt(n) growth - assumes random, independent errors
	Linear       ///< n growth - assumes worst-case correlated errors
};

// ============================================================================
// ULP utilities
// ============================================================================

/// Get the ULP (Unit in Last Place) of a floating-point value
/// ULP is the spacing between adjacent floating-point values at this magnitude
template<typename T>
inline T ulp(T x) {
	static_assert(std::is_floating_point_v<T>, "ulp requires floating-point type");

	if (!std::isfinite(x)) return std::numeric_limits<T>::quiet_NaN();
	if (x == T(0)) return std::numeric_limits<T>::denorm_min();

	x = std::abs(x);

	// ULP = 2^(exponent - mantissa_bits)
	// For normalized numbers: ulp(x) = epsilon * 2^floor(log2(x))
	int exp;
	std::frexp(x, &exp);
	return std::ldexp(std::numeric_limits<T>::epsilon(), exp - 1);
}

/// Get the number of ULPs between two values
template<typename T>
inline double ulp_distance(T a, T b) {
	static_assert(std::is_floating_point_v<T>, "ulp_distance requires floating-point type");

	if (!std::isfinite(a) || !std::isfinite(b)) {
		return std::numeric_limits<double>::infinity();
	}

	T diff = std::abs(a - b);
	T u = ulp(std::max(std::abs(a), std::abs(b)));

	if (u == T(0)) return 0.0;
	return static_cast<double>(diff / u);
}

/// Get mantissa bits for a type
template<typename T>
constexpr int mantissa_bits() {
	return std::numeric_limits<T>::digits - 1;
}

// ============================================================================
// TrackedStatistical: Fast statistical error estimation
// ============================================================================

/// TrackedStatistical provides fast, approximate error tracking using ULP statistics
///
/// @tparam T The underlying scalar type (float, double, long double)
/// @tparam Model Error accumulation model (RandomWalk or Linear)
///
/// Error is estimated based on operation count and the statistical model,
/// without computing actual reference values. This makes it very fast but
/// only provides estimates, not exact errors.
template<typename T, ErrorModel Model = ErrorModel::RandomWalk>
class TrackedStatistical {
	static_assert(std::is_floating_point_v<T>,
		"TrackedStatistical requires IEEE floating-point type");

public:
	using value_type = T;
	static constexpr ErrorModel model = Model;

	// ------------------------------------------------------------------------
	// Operation costs in ULPs (typical IEEE correctly-rounded operations)
	// ------------------------------------------------------------------------

	static constexpr double ADD_COST = 0.5;     // +/- : 0.5 ULP
	static constexpr double MUL_COST = 0.5;     // *   : 0.5 ULP
	static constexpr double DIV_COST = 0.5;     // /   : 0.5 ULP
	static constexpr double SQRT_COST = 0.5;    // sqrt: 0.5 ULP
	static constexpr double TRANS_COST = 1.0;   // transcendentals: ~1 ULP

	// ------------------------------------------------------------------------
	// Constructors
	// ------------------------------------------------------------------------

	/// Default constructor: zero value
	constexpr TrackedStatistical() noexcept
		: value_(T(0)), ulp_error_(0.0), ops_(0) {}

	/// Construct from value (exact, no error yet)
	TrackedStatistical(T v) noexcept
		: value_(v), ulp_error_(0.0), ops_(0) {}

	/// Construct with known error state
	TrackedStatistical(T v, double ulp_err, size_t ops) noexcept
		: value_(v), ulp_error_(ulp_err), ops_(ops) {}

	// Copy/move
	constexpr TrackedStatistical(const TrackedStatistical&) noexcept = default;
	constexpr TrackedStatistical(TrackedStatistical&&) noexcept = default;
	constexpr TrackedStatistical& operator=(const TrackedStatistical&) noexcept = default;
	constexpr TrackedStatistical& operator=(TrackedStatistical&&) noexcept = default;

	template<typename U>
	TrackedStatistical& operator=(U v) noexcept {
		value_ = static_cast<T>(v);
		ulp_error_ = 0.0;
		ops_ = 0;
		return *this;
	}

	// ------------------------------------------------------------------------
	// Accessors
	// ------------------------------------------------------------------------

	/// Get the computed value
	T value() const noexcept { return value_; }

	/// Get accumulated ULP error estimate
	double ulp_error() const noexcept { return ulp_error_; }

	/// Get total operation count
	size_t operations() const noexcept { return ops_; }

	/// Implicit conversion to underlying type
	operator T() const noexcept { return value_; }

	// ------------------------------------------------------------------------
	// Error metrics
	// ------------------------------------------------------------------------

	/// Get absolute error estimate
	double error() const noexcept {
		return ulp_error_ * static_cast<double>(ulp(value_));
	}

	/// Get relative error estimate
	double relative_error() const noexcept {
		if (value_ == T(0)) return 0.0;
		return error() / std::abs(static_cast<double>(value_));
	}

	/// Estimate valid bits of precision
	double valid_bits() const noexcept {
		if (ulp_error_ <= 0.0) return static_cast<double>(mantissa_bits<T>());
		// Each ULP of error costs ~1 bit
		double lost_bits = std::log2(ulp_error_);
		return std::max(0.0, static_cast<double>(mantissa_bits<T>()) - lost_bits);
	}

	/// Check if value is still considered exact (no operations performed)
	bool is_exact() const noexcept {
		return ops_ == 0;
	}

	/// Get the error model name
	static constexpr const char* model_name() noexcept {
		return Model == ErrorModel::RandomWalk ? "RandomWalk" : "Linear";
	}

	/// Get strategy name
	static constexpr const char* strategy_name() noexcept {
		return "Statistical";
	}

	// ------------------------------------------------------------------------
	// Error accumulation
	// ------------------------------------------------------------------------

	/// Combine errors from two operands plus new operation error
	static double combine_errors(double err1, double err2, double op_cost) noexcept {
		if constexpr (Model == ErrorModel::RandomWalk) {
			// Errors add in quadrature (RSS) for random walk
			return std::sqrt(err1 * err1 + err2 * err2 + op_cost * op_cost);
		} else {
			// Linear (worst-case) accumulation
			return err1 + err2 + op_cost;
		}
	}

	/// Add new operation error to existing error
	static double add_operation_error(double current_err, double op_cost) noexcept {
		if constexpr (Model == ErrorModel::RandomWalk) {
			return std::sqrt(current_err * current_err + op_cost * op_cost);
		} else {
			return current_err + op_cost;
		}
	}

	// ------------------------------------------------------------------------
	// Arithmetic operators
	// ------------------------------------------------------------------------

	TrackedStatistical operator+(const TrackedStatistical& rhs) const {
		T result = value_ + rhs.value_;
		double new_error = combine_errors(ulp_error_, rhs.ulp_error_, ADD_COST);
		return TrackedStatistical(result, new_error, ops_ + rhs.ops_ + 1);
	}

	TrackedStatistical operator-(const TrackedStatistical& rhs) const {
		T result = value_ - rhs.value_;

		// Check for potential cancellation
		double op_cost = ADD_COST;
		if (std::abs(result) < std::abs(value_) * T(0.01) &&
		    std::abs(result) < std::abs(rhs.value_) * T(0.01)) {
			// Near-cancellation: error is magnified
			// The relative error of the result can be much larger
			T larger = std::max(std::abs(value_), std::abs(rhs.value_));
			if (result != T(0)) {
				double magnification = static_cast<double>(larger / std::abs(result));
				op_cost = ADD_COST * std::min(magnification, 1000.0);  // Cap magnification
			}
		}

		double new_error = combine_errors(ulp_error_, rhs.ulp_error_, op_cost);
		return TrackedStatistical(result, new_error, ops_ + rhs.ops_ + 1);
	}

	TrackedStatistical operator-() const {
		return TrackedStatistical(-value_, ulp_error_, ops_);
	}

	TrackedStatistical operator*(const TrackedStatistical& rhs) const {
		T result = value_ * rhs.value_;
		double new_error = combine_errors(ulp_error_, rhs.ulp_error_, MUL_COST);
		return TrackedStatistical(result, new_error, ops_ + rhs.ops_ + 1);
	}

	TrackedStatistical operator/(const TrackedStatistical& rhs) const {
		T result = value_ / rhs.value_;
		double new_error = combine_errors(ulp_error_, rhs.ulp_error_, DIV_COST);
		return TrackedStatistical(result, new_error, ops_ + rhs.ops_ + 1);
	}

	// Compound assignment
	TrackedStatistical& operator+=(const TrackedStatistical& rhs) {
		*this = *this + rhs;
		return *this;
	}

	TrackedStatistical& operator-=(const TrackedStatistical& rhs) {
		*this = *this - rhs;
		return *this;
	}

	TrackedStatistical& operator*=(const TrackedStatistical& rhs) {
		*this = *this * rhs;
		return *this;
	}

	TrackedStatistical& operator/=(const TrackedStatistical& rhs) {
		*this = *this / rhs;
		return *this;
	}

	// ------------------------------------------------------------------------
	// Comparison operators
	// ------------------------------------------------------------------------

	bool operator<(const TrackedStatistical& rhs) const noexcept {
		return value_ < rhs.value_;
	}

	bool operator>(const TrackedStatistical& rhs) const noexcept {
		return value_ > rhs.value_;
	}

	bool operator<=(const TrackedStatistical& rhs) const noexcept {
		return value_ <= rhs.value_;
	}

	bool operator>=(const TrackedStatistical& rhs) const noexcept {
		return value_ >= rhs.value_;
	}

	bool operator==(const TrackedStatistical& rhs) const noexcept {
		return value_ == rhs.value_;
	}

	bool operator!=(const TrackedStatistical& rhs) const noexcept {
		return value_ != rhs.value_;
	}

	// ------------------------------------------------------------------------
	// Uncertain comparison (considering error bounds)
	// ------------------------------------------------------------------------

	/// Are values definitely different (non-overlapping error regions)?
	bool definitely_different(const TrackedStatistical& rhs) const noexcept {
		double my_err = error();
		double rhs_err = rhs.error();
		double diff = std::abs(static_cast<double>(value_) - static_cast<double>(rhs.value_));
		return diff > (my_err + rhs_err);
	}

	/// Could values be equal (overlapping error regions)?
	bool possibly_equal(const TrackedStatistical& rhs) const noexcept {
		return !definitely_different(rhs);
	}

	// ------------------------------------------------------------------------
	// Output
	// ------------------------------------------------------------------------

	friend std::ostream& operator<<(std::ostream& os, const TrackedStatistical& v) {
		return os << v.value_;
	}

	void report(std::ostream& os) const {
		os << "TrackedStatistical Report (" << model_name() << " model):\n";
		os << "  Value:         " << std::setprecision(17) << value_ << "\n";
		os << "  ULP error:     " << std::setprecision(2) << ulp_error_ << " ULPs\n";
		os << "  Abs error:     " << std::scientific << error() << "\n";
		os << "  Rel error:     " << relative_error() << "\n";
		os << "  Valid bits:    " << std::fixed << std::setprecision(1) << valid_bits() << "\n";
		os << "  Operations:    " << ops_ << "\n";
		os << "  Mantissa bits: " << mantissa_bits<T>() << "\n";
		os << "  Is exact:      " << (is_exact() ? "yes" : "no") << "\n";
	}

private:
	T value_;           ///< The computed value
	double ulp_error_;  ///< Accumulated error in ULPs
	size_t ops_;        ///< Operation count
};

// ============================================================================
// Mathematical functions
// ============================================================================

template<typename T, ErrorModel M>
TrackedStatistical<T, M> sqrt(const TrackedStatistical<T, M>& x) {
	using TS = TrackedStatistical<T, M>;
	T result = std::sqrt(x.value());
	double new_error = TS::add_operation_error(x.ulp_error(), TS::SQRT_COST);
	return TS(result, new_error, x.operations() + 1);
}

template<typename T, ErrorModel M>
TrackedStatistical<T, M> abs(const TrackedStatistical<T, M>& x) {
	return TrackedStatistical<T, M>(std::abs(x.value()), x.ulp_error(), x.operations());
}

template<typename T, ErrorModel M>
TrackedStatistical<T, M> exp(const TrackedStatistical<T, M>& x) {
	using TS = TrackedStatistical<T, M>;
	T result = std::exp(x.value());
	double new_error = TS::add_operation_error(x.ulp_error(), TS::TRANS_COST);
	return TS(result, new_error, x.operations() + 1);
}

template<typename T, ErrorModel M>
TrackedStatistical<T, M> log(const TrackedStatistical<T, M>& x) {
	using TS = TrackedStatistical<T, M>;
	T result = std::log(x.value());
	double new_error = TS::add_operation_error(x.ulp_error(), TS::TRANS_COST);
	return TS(result, new_error, x.operations() + 1);
}

template<typename T, ErrorModel M>
TrackedStatistical<T, M> sin(const TrackedStatistical<T, M>& x) {
	using TS = TrackedStatistical<T, M>;
	T result = std::sin(x.value());
	double new_error = TS::add_operation_error(x.ulp_error(), TS::TRANS_COST);
	return TS(result, new_error, x.operations() + 1);
}

template<typename T, ErrorModel M>
TrackedStatistical<T, M> cos(const TrackedStatistical<T, M>& x) {
	using TS = TrackedStatistical<T, M>;
	T result = std::cos(x.value());
	double new_error = TS::add_operation_error(x.ulp_error(), TS::TRANS_COST);
	return TS(result, new_error, x.operations() + 1);
}

template<typename T, ErrorModel M>
TrackedStatistical<T, M> pow(const TrackedStatistical<T, M>& base, int exp) {
	using TS = TrackedStatistical<T, M>;

	if (exp == 0) return TS(T(1));
	if (exp == 1) return base;

	if (exp < 0) {
		return TS(T(1)) / pow(base, -exp);
	}

	// Binary exponentiation
	TS result(T(1));
	TS b = base;
	int e = exp;
	while (e > 0) {
		if (e & 1) result = result * b;
		b = b * b;
		e >>= 1;
	}
	return result;
}

template<typename T, ErrorModel M>
TrackedStatistical<T, M> pow(const TrackedStatistical<T, M>& base,
                             const TrackedStatistical<T, M>& exp) {
	using TS = TrackedStatistical<T, M>;
	T result = std::pow(base.value(), exp.value());
	// pow uses exp and log internally, so costs ~2 transcendental ops
	double new_error = TS::combine_errors(base.ulp_error(), exp.ulp_error(),
	                                       2.0 * TS::TRANS_COST);
	return TS(result, new_error, base.operations() + exp.operations() + 1);
}

// ============================================================================
// Type aliases for common configurations
// ============================================================================

template<typename T>
using TrackedStatisticalRW = TrackedStatistical<T, ErrorModel::RandomWalk>;

template<typename T>
using TrackedStatisticalLinear = TrackedStatistical<T, ErrorModel::Linear>;

// Common instantiations
using TrackedStatFloat = TrackedStatistical<float>;
using TrackedStatDouble = TrackedStatistical<double>;
using TrackedStatFloatLinear = TrackedStatistical<float, ErrorModel::Linear>;
using TrackedStatDoubleLinear = TrackedStatistical<double, ErrorModel::Linear>;

// ============================================================================
// Comparison helper: compare actual vs estimated error
// ============================================================================

/// Compare statistical estimate against actual shadow computation
/// Useful for validating the statistical model
template<typename T, ErrorModel M>
struct StatisticalValidation {
	T value;
	T shadow;
	double actual_error;
	double estimated_error;
	double actual_ulps;
	double estimated_ulps;
	bool conservative;  // Is estimate >= actual?

	static StatisticalValidation compute(
		const TrackedStatistical<T, M>& tracked,
		T shadow_value
	) {
		StatisticalValidation v;
		v.value = tracked.value();
		v.shadow = shadow_value;
		v.actual_error = std::abs(static_cast<double>(v.value) - static_cast<double>(v.shadow));
		v.estimated_error = tracked.error();
		v.actual_ulps = ulp_distance(v.value, v.shadow);
		v.estimated_ulps = tracked.ulp_error();
		v.conservative = v.estimated_ulps >= v.actual_ulps;
		return v;
	}

	void report(std::ostream& os) const {
		os << "Statistical Validation:\n";
		os << "  Value:          " << value << "\n";
		os << "  Shadow:         " << shadow << "\n";
		os << "  Actual error:   " << std::scientific << actual_error << " ("
		   << std::fixed << std::setprecision(2) << actual_ulps << " ULPs)\n";
		os << "  Estimated error:" << std::scientific << estimated_error << " ("
		   << std::fixed << std::setprecision(2) << estimated_ulps << " ULPs)\n";
		os << "  Conservative:   " << (conservative ? "yes" : "NO - underestimate!") << "\n";
	}
};

}} // namespace sw::universal
