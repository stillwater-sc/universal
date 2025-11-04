#pragma once
// test_suite_mathlib_adaptive.hpp: adaptive precision testing utilities for mathlib functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>
#include <string>

namespace sw { namespace universal {

	/////////////////////////////////////////////////////////////////////////////////////////
	///                    ADAPTIVE PRECISION THRESHOLD UTILITIES                         ///
	/////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// Get adaptive epsilon threshold based on the precision of the arithmetic type.
	/// The threshold is set to 10^(-(digits10 - margin)) to provide a conservative
	/// error bound based on the type's decimal precision.
	/// </summary>
	/// <typeparam name="Real">Arithmetic type to test</typeparam>
	/// <param name="margin">Safety margin in decimal digits (default 2)</param>
	/// <returns>Threshold value for error checking</returns>
	template<typename Real>
	inline double get_adaptive_threshold(int margin = 2) {
		constexpr int digits10 = std::numeric_limits<Real>::digits10;

		// For types with very low precision (< margin digits), use a minimum threshold
		if (digits10 <= margin) {
			return std::pow(10.0, -std::max(1, digits10 - 1));
		}

		// Calculate desired threshold
		double desired_threshold = std::pow(10.0, -(digits10 - margin));

		// Clamp to minimum representable threshold in double
		// (double can represent down to ~1e-308, use 1e-300 as safe minimum)
		constexpr double min_threshold = 1.0e-300;
		return (desired_threshold > min_threshold) ? desired_threshold : min_threshold;
	}

	/// <summary>
	/// Check if relative error between result and expected value is within threshold.
	/// Relative error is computed as |result - expected| / |expected|
	/// </summary>
	/// <typeparam name="Real">Arithmetic type to test</typeparam>
	/// <param name="result">Computed result</param>
	/// <param name="expected">Expected value</param>
	/// <param name="threshold">Error threshold (if 0, uses adaptive threshold)</param>
	/// <returns>True if error is within threshold</returns>
	template<typename Real>
	inline bool check_relative_error(const Real& result, const Real& expected, double threshold = 0.0) {
		// Use adaptive threshold if not specified
		if (threshold <= 0.0) {
			threshold = get_adaptive_threshold<Real>();
		}

		// Handle exact zero case by converting to double first
		double expected_val = double(expected);
		if (expected_val == 0.0) {
			// For zero expected, check absolute error instead
			double result_val = double(result);
			return std::abs(result_val) < threshold;
		}

		// Compute relative error using double precision for comparison
		// This avoids potential issues with adaptive-precision arithmetic
		double result_val = double(result);
		double rel_error = std::abs((result_val - expected_val) / expected_val);
		return rel_error < threshold;
	}

	/// <summary>
	/// Check if absolute error between result and expected value is within threshold.
	/// Absolute error is computed as |result - expected|
	/// </summary>
	/// <typeparam name="Real">Arithmetic type to test</typeparam>
	/// <param name="result">Computed result</param>
	/// <param name="expected">Expected value</param>
	/// <param name="threshold">Error threshold (if 0, uses adaptive threshold)</param>
	/// <returns>True if error is within threshold</returns>
	template<typename Real>
	inline bool check_absolute_error(const Real& result, const Real& expected, double threshold = 0.0) {
		// Use adaptive threshold if not specified
		if (threshold <= 0.0) {
			threshold = get_adaptive_threshold<Real>();
		}

		Real abs_error = abs(result - expected);
		return double(abs_error) < threshold;
	}

	/// <summary>
	/// Check if a value is mathematically exact (for cases like exp(0)=1, log(1)=0).
	/// Uses exact equality comparison.
	/// </summary>
	/// <typeparam name="Real">Arithmetic type to test</typeparam>
	/// <param name="result">Computed result</param>
	/// <param name="expected">Expected exact value</param>
	/// <returns>True if values are exactly equal</returns>
	template<typename Real>
	inline bool check_exact_value(const Real& result, const Real& expected) {
		return result == expected;
	}

	/// <summary>
	/// Report detailed error information for a failed test case.
	/// Shows the function, inputs, result, expected value, error magnitude, and threshold.
	/// </summary>
	/// <typeparam name="Real">Arithmetic type being tested</typeparam>
	/// <param name="function_name">Name of the function being tested (e.g., "exp", "sin")</param>
	/// <param name="input">Input value(s) as a string (e.g., "0.5" or "x=0.5, y=1.0")</param>
	/// <param name="result">Computed result</param>
	/// <param name="expected">Expected value</param>
	/// <param name="threshold">Threshold used for comparison</param>
	/// <param name="use_relative">True if relative error, false if absolute error</param>
	template<typename Real>
	void report_error_detail(const std::string& function_name,
	                         const std::string& input,
	                         const Real& result,
	                         const Real& expected,
	                         double threshold,
	                         bool use_relative = true) {

		auto old_precision = std::cerr.precision();
		std::cerr << std::setprecision(std::numeric_limits<Real>::max_digits10);

		std::cerr << "FAIL: " << function_name << "(" << input << ")\n";
		std::cerr << "  Expected: " << expected << "\n";
		std::cerr << "  Result:   " << result << "\n";

		if (use_relative) {
			Real rel_error = abs((result - expected) / expected);
			std::cerr << "  Relative error: " << double(rel_error) << "\n";
		} else {
			Real abs_error = abs(result - expected);
			std::cerr << "  Absolute error: " << double(abs_error) << "\n";
		}

		std::cerr << "  Threshold:      " << threshold << "\n";
		std::cerr << "  Type precision: " << std::numeric_limits<Real>::digits10 << " decimal digits\n";

		std::cerr << std::setprecision(old_precision);
	}

	/// <summary>
	/// Verify a mathematical identity holds within adaptive precision bounds.
	/// For example: exp(log(x)) == x, or sin²(x) + cos²(x) == 1
	/// </summary>
	/// <typeparam name="Real">Arithmetic type to test</typeparam>
	/// <param name="identity_name">Description of the identity (e.g., "exp(log(x)) == x")</param>
	/// <param name="lhs">Left-hand side of identity</param>
	/// <param name="rhs">Right-hand side of identity</param>
	/// <param name="threshold">Error threshold (if 0, uses adaptive threshold)</param>
	/// <param name="reportTestCases">Whether to report failures</param>
	/// <returns>1 if identity fails, 0 if it holds</returns>
	template<typename Real>
	inline int verify_identity(const std::string& identity_name,
	                           const Real& lhs,
	                           const Real& rhs,
	                           double threshold = 0.0,
	                           bool reportTestCases = true) {
		if (!check_relative_error(lhs, rhs, threshold)) {
			if (reportTestCases) {
				double actual_threshold = (threshold > 0.0) ? threshold : get_adaptive_threshold<Real>();
				report_error_detail(identity_name, "identity", lhs, rhs, actual_threshold, true);
			}
			return 1;
		}
		return 0;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///                    SPECIALIZED ERROR CHECKING STRATEGIES                          ///
	/////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// Error checking strategy for exact mathematical values.
	/// Examples: exp(0)=1, log(1)=0, pow(x,0)=1, sin(0)=0, cos(0)=1
	/// These should be represented exactly in any reasonable number system.
	/// </summary>
	template<typename Real>
	struct ExactValueStrategy {
		static bool check(const Real& result, const Real& expected) {
			return check_exact_value(result, expected);
		}

		static void report(const std::string& function_name,
		                   const std::string& input,
		                   const Real& result,
		                   const Real& expected) {
			std::cerr << "FAIL: " << function_name << "(" << input << ") - Expected exact value\n";
			std::cerr << "  Expected: " << expected << " (exact)\n";
			std::cerr << "  Result:   " << result << "\n";
		}
	};

	/// <summary>
	/// Error checking strategy for approximate values with adaptive precision.
	/// Used for most mathematical functions where the result is computed approximately.
	/// </summary>
	template<typename Real>
	struct ApproximateValueStrategy {
		static bool check(const Real& result, const Real& expected, double threshold = 0.0) {
			return check_relative_error(result, expected, threshold);
		}

		static void report(const std::string& function_name,
		                   const std::string& input,
		                   const Real& result,
		                   const Real& expected,
		                   double threshold = 0.0) {
			if (threshold <= 0.0) {
				threshold = get_adaptive_threshold<Real>();
			}
			report_error_detail(function_name, input, result, expected, threshold, true);
		}
	};

	/// <summary>
	/// Error checking strategy for mathematical identities.
	/// Examples: log(exp(x))=x, sin²(x)+cos²(x)=1, cosh²(x)-sinh²(x)=1
	/// Uses relative error with adaptive precision.
	/// </summary>
	template<typename Real>
	struct IdentityStrategy {
		static bool check(const Real& lhs, const Real& rhs, double threshold = 0.0) {
			return check_relative_error(lhs, rhs, threshold);
		}

		static void report(const std::string& identity_name,
		                   const Real& lhs,
		                   const Real& rhs,
		                   double threshold = 0.0) {
			if (threshold <= 0.0) {
				threshold = get_adaptive_threshold<Real>();
			}
			std::cerr << "FAIL: Identity violation: " << identity_name << "\n";
			std::cerr << "  LHS:       " << lhs << "\n";
			std::cerr << "  RHS:       " << rhs << "\n";
			Real rel_error = abs((lhs - rhs) / rhs);
			std::cerr << "  Rel Error: " << double(rel_error) << "\n";
			std::cerr << "  Threshold: " << threshold << "\n";
		}
	};

}} // namespace sw::universal
