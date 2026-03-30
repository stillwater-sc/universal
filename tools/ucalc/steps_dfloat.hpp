#pragma once
// steps_dfloat.hpp: step-by-step arithmetic decomposition for decimal float types
//
// Decimal IEEE arithmetic operates in base 10:
//   Value = (-1)^sign * coefficient * 10^exponent
// where coefficient has up to ndigits significant decimal digits.
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

// Decimal float component decomposition
struct DfloatComponents {
	bool sign;
	long long coefficient;   // decimal integer coefficient
	int exponent;            // decimal exponent (value = coeff * 10^exp)
	int ndigits;             // max significant decimal digits

	std::string format_value() const {
		std::ostringstream ss;
		ss << (sign ? "-" : "+") << coefficient << " * 10^" << exponent;
		return ss.str();
	}
};

// Decompose a double into decimal float components
inline DfloatComponents decompose_dfloat(double v, int ndigits) {
	DfloatComponents c;
	c.ndigits = ndigits;
	c.sign = (v < 0.0);

	if (v == 0.0) {
		c.coefficient = 0;
		c.exponent = 0;
		return c;
	}

	double av = std::abs(v);
	// Find the decimal exponent
	int dec_exp = static_cast<int>(std::floor(std::log10(av)));
	// Normalize coefficient to ndigits digits
	// coefficient = round(av * 10^(ndigits - 1 - dec_exp))
	int shift = ndigits - 1 - dec_exp;
	c.coefficient = static_cast<long long>(std::round(av * std::pow(10.0, shift)));
	c.exponent = -shift;

	// Remove trailing zeros from coefficient
	while (c.coefficient != 0 && c.coefficient % 10 == 0) {
		c.coefficient /= 10;
		c.exponent++;
	}

	return c;
}

// Explain decimal float addition step by step
inline std::vector<StepDescription> explain_dfloat_add(
    double a_val, double b_val, int ndigits, bool is_subtraction = false)
{
	std::vector<StepDescription> steps;
	int step = 0;

	double b = is_subtraction ? -b_val : b_val;
	auto ca = decompose_dfloat(a_val, ndigits);
	auto cb = decompose_dfloat(b, ndigits);

	// Step 1: Decompose
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decompose decimal operands";
		std::ostringstream detail;
		detail << "A = " << ca.format_value()
		       << " (" << ndigits << " significant digits)\n"
		       << "           B = " << cb.format_value();
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 2: Align decimal exponents
	int common_exp = std::min(ca.exponent, cb.exponent);
	long long coeff_a = ca.coefficient;
	long long coeff_b = cb.coefficient;
	int shift_a = ca.exponent - common_exp;
	int shift_b = cb.exponent - common_exp;
	for (int i = 0; i < shift_a; ++i) coeff_a *= 10;
	for (int i = 0; i < shift_b; ++i) coeff_b *= 10;
	if (ca.sign) coeff_a = -coeff_a;
	if (cb.sign) coeff_b = -coeff_b;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Align decimal exponents";
		std::ostringstream detail;
		if (shift_a == 0 && shift_b == 0) {
			detail << "exponents equal (10^" << common_exp << "), no shift needed";
		} else if (shift_a > 0) {
			detail << "scale A coefficient up by 10^" << shift_a
			       << " -> " << coeff_a << " * 10^" << common_exp;
		} else {
			detail << "scale B coefficient up by 10^" << shift_b
			       << " -> " << coeff_b << " * 10^" << common_exp;
		}
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 3: Add/subtract coefficients
	long long result_coeff = coeff_a + coeff_b;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = is_subtraction ? "Subtract coefficients" : "Add coefficients";
		std::ostringstream detail;
		detail << coeff_a << (coeff_b >= 0 ? " + " : " - ") << std::abs(coeff_b)
		       << " = " << result_coeff
		       << " (decimal integer arithmetic)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 4: Normalize to ndigits
	bool result_sign = (result_coeff < 0);
	long long abs_coeff = std::abs(result_coeff);
	int result_exp = common_exp;
	// Count digits
	int digits = 0;
	{
		long long tmp = abs_coeff;
		if (tmp == 0) digits = 1;
		while (tmp > 0) { digits++; tmp /= 10; }
	}
	std::string norm_action;
	if (digits > ndigits) {
		// Too many digits: round by removing excess
		int excess = digits - ndigits;
		long long divisor = 1;
		for (int i = 0; i < excess; ++i) divisor *= 10;
		abs_coeff = (abs_coeff + divisor / 2) / divisor; // round-half-up
		result_exp += excess;
		norm_action = "truncate " + std::to_string(excess) + " excess digit(s)";
	} else if (digits < ndigits && abs_coeff != 0) {
		norm_action = "coefficient has " + std::to_string(digits) + " digit(s), fits in " + std::to_string(ndigits);
	} else {
		norm_action = "already " + std::to_string(ndigits) + " digits, no normalization needed";
	}
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Normalize coefficient";
		std::ostringstream detail;
		detail << norm_action << "\n"
		       << "           result: " << (result_sign ? "-" : "+") << abs_coeff
		       << " * 10^" << result_exp;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Final value
	double result = (result_sign ? -1.0 : 1.0) * abs_coeff * std::pow(10.0, result_exp);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Result";
		std::ostringstream detail;
		detail << std::setprecision(12) << result;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Explain decimal float multiplication step by step
inline std::vector<StepDescription> explain_dfloat_mul(
    double a_val, double b_val, int ndigits)
{
	std::vector<StepDescription> steps;
	int step = 0;

	auto ca = decompose_dfloat(a_val, ndigits);
	auto cb = decompose_dfloat(b_val, ndigits);

	// Step 1: Decompose
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decompose decimal operands";
		std::ostringstream detail;
		detail << "A = " << ca.format_value() << "\n"
		       << "           B = " << cb.format_value();
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 2: Determine sign
	bool result_sign = (ca.sign != cb.sign);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Determine sign";
		s.detail = std::string(ca.sign ? "-" : "+") + " * "
		         + std::string(cb.sign ? "-" : "+") + " = "
		         + std::string(result_sign ? "-" : "+");
		steps.push_back(std::move(s));
	}

	// Step 3: Add exponents
	int result_exp = ca.exponent + cb.exponent;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Add decimal exponents";
		std::ostringstream detail;
		detail << ca.exponent << " + " << cb.exponent << " = " << result_exp;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 4: Multiply coefficients
	long long product = ca.coefficient * cb.coefficient;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Multiply decimal coefficients";
		std::ostringstream detail;
		detail << ca.coefficient << " * " << cb.coefficient << " = " << product;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Normalize to ndigits
	int digits = 0;
	{
		long long tmp = product;
		if (tmp == 0) digits = 1;
		while (tmp > 0) { digits++; tmp /= 10; }
	}
	if (digits > ndigits) {
		int excess = digits - ndigits;
		long long divisor = 1;
		for (int i = 0; i < excess; ++i) divisor *= 10;
		product = (product + divisor / 2) / divisor;
		result_exp += excess;
	}
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Normalize to " + std::to_string(ndigits) + " digits";
		std::ostringstream detail;
		detail << (result_sign ? "-" : "+") << product << " * 10^" << result_exp;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Main dispatch for dfloat steps
inline std::vector<StepDescription> explain_dfloat(
    const Value& a, const Value& b, const std::string& op, int ndigits)
{
	if (op == "add") return explain_dfloat_add(a.num, b.num, ndigits);
	if (op == "sub") return explain_dfloat_add(a.num, b.num, ndigits, true);
	if (op == "mul") return explain_dfloat_mul(a.num, b.num, ndigits);
	return {};
}

}} // namespace sw::ucalc
