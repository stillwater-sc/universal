#pragma once
// steps_dfloat.hpp: step-by-step arithmetic decomposition for decimal float types
//
// Decimal IEEE arithmetic operates in base 10:
//   Value = (-1)^sign * coefficient * 10^exponent
// where coefficient has up to ndigits significant decimal digits.
//
// Note: decomposition uses double approximation of the dfloat value
// (the same double interchange as all ucalc commands). This is adequate
// for educational display but may not match the native dfloat encoding
// exactly for values at the edge of double's precision.
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
// Uses double for coefficient to avoid overflow for decimal64 (ndigits=16)
// where coefficient^2 exceeds int64 range.
struct DfloatComponents {
	bool sign;
	double coefficient;      // decimal coefficient (integer-valued double)
	int exponent;            // decimal exponent (value = coeff * 10^exp)
	int ndigits;             // max significant decimal digits

	std::string format_value() const {
		std::ostringstream ss;
		ss << (sign ? "-" : "+")
		   << std::setprecision(0) << std::fixed << coefficient
		   << std::defaultfloat << " * 10^" << exponent;
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
	int dec_exp = static_cast<int>(std::floor(std::log10(av)));
	int shift = ndigits - 1 - dec_exp;
	c.coefficient = std::round(av * std::pow(10.0, shift));
	c.exponent = -shift;

	// Remove trailing zeros from coefficient
	while (c.coefficient > 0 && std::fmod(c.coefficient, 10.0) == 0.0) {
		c.coefficient /= 10.0;
		c.exponent++;
	}

	return c;
}

// Normalize a coefficient to ndigits, handling post-round carry
inline void normalize_coefficient(double& abs_coeff, int& result_exp, int ndigits, std::string& action) {
	int digits = 0;
	{
		double tmp = abs_coeff;
		if (tmp == 0.0) digits = 1;
		while (tmp >= 1.0) { digits++; tmp /= 10.0; }
	}
	if (digits > ndigits) {
		int excess = digits - ndigits;
		double divisor = std::pow(10.0, excess);
		abs_coeff = std::round(abs_coeff / divisor);
		result_exp += excess;
		action = "truncate " + std::to_string(excess) + " excess digit(s)";
		// Handle carry from rounding (e.g., 99995 -> 10000 for 4 digits)
		double limit = std::pow(10.0, ndigits);
		if (abs_coeff >= limit) {
			abs_coeff = std::round(abs_coeff / 10.0);
			result_exp++;
			action += " + carry";
		}
	} else if (digits < ndigits && abs_coeff != 0.0) {
		action = "coefficient has " + std::to_string(digits) + " digit(s), fits in " + std::to_string(ndigits);
	} else {
		action = "already " + std::to_string(ndigits) + " digits, no normalization needed";
	}
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
	double coeff_a = ca.coefficient;
	double coeff_b = cb.coefficient;
	int shift_a = ca.exponent - common_exp;
	int shift_b = cb.exponent - common_exp;
	coeff_a *= std::pow(10.0, shift_a);
	coeff_b *= std::pow(10.0, shift_b);
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
			       << " -> " << std::setprecision(0) << std::fixed << coeff_a
			       << std::defaultfloat << " * 10^" << common_exp;
		} else {
			detail << "scale B coefficient up by 10^" << shift_b
			       << " -> " << std::setprecision(0) << std::fixed << coeff_b
			       << std::defaultfloat << " * 10^" << common_exp;
		}
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 3: Add/subtract coefficients
	double result_coeff = coeff_a + coeff_b;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = is_subtraction ? "Subtract coefficients" : "Add coefficients";
		std::ostringstream detail;
		detail << std::setprecision(0) << std::fixed
		       << coeff_a << (coeff_b >= 0 ? " + " : " - ") << std::abs(coeff_b)
		       << " = " << result_coeff
		       << std::defaultfloat << " (decimal integer arithmetic)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 4: Normalize to ndigits
	bool result_sign = (result_coeff < 0.0);
	double abs_coeff = std::abs(result_coeff);
	int result_exp = common_exp;
	std::string norm_action;
	normalize_coefficient(abs_coeff, result_exp, ndigits, norm_action);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Normalize coefficient";
		std::ostringstream detail;
		detail << norm_action << "\n"
		       << "           result: " << (result_sign ? "-" : "+")
		       << std::setprecision(0) << std::fixed << abs_coeff
		       << std::defaultfloat << " * 10^" << result_exp;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Final value
	double result = (result_sign ? -1.0 : 1.0) * abs_coeff * std::pow(10.0, result_exp);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Result";
		s.detail = std::to_string(result);
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
	double product = ca.coefficient * cb.coefficient;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Multiply decimal coefficients";
		std::ostringstream detail;
		detail << std::setprecision(0) << std::fixed
		       << ca.coefficient << " * " << cb.coefficient << " = " << product
		       << std::defaultfloat;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Normalize to ndigits
	std::string norm_action;
	normalize_coefficient(product, result_exp, ndigits, norm_action);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Normalize to " + std::to_string(ndigits) + " digits";
		std::ostringstream detail;
		detail << (result_sign ? "-" : "+")
		       << std::setprecision(0) << std::fixed << product
		       << std::defaultfloat << " * 10^" << result_exp;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Explain decimal float division step by step
inline std::vector<StepDescription> explain_dfloat_div(
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

	// Step 2: Check for division by zero
	if (cb.coefficient == 0.0) {
		StepDescription s;
		s.step_number = ++step;
		s.label = "Division by zero";
		s.detail = "divisor coefficient is zero";
		steps.push_back(std::move(s));
		return steps;
	}

	// Step 3: Determine sign
	bool result_sign = (ca.sign != cb.sign);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Determine sign";
		s.detail = std::string(ca.sign ? "-" : "+") + " / "
		         + std::string(cb.sign ? "-" : "+") + " = "
		         + std::string(result_sign ? "-" : "+");
		steps.push_back(std::move(s));
	}

	// Step 4: Subtract exponents
	int result_exp = ca.exponent - cb.exponent;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Subtract decimal exponents";
		std::ostringstream detail;
		detail << ca.exponent << " - " << cb.exponent << " = " << result_exp;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Divide coefficients (scale dividend up to get ndigits precision)
	double scaled_dividend = ca.coefficient * std::pow(10.0, ndigits);
	double quotient = std::round(scaled_dividend / cb.coefficient);
	result_exp -= ndigits;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Divide decimal coefficients";
		std::ostringstream detail;
		detail << std::setprecision(0) << std::fixed
		       << ca.coefficient << " / " << cb.coefficient
		       << " (scale by 10^" << ndigits << " for precision)\n"
		       << "           = " << quotient
		       << std::defaultfloat << " * 10^" << result_exp;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 6: Normalize
	std::string norm_action;
	normalize_coefficient(quotient, result_exp, ndigits, norm_action);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Normalize to " + std::to_string(ndigits) + " digits";
		std::ostringstream detail;
		detail << (result_sign ? "-" : "+")
		       << std::setprecision(0) << std::fixed << quotient
		       << std::defaultfloat << " * 10^" << result_exp;
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
	if (op == "div") return explain_dfloat_div(a.num, b.num, ndigits);
	return {};
}

}} // namespace sw::ucalc
