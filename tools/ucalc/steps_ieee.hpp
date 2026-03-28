#pragma once
// steps_ieee.hpp: step-by-step arithmetic decomposition for binary IEEE types
//
// Decomposes add/sub/mul into the canonical IEEE-754 stages:
//   Add/Sub: decompose operands, align exponents, add/subtract significands,
//            normalize, round
//   Mul: decompose operands, add exponents, multiply significands,
//        normalize, round
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
#include <cstdint>

#include "type_dispatch.hpp"

namespace sw { namespace ucalc {

// Decompose a double into sign, unbiased exponent, and significand bits string
struct IEEEComponents {
	bool sign;
	int exponent;           // unbiased
	double significand;     // 1.xxxx (normalized), or 0.xxxx (subnormal)
	std::string sig_binary; // binary string of significand, e.g. "1.10100..."
	int precision_bits;     // number of significand bits (including hidden 1)

	std::string format_value() const {
		std::ostringstream ss;
		ss << (sign ? "-" : "+") << sig_binary << " * 2^" << exponent;
		return ss.str();
	}
};

// Decompose a value into IEEE components using the type's precision
inline IEEEComponents decompose_ieee(double v, int precision_bits) {
	IEEEComponents c;
	c.precision_bits = precision_bits;

	if (v == 0.0) {
		c.sign = std::signbit(v);
		c.exponent = 0;
		c.significand = 0.0;
		c.sig_binary = "0." + std::string(precision_bits - 1, '0');
		return c;
	}

	c.sign = (v < 0.0);
	double av = std::abs(v);

	// Extract exponent and significand
	int exp;
	double frac = std::frexp(av, &exp);
	// frexp returns [0.5, 1.0) * 2^exp, but IEEE wants [1.0, 2.0) * 2^(exp-1)
	c.significand = frac * 2.0;
	c.exponent = exp - 1;

	// Build binary string of significand
	std::ostringstream ss;
	double sig = c.significand;
	// Integer part (hidden bit)
	int int_part = static_cast<int>(sig);
	ss << int_part << ".";
	sig -= int_part;

	// Fraction bits
	for (int i = 0; i < precision_bits - 1; ++i) {
		sig *= 2.0;
		int bit = static_cast<int>(sig);
		ss << bit;
		sig -= bit;
	}
	c.sig_binary = ss.str();
	return c;
}

// Explain IEEE binary addition step by step
inline std::vector<StepDescription> explain_ieee_add(
    double a_val, double b_val, int precision_bits, bool is_subtraction = false)
{
	std::vector<StepDescription> steps;
	int step = 0;

	double a = a_val, b = b_val;
	if (is_subtraction) b = -b;

	auto ca = decompose_ieee(a, precision_bits);
	auto cb = decompose_ieee(b, precision_bits);

	// Step 1: Decompose operands
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decompose operands";
		std::ostringstream detail;
		detail << "A = " << ca.format_value() << "\n"
		       << "           B = " << cb.format_value();
		s.detail = detail.str();
		s.after = "sign, exponent, significand extracted";
		steps.push_back(std::move(s));
	}

	// Handle special cases
	if (a == 0.0 && b == 0.0) {
		StepDescription s;
		s.step_number = ++step;
		s.label = "Result";
		s.detail = "both operands zero";
		s.after = "0.0";
		steps.push_back(std::move(s));
		return steps;
	}

	// Step 2: Align exponents
	int exp_diff = ca.exponent - cb.exponent;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Align exponents";
		std::ostringstream detail;
		if (exp_diff == 0) {
			detail << "exponents equal (both 2^" << ca.exponent << "), no shift needed";
		} else if (exp_diff > 0) {
			detail << "shift B right by " << exp_diff << " position"
			       << (exp_diff > 1 ? "s" : "") << " to match exponent 2^" << ca.exponent;
		} else {
			detail << "shift A right by " << (-exp_diff) << " position"
			       << (-exp_diff > 1 ? "s" : "") << " to match exponent 2^" << cb.exponent;
		}
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 3: Add/subtract significands
	int common_exp = std::max(ca.exponent, cb.exponent);
	double sig_a = ca.significand * std::ldexp(1.0, ca.exponent - common_exp);
	double sig_b = cb.significand * std::ldexp(1.0, cb.exponent - common_exp);
	if (ca.sign) sig_a = -sig_a;
	if (cb.sign) sig_b = -sig_b;
	double sig_sum = sig_a + sig_b;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = is_subtraction ? "Subtract significands" : "Add significands";
		std::ostringstream detail;
		detail << std::setprecision(10) << sig_a << (sig_b >= 0 ? " + " : " - ")
		       << std::abs(sig_b) << " = " << sig_sum
		       << " (at exponent 2^" << common_exp << ")";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 4: Normalize
	bool result_sign = (sig_sum < 0.0);
	double abs_sum = std::abs(sig_sum);
	int result_exp = common_exp;
	if (abs_sum == 0.0) {
		StepDescription s;
		s.step_number = ++step;
		s.label = "Result";
		s.detail = "significands cancelled, result is zero";
		s.after = "0.0";
		steps.push_back(std::move(s));
		return steps;
	}

	int norm_shifts = 0;
	std::string norm_dir;
	if (abs_sum >= 2.0) {
		while (abs_sum >= 2.0) {
			abs_sum *= 0.5;
			result_exp++;
			norm_shifts++;
		}
		norm_dir = "right";
	} else if (abs_sum < 1.0) {
		while (abs_sum < 1.0 && result_exp > -1022) {
			abs_sum *= 2.0;
			result_exp--;
			norm_shifts++;
		}
		norm_dir = "left";
	}
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Normalize";
		std::ostringstream detail;
		if (norm_shifts == 0) {
			detail << "already normalized (1.xxx form)";
		} else {
			detail << "shift " << norm_dir << " by " << norm_shifts
			       << " -> " << std::setprecision(10) << abs_sum
			       << " * 2^" << result_exp;
		}
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Round to target precision
	double result = std::ldexp(abs_sum, result_exp);
	if (result_sign) result = -result;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Round";
		auto cr = decompose_ieee(result, precision_bits);
		std::ostringstream detail;
		detail << "round to " << precision_bits << " significand bits\n"
		       << "           result: " << cr.format_value()
		       << " = " << std::setprecision(12) << result;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Explain IEEE binary multiplication step by step
inline std::vector<StepDescription> explain_ieee_mul(
    double a_val, double b_val, int precision_bits)
{
	std::vector<StepDescription> steps;
	int step = 0;

	auto ca = decompose_ieee(a_val, precision_bits);
	auto cb = decompose_ieee(b_val, precision_bits);

	// Step 1: Decompose
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decompose operands";
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
		s.label = "Add exponents";
		std::ostringstream detail;
		detail << ca.exponent << " + " << cb.exponent << " = " << result_exp;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 4: Multiply significands
	double sig_product = ca.significand * cb.significand;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Multiply significands";
		std::ostringstream detail;
		detail << std::setprecision(10) << ca.significand << " * " << cb.significand
		       << " = " << sig_product;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Normalize
	int norm_shifts = 0;
	if (sig_product >= 2.0) {
		sig_product *= 0.5;
		result_exp++;
		norm_shifts = 1;
	}
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Normalize";
		std::ostringstream detail;
		if (norm_shifts == 0) {
			detail << "already normalized";
		} else {
			detail << "shift right by 1 -> " << std::setprecision(10) << sig_product
			       << " * 2^" << result_exp;
		}
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 6: Round
	double result = std::ldexp(sig_product, result_exp);
	if (result_sign) result = -result;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Round";
		auto cr = decompose_ieee(result, precision_bits);
		std::ostringstream detail;
		detail << "round to " << precision_bits << " significand bits\n"
		       << "           result: " << cr.format_value()
		       << " = " << std::setprecision(12) << result;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Explain IEEE binary division step by step
inline std::vector<StepDescription> explain_ieee_div(
    double a_val, double b_val, int precision_bits)
{
	std::vector<StepDescription> steps;
	int step = 0;

	auto ca = decompose_ieee(a_val, precision_bits);
	auto cb = decompose_ieee(b_val, precision_bits);

	// Step 1: Decompose
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decompose operands";
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
		s.label = "Determine sign";
		s.detail = std::string(ca.sign ? "-" : "+") + " / "
		         + std::string(cb.sign ? "-" : "+") + " = "
		         + std::string(result_sign ? "-" : "+");
		steps.push_back(std::move(s));
	}

	// Step 3: Subtract exponents
	int result_exp = ca.exponent - cb.exponent;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Subtract exponents";
		std::ostringstream detail;
		detail << ca.exponent << " - " << cb.exponent << " = " << result_exp;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 4: Divide significands
	double sig_quotient = ca.significand / cb.significand;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Divide significands";
		std::ostringstream detail;
		detail << std::setprecision(10) << ca.significand << " / " << cb.significand
		       << " = " << sig_quotient;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Normalize
	int norm_shifts = 0;
	if (sig_quotient < 1.0 && sig_quotient > 0.0) {
		while (sig_quotient < 1.0) {
			sig_quotient *= 2.0;
			result_exp--;
			norm_shifts++;
		}
	}
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Normalize";
		std::ostringstream detail;
		if (norm_shifts == 0) {
			detail << "already normalized";
		} else {
			detail << "shift left by " << norm_shifts
			       << " -> " << std::setprecision(10) << sig_quotient
			       << " * 2^" << result_exp;
		}
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 6: Round
	double result = std::ldexp(sig_quotient, result_exp);
	if (result_sign) result = -result;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Round";
		auto cr = decompose_ieee(result, precision_bits);
		std::ostringstream detail;
		detail << "round to " << precision_bits << " significand bits\n"
		       << "           result: " << cr.format_value()
		       << " = " << std::setprecision(12) << result;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Main dispatch: explain an operation for IEEE binary types
inline std::vector<StepDescription> explain_ieee(
    const Value& a, const Value& b, const std::string& op, int precision_bits)
{
	if (op == "add") return explain_ieee_add(a.num, b.num, precision_bits);
	if (op == "sub") return explain_ieee_add(a.num, b.num, precision_bits, true);
	if (op == "mul") return explain_ieee_mul(a.num, b.num, precision_bits);
	if (op == "div") return explain_ieee_div(a.num, b.num, precision_bits);
	return {}; // unsupported operation
}

}} // namespace sw::ucalc
