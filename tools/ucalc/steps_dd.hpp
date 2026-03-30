#pragma once
// steps_dd.hpp: step-by-step arithmetic decomposition for double-double/quad-double
//
// Multi-component types use error-free transformations:
//   two_sum(a,b) -> (s, e) where s + e = a + b exactly
//   two_prod(a,b) -> (p, e) where p + e = a * b exactly
// Each operation captures its rounding error in a lower limb.
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

// Error-free transformations (the building blocks)
inline std::pair<double, double> two_sum_eft(double a, double b) {
	double s = a + b;
	double v = s - a;
	double e = (a - (s - v)) + (b - v);
	return { s, e };
}

inline std::pair<double, double> two_prod_eft(double a, double b) {
	double p = a * b;
	double e = std::fma(a, b, -p);
	return { p, e };
}

// Explain dd addition step by step
inline std::vector<StepDescription> explain_dd_add(double a_val, double b_val) {
	std::vector<StepDescription> steps;
	int step = 0;

	// Decompose: for dd, a = a_hi + a_lo. Since we only have the double
	// approximation, a_hi = a_val, a_lo = 0. For actual dd values the
	// lo limb carries the error from previous operations.
	double a_hi = a_val, a_lo = 0.0;
	double b_hi = b_val, b_lo = 0.0;

	// Step 1: Decompose
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decompose double-double operands";
		std::ostringstream detail;
		detail << std::setprecision(17)
		       << "A = (hi: " << a_hi << ", lo: " << a_lo << ")\n"
		       << "           B = (hi: " << b_hi << ", lo: " << b_lo << ")";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 2: two_sum on the high limbs
	auto [s_hi, e_hi] = two_sum_eft(a_hi, b_hi);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "two_sum(a.hi, b.hi) -- error-free transformation";
		std::ostringstream detail;
		detail << std::setprecision(17)
		       << "two_sum(" << a_hi << ", " << b_hi << ")\n"
		       << "           sum = " << s_hi << "\n"
		       << "           err = " << e_hi << "\n"
		       << "           (sum + err = a.hi + b.hi EXACTLY -- no rounding loss!)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 3: Add error terms
	double e_total = e_hi + a_lo + b_lo;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Accumulate error terms";
		std::ostringstream detail;
		detail << std::setprecision(17)
		       << "e_total = two_sum_error + a.lo + b.lo\n"
		       << "        = " << e_hi << " + " << a_lo << " + " << b_lo
		       << " = " << e_total;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 4: Renormalize (two_sum)
	auto [r_hi, r_lo] = two_sum_eft(s_hi, e_total);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Renormalize with two_sum";
		std::ostringstream detail;
		detail << std::setprecision(17)
		       << "two_sum(" << s_hi << ", " << e_total << ")\n"
		       << "           result.hi = " << r_hi << "\n"
		       << "           result.lo = " << r_lo << "\n"
		       << "           (restores non-overlapping property)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Result
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Result";
		std::ostringstream detail;
		detail << std::setprecision(17)
		       << "dd = (hi: " << r_hi << ", lo: " << r_lo << ")\n"
		       << "           value ~= " << std::setprecision(32) << (r_hi + r_lo);
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Explain dd multiplication step by step
inline std::vector<StepDescription> explain_dd_mul(double a_val, double b_val) {
	std::vector<StepDescription> steps;
	int step = 0;

	double a_hi = a_val, a_lo = 0.0;
	double b_hi = b_val, b_lo = 0.0;

	// Step 1: Decompose
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decompose double-double operands";
		std::ostringstream detail;
		detail << std::setprecision(17)
		       << "A = (hi: " << a_hi << ", lo: " << a_lo << ")\n"
		       << "           B = (hi: " << b_hi << ", lo: " << b_lo << ")";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 2: two_prod on the high limbs
	auto [p_hi, e_hi] = two_prod_eft(a_hi, b_hi);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "two_prod(a.hi, b.hi) -- error-free transformation via FMA";
		std::ostringstream detail;
		detail << std::setprecision(17)
		       << "two_prod(" << a_hi << ", " << b_hi << ")\n"
		       << "           product = " << p_hi << "\n"
		       << "           err     = " << e_hi << " (captured by FMA)\n"
		       << "           (product + err = a.hi * b.hi EXACTLY)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 3: Cross terms
	double cross = a_hi * b_lo + a_lo * b_hi;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Cross terms: a.hi*b.lo + a.lo*b.hi";
		std::ostringstream detail;
		detail << std::setprecision(17)
		       << a_hi << " * " << b_lo << " + " << a_lo << " * " << b_hi
		       << " = " << cross;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 4: Accumulate errors
	double e_total = e_hi + cross;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Accumulate error terms";
		std::ostringstream detail;
		detail << std::setprecision(17)
		       << "e_total = two_prod_error + cross\n"
		       << "        = " << e_hi << " + " << cross << " = " << e_total;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Renormalize
	auto [r_hi, r_lo] = two_sum_eft(p_hi, e_total);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Renormalize with two_sum";
		std::ostringstream detail;
		detail << std::setprecision(17)
		       << "result.hi = " << r_hi << "\n"
		       << "           result.lo = " << r_lo;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 6: Result
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Result";
		std::ostringstream detail;
		detail << std::setprecision(17)
		       << "dd = (hi: " << r_hi << ", lo: " << r_lo << ")\n"
		       << "           value ~= " << std::setprecision(32) << (r_hi + r_lo);
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Main dispatch
inline std::vector<StepDescription> explain_dd(
    const Value& a, const Value& b, const std::string& op)
{
	if (op == "add") return explain_dd_add(a.num, b.num);
	if (op == "sub") return explain_dd_add(a.num, -b.num);
	if (op == "mul") return explain_dd_mul(a.num, b.num);
	return {};
}

}} // namespace sw::ucalc
