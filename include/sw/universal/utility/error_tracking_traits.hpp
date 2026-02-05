#pragma once
// error_tracking_traits.hpp: type traits for error tracking capabilities
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Error tracking traits enable automatic selection of error tracking strategies
// based on the capabilities of different number types:
//
// - Exact: two_sum/two_prod for IEEE floats (perfect error computation)
// - Shadow: Higher precision shadow computation for posits, etc.
// - Bounded: Interval arithmetic for rigorous bounds
// - Statistical: ULP-based statistical model (fast, approximate)
// - Inherent: Type natively tracks uncertainty (areal, interval, valid)
//
// Usage:
//   #include <universal/utility/error_tracking_traits.hpp>
//
//   using namespace sw::universal;
//
//   // Check type capabilities
//   static_assert(error_tracking_traits<float>::has_exact_errors);
//   static_assert(error_tracking_traits<posit<32,2>>::default_strategy == ErrorStrategy::Shadow);
//   static_assert(error_tracking_traits<lns<32,8>>::exact_multiplication);
//   static_assert(error_tracking_traits<areal<32,8>>::tracks_uncertainty);
//   static_assert(error_tracking_traits<interval<double>>::is_interval_type);
//
//   // Get shadow type for any number system
//   using ShadowType = typename error_tracking_traits<posit<32,2>>::shadow_type;

#include <type_traits>
#include <cstdint>

namespace sw { namespace universal {

// ============================================================================
// Error Tracking Strategies
// ============================================================================

/// Enumeration of available error tracking strategies
enum class ErrorStrategy {
	Exact,       ///< two_sum/two_prod - IEEE floats only, perfect error tracking
	Shadow,      ///< Higher precision shadow computation
	Bounded,     ///< Interval arithmetic for rigorous bounds
	Statistical, ///< ULP-based statistical model (fast, approximate)
	Inherent     ///< Type natively tracks error (areal, interval, valid)
};

// ============================================================================
// Forward declarations for Universal number types
// ============================================================================

// cfloat: classic floating-point
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
class cfloat;

// posit: tapered floating-point
template<unsigned nbits, unsigned es>
class posit;

// lns: logarithmic number system
template<unsigned nbits, unsigned rbits, typename bt, auto...x>
class lns;

// areal: faithful floating-point with uncertainty bit
template<unsigned nbits, unsigned es, typename bt>
class areal;

// interval: classical interval arithmetic
template<typename Scalar>
class interval;

// valid: posit-based interval arithmetic
template<unsigned nbits, unsigned es>
class valid;

// ============================================================================
// Base error_tracking_traits template
// ============================================================================

/// Primary template for error tracking traits
/// Provides default values suitable for unknown types
template<typename T, typename = void>
struct error_tracking_traits {
	/// Does this type support exact error computation (two_sum/two_prod)?
	static constexpr bool has_exact_errors = false;

	/// Does this type support directed rounding for interval arithmetic?
	static constexpr bool has_directed_rounding = false;

	/// Is multiplication exact in this representation?
	/// (True for LNS where mult is just addition in log domain)
	static constexpr bool exact_multiplication = false;

	/// Does this type natively track uncertainty?
	/// (True for areal with ubit, interval types)
	static constexpr bool tracks_uncertainty = false;

	/// Is this an interval type that represents a range of values?
	static constexpr bool is_interval_type = false;

	/// Recommended default error tracking strategy
	static constexpr ErrorStrategy default_strategy = ErrorStrategy::Shadow;

	/// Type to use for shadow computation (higher precision reference)
	using shadow_type = long double;

	/// Number of bits in the type (0 if unknown/variable)
	static constexpr unsigned nbits = 0;
};

// ============================================================================
// IEEE float specialization
// ============================================================================

template<>
struct error_tracking_traits<float> {
	static constexpr bool has_exact_errors = true;       // two_sum/two_prod work
	static constexpr bool has_directed_rounding = true;  // fesetround available
	static constexpr bool exact_multiplication = false;
	static constexpr bool tracks_uncertainty = false;
	static constexpr bool is_interval_type = false;
	static constexpr ErrorStrategy default_strategy = ErrorStrategy::Exact;
	using shadow_type = double;
	static constexpr unsigned nbits = 32;
};

// ============================================================================
// IEEE double specialization
// ============================================================================

template<>
struct error_tracking_traits<double> {
	static constexpr bool has_exact_errors = true;       // two_sum/two_prod work
	static constexpr bool has_directed_rounding = true;  // fesetround available
	static constexpr bool exact_multiplication = false;
	static constexpr bool tracks_uncertainty = false;
	static constexpr bool is_interval_type = false;
	static constexpr ErrorStrategy default_strategy = ErrorStrategy::Exact;
	using shadow_type = long double;
	static constexpr unsigned nbits = 64;
};

// ============================================================================
// IEEE long double specialization
// ============================================================================

template<>
struct error_tracking_traits<long double> {
	static constexpr bool has_exact_errors = true;       // two_sum/two_prod work
	static constexpr bool has_directed_rounding = true;  // fesetround available
	static constexpr bool exact_multiplication = false;
	static constexpr bool tracks_uncertainty = false;
	static constexpr bool is_interval_type = false;
	static constexpr ErrorStrategy default_strategy = ErrorStrategy::Exact;
	using shadow_type = long double;  // No higher precision available
	static constexpr unsigned nbits = sizeof(long double) * 8;
};

// ============================================================================
// cfloat specialization
// Classic floating-point with configurable subnormals/supernormals
// ============================================================================

template<unsigned _nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
struct error_tracking_traits<cfloat<_nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>> {
	static constexpr bool has_exact_errors = true;       // two_sum/two_prod work for IEEE-like
	static constexpr bool has_directed_rounding = false; // Not yet implemented
	static constexpr bool exact_multiplication = false;
	static constexpr bool tracks_uncertainty = false;
	static constexpr bool is_interval_type = false;
	static constexpr ErrorStrategy default_strategy = ErrorStrategy::Exact;

	// Shadow to double if small, long double if larger
	using shadow_type = std::conditional_t<(_nbits <= 32), double, long double>;
	static constexpr unsigned nbits = _nbits;
};

// ============================================================================
// posit specialization
// Tapered floating-point with variable precision
// No clean error separation possible due to tapered precision
// ============================================================================

template<unsigned _nbits, unsigned es>
struct error_tracking_traits<posit<_nbits, es>> {
	static constexpr bool has_exact_errors = false;      // No two_sum for posits
	static constexpr bool has_directed_rounding = false; // Not applicable
	static constexpr bool exact_multiplication = false;
	static constexpr bool tracks_uncertainty = false;
	static constexpr bool is_interval_type = false;
	static constexpr ErrorStrategy default_strategy = ErrorStrategy::Shadow;

	// Use double for small posits, long double for larger
	using shadow_type = std::conditional_t<(_nbits <= 32), double, long double>;
	static constexpr unsigned nbits = _nbits;
};

// ============================================================================
// lns specialization
// Logarithmic Number System
// KEY INSIGHT: Multiplication is EXACT (it's addition in log domain)
// Only addition/subtraction introduces error
// ============================================================================

template<unsigned _nbits, unsigned rbits, typename bt, auto...x>
struct error_tracking_traits<lns<_nbits, rbits, bt, x...>> {
	static constexpr bool has_exact_errors = false;      // No two_sum
	static constexpr bool has_directed_rounding = false;
	static constexpr bool exact_multiplication = true;   // KEY: Mult is exact!
	static constexpr bool tracks_uncertainty = false;
	static constexpr bool is_interval_type = false;
	static constexpr ErrorStrategy default_strategy = ErrorStrategy::Shadow;

	using shadow_type = double;
	static constexpr unsigned nbits = _nbits;
};

// ============================================================================
// areal specialization
// Faithful floating-point with uncertainty bit (ubit)
// The ubit indicates whether value is exact or represents an interval
//
// Encoding: [sign | exponent | fraction | ubit]
// - ubit=0: Value is exactly representable
// - ubit=1: True value lies in open interval (value, next_value)
// ============================================================================

template<unsigned _nbits, unsigned es, typename bt>
struct error_tracking_traits<areal<_nbits, es, bt>> {
	static constexpr bool has_exact_errors = false;
	static constexpr bool has_directed_rounding = false;
	static constexpr bool exact_multiplication = false;
	static constexpr bool tracks_uncertainty = true;     // KEY: Native tracking!
	static constexpr bool is_interval_type = true;       // It's an interval type
	static constexpr ErrorStrategy default_strategy = ErrorStrategy::Inherent;

	// Shadow typically not needed since uncertainty is tracked natively
	using shadow_type = double;
	static constexpr unsigned nbits = _nbits;
};

// ============================================================================
// interval specialization
// Classical interval arithmetic [lo, hi]
// Provides rigorous mathematical bounds via directed rounding
// ============================================================================

template<typename Scalar>
struct error_tracking_traits<interval<Scalar>> {
	static constexpr bool has_exact_errors = false;
	static constexpr bool has_directed_rounding = error_tracking_traits<Scalar>::has_directed_rounding;
	static constexpr bool exact_multiplication = false;
	static constexpr bool tracks_uncertainty = true;     // KEY: Native tracking!
	static constexpr bool is_interval_type = true;
	static constexpr ErrorStrategy default_strategy = ErrorStrategy::Inherent;

	// Self-shadowing: interval bounds themselves
	using shadow_type = interval<Scalar>;
	static constexpr unsigned nbits = 2 * error_tracking_traits<Scalar>::nbits;
};

// ============================================================================
// valid specialization
// Posit-based interval arithmetic with open/closed bounds
// ============================================================================

template<unsigned _nbits, unsigned es>
struct error_tracking_traits<valid<_nbits, es>> {
	static constexpr bool has_exact_errors = false;
	static constexpr bool has_directed_rounding = false;
	static constexpr bool exact_multiplication = false;
	static constexpr bool tracks_uncertainty = true;     // Has open/closed bounds
	static constexpr bool is_interval_type = true;
	static constexpr ErrorStrategy default_strategy = ErrorStrategy::Inherent;

	using shadow_type = valid<_nbits, es>;  // Self-shadowing
	static constexpr unsigned nbits = 2 * _nbits;  // Two posit bounds
};

// ============================================================================
// Convenience type aliases and helper metafunctions
// ============================================================================

/// Get the shadow type for a given number type
template<typename T>
using shadow_type_t = typename error_tracking_traits<T>::shadow_type;

/// Check if a type supports exact error tracking
template<typename T>
inline constexpr bool has_exact_errors_v = error_tracking_traits<T>::has_exact_errors;

/// Check if a type has exact multiplication (LNS)
template<typename T>
inline constexpr bool exact_multiplication_v = error_tracking_traits<T>::exact_multiplication;

/// Check if a type natively tracks uncertainty
template<typename T>
inline constexpr bool tracks_uncertainty_v = error_tracking_traits<T>::tracks_uncertainty;

/// Check if a type is an interval type
template<typename T>
inline constexpr bool is_interval_type_v = error_tracking_traits<T>::is_interval_type;

/// Get the default error strategy for a type
template<typename T>
inline constexpr ErrorStrategy default_strategy_v = error_tracking_traits<T>::default_strategy;

// ============================================================================
// Strategy name utilities
// ============================================================================

/// Get a string name for an error strategy
inline constexpr const char* strategy_name(ErrorStrategy s) {
	switch (s) {
		case ErrorStrategy::Exact:       return "Exact";
		case ErrorStrategy::Shadow:      return "Shadow";
		case ErrorStrategy::Bounded:     return "Bounded";
		case ErrorStrategy::Statistical: return "Statistical";
		case ErrorStrategy::Inherent:    return "Inherent";
		default:                         return "Unknown";
	}
}

// ============================================================================
// Traits summary output (for debugging/documentation)
// ============================================================================

/// Output a summary of error tracking traits for a type
template<typename T>
void report_error_tracking_traits(std::ostream& os) {
	using traits = error_tracking_traits<T>;
	os << "Error Tracking Traits Summary:\n";
	os << "  has_exact_errors:     " << (traits::has_exact_errors ? "yes" : "no") << '\n';
	os << "  has_directed_rounding:" << (traits::has_directed_rounding ? "yes" : "no") << '\n';
	os << "  exact_multiplication: " << (traits::exact_multiplication ? "yes" : "no") << '\n';
	os << "  tracks_uncertainty:   " << (traits::tracks_uncertainty ? "yes" : "no") << '\n';
	os << "  is_interval_type:     " << (traits::is_interval_type ? "yes" : "no") << '\n';
	os << "  default_strategy:     " << strategy_name(traits::default_strategy) << '\n';
	os << "  nbits:                " << traits::nbits << '\n';
}

}} // namespace sw::universal
