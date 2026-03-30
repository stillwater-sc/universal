#pragma once
// steps_dbns.hpp: step-by-step arithmetic decomposition for double-base number system
//
// DBNS represents values as sign * 2^a * 3^b.
// Multiplication is trivial: add both exponent pairs.
// Division is also trivial: subtract both exponent pairs.
// Addition requires factoring out common powers and evaluating the residual.
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

struct DbnsComponents {
	bool sign;
	int exp_2;    // exponent of base 2
	int exp_3;    // exponent of base 3
	double value;

	std::string format_value() const {
		std::ostringstream ss;
		ss << (sign ? "-" : "+") << "2^" << exp_2 << " * 3^" << exp_3
		   << " = " << std::setprecision(10) << std::abs(value);
		return ss.str();
	}
};

// Approximate decomposition of a double into DBNS components
// Finds integer (a, b) minimizing |2^a * 3^b - |v||
inline DbnsComponents decompose_dbns(double v) {
	DbnsComponents c;
	c.sign = (v < 0.0);
	c.value = v;

	if (v == 0.0) {
		c.exp_2 = 0; c.exp_3 = 0;
		return c;
	}

	double av = std::abs(v);
	double log2_av = std::log2(av);
	double log2_3 = std::log2(3.0);

	// Search for best (a, b) pair
	double best_err = 1e30;
	int best_a = 0, best_b = 0;
	for (int b = -8; b <= 8; ++b) {
		double a_exact = log2_av - b * log2_3;
		int a = static_cast<int>(std::round(a_exact));
		double approx = std::ldexp(1.0, a) * std::pow(3.0, b);
		double err = std::abs(approx - av);
		if (err < best_err) {
			best_err = err;
			best_a = a;
			best_b = b;
		}
	}
	c.exp_2 = best_a;
	c.exp_3 = best_b;
	return c;
}

// Explain DBNS multiplication (trivial: add exponent pairs)
inline std::vector<StepDescription> explain_dbns_mul(double a_val, double b_val) {
	std::vector<StepDescription> steps;
	int step = 0;

	auto ca = decompose_dbns(a_val);
	auto cb = decompose_dbns(b_val);

	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decompose DBNS operands";
		std::ostringstream detail;
		detail << "A = " << ca.format_value() << "\n"
		       << "           B = " << cb.format_value();
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	bool result_sign = (ca.sign != cb.sign);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Determine sign (XOR)";
		s.detail = std::string(ca.sign ? "-" : "+") + " * "
		         + std::string(cb.sign ? "-" : "+") + " = "
		         + std::string(result_sign ? "-" : "+");
		steps.push_back(std::move(s));
	}

	int r_a = ca.exp_2 + cb.exp_2;
	int r_b = ca.exp_3 + cb.exp_3;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Add exponent pairs (multiplication is two integer additions!)";
		std::ostringstream detail;
		detail << "base-2 exponent: " << ca.exp_2 << " + " << cb.exp_2 << " = " << r_a << "\n"
		       << "           base-3 exponent: " << ca.exp_3 << " + " << cb.exp_3 << " = " << r_b << "\n"
		       << "           (two integer adds -- no multiplier needed, like LNS but denser grid)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	double result = (result_sign ? -1.0 : 1.0) * std::ldexp(1.0, r_a) * std::pow(3.0, r_b);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Result";
		std::ostringstream detail;
		detail << (result_sign ? "-" : "+") << "2^" << r_a << " * 3^" << r_b
		       << " = " << std::setprecision(10) << result;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Explain DBNS addition (complex: factor alignment)
inline std::vector<StepDescription> explain_dbns_add(
    double a_val, double b_val, bool is_subtraction = false)
{
	std::vector<StepDescription> steps;
	int step = 0;

	double b = is_subtraction ? -b_val : b_val;
	auto ca = decompose_dbns(a_val);
	auto cb = decompose_dbns(b);

	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decompose DBNS operands";
		std::ostringstream detail;
		detail << "A = " << ca.format_value() << "\n"
		       << "           B = " << cb.format_value();
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Factor out common powers
	int common_2 = std::min(ca.exp_2, cb.exp_2);
	int common_3 = std::min(ca.exp_3, cb.exp_3);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Factor out common powers";
		std::ostringstream detail;
		detail << "common factor: 2^" << common_2 << " * 3^" << common_3 << "\n"
		       << "           A residual: 2^" << (ca.exp_2 - common_2) << " * 3^" << (ca.exp_3 - common_3) << "\n"
		       << "           B residual: 2^" << (cb.exp_2 - common_2) << " * 3^" << (cb.exp_3 - common_3);
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Evaluate residuals and add
	double res_a = std::ldexp(1.0, ca.exp_2 - common_2) * std::pow(3.0, ca.exp_3 - common_3);
	double res_b = std::ldexp(1.0, cb.exp_2 - common_2) * std::pow(3.0, cb.exp_3 - common_3);
	if (ca.sign) res_a = -res_a;
	if (cb.sign) res_b = -res_b;
	double sum = res_a + res_b;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Add residuals";
		std::ostringstream detail;
		detail << std::setprecision(10) << res_a << " + " << res_b << " = " << sum << "\n"
		       << "           (requires evaluating residuals -- similar cost to LNS addition)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Re-encode sum as DBNS
	double result = sum * std::ldexp(1.0, common_2) * std::pow(3.0, common_3);
	auto cr = decompose_dbns(result);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Re-encode as DBNS";
		std::ostringstream detail;
		detail << "multiply by common factor: 2^" << common_2 << " * 3^" << common_3 << "\n"
		       << "           result: " << cr.format_value();
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Main dispatch
inline std::vector<StepDescription> explain_dbns(
    const Value& a, const Value& b, const std::string& op)
{
	if (op == "add") return explain_dbns_add(a.num, b.num);
	if (op == "sub") return explain_dbns_add(a.num, b.num, true);
	if (op == "mul") return explain_dbns_mul(a.num, b.num);
	if (op == "div") {
		// Division: subtract exponent pairs (trivial, like multiplication)
		auto ca = decompose_dbns(a.num);
		auto cb = decompose_dbns(b.num);
		std::vector<StepDescription> steps;
		int step = 0;
		{
			StepDescription s;
			s.step_number = ++step;
			s.label = "Decompose DBNS operands";
			std::ostringstream detail;
			detail << "A = " << ca.format_value() << "\n"
			       << "           B = " << cb.format_value();
			s.detail = detail.str();
			steps.push_back(std::move(s));
		}
		int r_a = ca.exp_2 - cb.exp_2;
		int r_b = ca.exp_3 - cb.exp_3;
		{
			StepDescription s;
			s.step_number = ++step;
			s.label = "Subtract exponent pairs (division is two integer subtractions!)";
			std::ostringstream detail;
			detail << "base-2: " << ca.exp_2 << " - " << cb.exp_2 << " = " << r_a << "\n"
			       << "           base-3: " << ca.exp_3 << " - " << cb.exp_3 << " = " << r_b;
			s.detail = detail.str();
			steps.push_back(std::move(s));
		}
		bool result_sign = (ca.sign != cb.sign);
		double result = (result_sign ? -1.0 : 1.0) * std::ldexp(1.0, r_a) * std::pow(3.0, r_b);
		{
			StepDescription s;
			s.step_number = ++step;
			s.label = "Result";
			std::ostringstream detail;
			detail << (result_sign ? "-" : "+") << "2^" << r_a << " * 3^" << r_b
			       << " = " << std::setprecision(10) << result;
			s.detail = detail.str();
			steps.push_back(std::move(s));
		}
		return steps;
	}
	return {};
}

}} // namespace sw::ucalc
