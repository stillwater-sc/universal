#pragma once
// steps_lns.hpp: step-by-step arithmetic decomposition for logarithmic number system
//
// LNS stores values as sign + fixed-point log2(|value|).
// Multiplication is trivial: add the log-values.
// Addition requires the Gaussian logarithm: log2(1 + 2^d).
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
#include <limits>

#include "type_dispatch.hpp"

namespace sw { namespace ucalc {

struct LnsComponents {
	bool sign;
	double log_value;   // log2(|value|)
	double value;       // the actual value

	std::string format_value() const {
		std::ostringstream ss;
		ss << (sign ? "-" : "+") << "2^" << std::setprecision(6) << log_value
		   << " = " << std::setprecision(10) << std::abs(value);
		return ss.str();
	}
};

inline LnsComponents decompose_lns(double v) {
	LnsComponents c;
	c.sign = (v < 0.0);
	c.value = v;
	if (v == 0.0) {
		c.log_value = -std::numeric_limits<double>::infinity();
	} else {
		c.log_value = std::log2(std::abs(v));
	}
	return c;
}

// Explain LNS multiplication (trivial: add log-values)
inline std::vector<StepDescription> explain_lns_mul(double a_val, double b_val) {
	std::vector<StepDescription> steps;
	int step = 0;

	auto ca = decompose_lns(a_val);
	auto cb = decompose_lns(b_val);

	// Step 1: Decompose
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decompose LNS operands";
		std::ostringstream detail;
		detail << "A = " << ca.format_value() << "\n"
		       << "           B = " << cb.format_value();
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 2: XOR signs
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

	// Step 3: Add log-values (THIS IS THE KEY INSIGHT: multiplication = addition in log domain)
	double result_log = ca.log_value + cb.log_value;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Add log-values (multiplication is FREE in LNS!)";
		std::ostringstream detail;
		detail << "log2(|A*B|) = log2(|A|) + log2(|B|)\n"
		       << "           = " << std::setprecision(6) << ca.log_value
		       << " + " << cb.log_value
		       << " = " << result_log << "\n"
		       << "           (just one fixed-point addition -- no multiplier hardware needed)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 4: Result
	double result = std::ldexp(1.0, 0) * std::pow(2.0, result_log);
	if (result_sign) result = -result;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Result";
		std::ostringstream detail;
		detail << (result_sign ? "-" : "+") << "2^" << std::setprecision(6) << result_log
		       << " = " << std::setprecision(10) << result;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Explain LNS addition (the hard case: requires Gaussian logarithm)
inline std::vector<StepDescription> explain_lns_add(
    double a_val, double b_val, bool is_subtraction = false)
{
	std::vector<StepDescription> steps;
	int step = 0;

	double b = is_subtraction ? -b_val : b_val;
	auto ca = decompose_lns(a_val);
	auto cb = decompose_lns(b);

	// Step 1: Decompose
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decompose LNS operands";
		std::ostringstream detail;
		detail << "A = " << ca.format_value() << "\n"
		       << "           B = " << cb.format_value();
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Fast path: both zero avoids -inf - -inf -> NaN
	if (a_val == 0.0 && b == 0.0) {
		StepDescription s;
		s.step_number = ++step;
		s.label = "Result";
		s.detail = "both operands zero -> 0";
		steps.push_back(std::move(s));
		return steps;
	}

	// Ensure |A| >= |B| for the formula
	bool swapped = false;
	if (std::abs(b) > std::abs(a_val)) {
		std::swap(ca, cb);
		swapped = true;
	}

	// Step 2: Compute log-difference
	double d = cb.log_value - ca.log_value;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Compute log-difference";
		std::ostringstream detail;
		detail << "d = log2(|B|) - log2(|A|) = "
		       << std::setprecision(6) << cb.log_value << " - " << ca.log_value
		       << " = " << d;
		if (swapped) detail << " (swapped so |A| >= |B|)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 3: Gaussian logarithm function
	// For same-sign: log2(|A| + |B|) = log2(|A|) + log2(1 + 2^d)
	// For diff-sign: log2(|A| - |B|) = log2(|A|) + log2(1 - 2^d)
	bool same_sign = (ca.sign == cb.sign);
	double gauss;
	if (same_sign) {
		gauss = std::log2(1.0 + std::pow(2.0, d));
	} else {
		gauss = std::log2(1.0 - std::pow(2.0, d));
	}
	{
		StepDescription s;
		s.step_number = ++step;
		if (same_sign) {
			s.label = "Gaussian logarithm: log2(1 + 2^d)";
		} else {
			s.label = "Gaussian logarithm: log2(1 - 2^d)";
		}
		std::ostringstream detail;
		if (same_sign) {
			detail << "log2(1 + 2^" << std::setprecision(4) << d << ")"
			       << " = " << std::setprecision(6) << gauss << "\n"
			       << "           (in hardware: table lookup or CORDIC -- this is the COST of LNS addition)\n"
			       << "           (current library: uses double conversion fallback)";
		} else {
			detail << "log2(1 - 2^" << std::setprecision(4) << d << ")"
			       << " = " << std::setprecision(6) << gauss << "\n"
			       << "           (subtraction case -- even harder, loses precision when d -> 0)\n"
			       << "           (current library: uses double conversion fallback)";
		}
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 4: Add Gaussian logarithm to log(|A|)
	double result_log = ca.log_value + gauss;
	bool result_sign = ca.sign;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Add to larger operand's log";
		std::ostringstream detail;
		detail << "log2(|result|) = log2(|A|) + gaussian\n"
		       << "           = " << std::setprecision(6) << ca.log_value
		       << " + " << gauss << " = " << result_log;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Result
	double result = std::pow(2.0, result_log);
	if (result_sign) result = -result;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Result";
		std::ostringstream detail;
		detail << (result_sign ? "-" : "+") << "2^" << std::setprecision(6) << result_log
		       << " = " << std::setprecision(10) << result;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Explain LNS division (trivial: subtract log-values)
inline std::vector<StepDescription> explain_lns_div(double a_val, double b_val) {
	std::vector<StepDescription> steps;
	int step = 0;

	auto ca = decompose_lns(a_val);
	auto cb = decompose_lns(b_val);

	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decompose LNS operands";
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
		s.detail = std::string(ca.sign ? "-" : "+") + " / "
		         + std::string(cb.sign ? "-" : "+") + " = "
		         + std::string(result_sign ? "-" : "+");
		steps.push_back(std::move(s));
	}

	double result_log = ca.log_value - cb.log_value;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Subtract log-values (division is FREE in LNS!)";
		std::ostringstream detail;
		detail << "log2(|A/B|) = log2(|A|) - log2(|B|)\n"
		       << "           = " << std::setprecision(6) << ca.log_value
		       << " - " << cb.log_value << " = " << result_log;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	double result = std::pow(2.0, result_log);
	if (result_sign) result = -result;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Result";
		std::ostringstream detail;
		detail << (result_sign ? "-" : "+") << "2^" << std::setprecision(6) << result_log
		       << " = " << std::setprecision(10) << result;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Main dispatch
inline std::vector<StepDescription> explain_lns(
    const Value& a, const Value& b, const std::string& op)
{
	if (op == "add") return explain_lns_add(a.num, b.num);
	if (op == "sub") return explain_lns_add(a.num, b.num, true);
	if (op == "mul") return explain_lns_mul(a.num, b.num);
	if (op == "div") return explain_lns_div(a.num, b.num);
	return {};
}

}} // namespace sw::ucalc
