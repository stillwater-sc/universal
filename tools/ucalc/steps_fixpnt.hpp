#pragma once
// steps_fixpnt.hpp: step-by-step arithmetic decomposition for fixed-point types
//
// Fixed-point arithmetic is simpler than floating-point:
//   - Radix point is in a fixed position (no alignment needed)
//   - Addition/subtraction is integer arithmetic on the underlying encoding
//   - Overflow behavior depends on the arithmetic mode (Modulo vs Saturating)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "type_dispatch.hpp"

namespace sw { namespace ucalc {

// Explain binary fixed-point addition step by step
inline std::vector<StepDescription> explain_fixpnt_add(
    double a_val, double b_val, int nbits, int rbits,
    const std::string& arith_mode, bool is_subtraction = false)
{
	std::vector<StepDescription> steps;
	int step = 0;
	int ibits = nbits - rbits;

	double b = is_subtraction ? -b_val : b_val;

	// Step 1: Show format
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Fixed-point format";
		std::ostringstream detail;
		detail << "fixpnt<" << nbits << "," << rbits << ">: "
		       << ibits << " integer bits, " << rbits << " fraction bits\n"
		       << "           radix point at bit " << rbits
		       << ", resolution = 2^-" << rbits
		       << " = " << std::setprecision(10) << std::ldexp(1.0, -rbits) << "\n"
		       << "           overflow mode: " << arith_mode;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 2: Show operands as fixed-point integers
	long long a_fixed = static_cast<long long>(std::round(a_val * std::ldexp(1.0, rbits)));
	long long b_fixed = static_cast<long long>(std::round(b * std::ldexp(1.0, rbits)));
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Convert to fixed-point integers";
		std::ostringstream detail;
		detail << "A = " << std::setprecision(10) << a_val
		       << " -> " << a_fixed << " (x 2^" << rbits << ")\n"
		       << "           B = " << std::setprecision(10) << b
		       << " -> " << b_fixed << " (x 2^" << rbits << ")";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 3: Integer arithmetic
	long long result_fixed = a_fixed + b_fixed;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = is_subtraction ? "Subtract integers" : "Add integers";
		std::ostringstream detail;
		detail << a_fixed << (b_fixed >= 0 ? " + " : " - ") << std::abs(b_fixed)
		       << " = " << result_fixed;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 4: Check overflow (use unsigned arithmetic to avoid UB for nbits >= 64)
	long long max_val, min_val;
	if (nbits >= 64) {
		max_val = std::numeric_limits<long long>::max();
		min_val = std::numeric_limits<long long>::min();
	} else {
		uint64_t one = 1ULL;
		max_val = static_cast<long long>((one << (nbits - 1)) - 1);
		min_val = -static_cast<long long>(one << (nbits - 1));
	}
	bool overflow = (result_fixed > max_val || result_fixed < min_val);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Check overflow";
		std::ostringstream detail;
		if (overflow) {
			detail << "OVERFLOW: " << result_fixed << " outside [" << min_val << ", " << max_val << "]\n"
			       << "           " << arith_mode << " arithmetic: ";
			if (arith_mode == "Modulo" && nbits < 64) {
				uint64_t range = 1ULL << nbits;
				long long wrapped = static_cast<long long>(
				    (static_cast<uint64_t>(result_fixed - min_val) % range));
				result_fixed = wrapped + min_val;
				detail << "wraps to " << result_fixed;
			} else {
				// Saturating: clamp
				result_fixed = (result_fixed > max_val) ? max_val : min_val;
				detail << "clamps to " << result_fixed;
			}
		} else {
			detail << "no overflow (" << result_fixed << " in [" << min_val << ", " << max_val << "])";
		}
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Convert back to real value
	double result = result_fixed * std::ldexp(1.0, -rbits);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Convert to real value";
		std::ostringstream detail;
		detail << result_fixed << " * 2^-" << rbits
		       << " = " << std::setprecision(12) << result;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Explain binary fixed-point multiplication
inline std::vector<StepDescription> explain_fixpnt_mul(
    double a_val, double b_val, int nbits, int rbits,
    const std::string& arith_mode)
{
	std::vector<StepDescription> steps;
	int step = 0;

	// Step 1: Show format
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Fixed-point format";
		std::ostringstream detail;
		detail << "fixpnt<" << nbits << "," << rbits << ">: "
		       << "resolution = 2^-" << rbits
		       << ", overflow mode: " << arith_mode;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 2: Convert to fixed-point
	long long a_fixed = static_cast<long long>(std::round(a_val * std::ldexp(1.0, rbits)));
	long long b_fixed = static_cast<long long>(std::round(b_val * std::ldexp(1.0, rbits)));
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Convert to fixed-point integers";
		std::ostringstream detail;
		detail << "A = " << a_fixed << ", B = " << b_fixed
		       << " (both x 2^" << rbits << ")";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 3: Multiply (result has 2*rbits fraction bits)
	// Use wider type to avoid signed overflow UB
	long long product;
#ifdef __SIZEOF_INT128__
	__int128 wide = static_cast<__int128>(a_fixed) * static_cast<__int128>(b_fixed);
	product = static_cast<long long>(wide);
#else
	// Fallback: compute via double (loses precision for large values)
	product = static_cast<long long>(static_cast<double>(a_fixed) * static_cast<double>(b_fixed));
#endif
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Multiply integers";
		std::ostringstream detail;
		detail << a_fixed << " * " << b_fixed << " = " << product
		       << " (has " << (2 * rbits) << " fraction bits, need " << rbits << ")";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 4: Shift right by rbits (discard extra fraction bits)
#ifdef __SIZEOF_INT128__
	long long result_fixed = static_cast<long long>(wide >> rbits);
#else
	long long result_fixed = product >> rbits;
#endif
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Shift right to realign radix point";
		std::ostringstream detail;
		detail << product << " >> " << rbits << " = " << result_fixed
		       << " (truncates " << rbits << " fraction bits)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Result
	double result = result_fixed * std::ldexp(1.0, -rbits);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Result";
		std::ostringstream detail;
		detail << result_fixed << " * 2^-" << rbits
		       << " = " << std::setprecision(12) << result;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Main dispatch for fixpnt steps
inline std::vector<StepDescription> explain_fixpnt(
    const Value& a, const Value& b, const std::string& op,
    int nbits, int rbits, const std::string& arith_mode)
{
	if (op == "add") return explain_fixpnt_add(a.num, b.num, nbits, rbits, arith_mode);
	if (op == "sub") return explain_fixpnt_add(a.num, b.num, nbits, rbits, arith_mode, true);
	if (op == "mul") return explain_fixpnt_mul(a.num, b.num, nbits, rbits, arith_mode);
	return {};
}

}} // namespace sw::ucalc
