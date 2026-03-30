#pragma once
// steps_hfloat.hpp: step-by-step arithmetic decomposition for hexadecimal float
//
// IBM System/360 hex float: value = 16^exponent * 0.h1h2h3...
// Key difference from IEEE: normalization is to hex-digit boundary,
// not bit boundary. This causes "wobbling precision" (0-3 wasted bits).
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

struct HfloatComponents {
	bool sign;
	int hex_exp;            // power of 16
	double hex_fraction;    // 0.h1h2h3... (value in [1/16, 1))
	int ndigits;            // number of hex fraction digits
	std::string hex_str;    // hex fraction as string

	std::string format_value() const {
		std::ostringstream ss;
		ss << (sign ? "-" : "+") << "16^" << hex_exp << " * 0x0." << hex_str;
		return ss.str();
	}
};

inline HfloatComponents decompose_hfloat(double v, int ndigits) {
	HfloatComponents c;
	c.ndigits = ndigits;
	c.sign = (v < 0.0);

	if (v == 0.0) {
		c.hex_exp = 0;
		c.hex_fraction = 0.0;
		c.hex_str = std::string(ndigits, '0');
		return c;
	}

	double av = std::abs(v);
	// hex exponent: 16^e where 1/16 <= fraction < 1
	c.hex_exp = static_cast<int>(std::ceil(std::log(av) / std::log(16.0)));
	c.hex_fraction = av / std::pow(16.0, c.hex_exp);
	// Adjust if fraction >= 1 or < 1/16
	if (c.hex_fraction >= 1.0) { c.hex_exp++; c.hex_fraction /= 16.0; }
	if (c.hex_fraction < 1.0 / 16.0 && c.hex_fraction > 0.0) { c.hex_exp--; c.hex_fraction *= 16.0; }

	// Build hex digit string
	std::ostringstream hs;
	double frac = c.hex_fraction;
	for (int i = 0; i < ndigits; ++i) {
		frac *= 16.0;
		int digit = static_cast<int>(frac);
		if (digit > 15) digit = 15;
		hs << "0123456789ABCDEF"[digit];
		frac -= digit;
	}
	c.hex_str = hs.str();

	return c;
}

// Explain hfloat addition
inline std::vector<StepDescription> explain_hfloat_add(
    double a_val, double b_val, int ndigits, bool is_subtraction = false)
{
	std::vector<StepDescription> steps;
	int step = 0;

	double b = is_subtraction ? -b_val : b_val;
	auto ca = decompose_hfloat(a_val, ndigits);
	auto cb = decompose_hfloat(b, ndigits);

	// Step 1: Decompose
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decompose hex float operands";
		std::ostringstream detail;
		detail << "A = " << ca.format_value() << "\n"
		       << "           B = " << cb.format_value();
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 2: Align hex exponents (shift by whole hex digits)
	int exp_diff = ca.hex_exp - cb.hex_exp;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Align hex exponents";
		std::ostringstream detail;
		if (exp_diff == 0) {
			detail << "exponents equal (16^" << ca.hex_exp << "), no shift needed";
		} else {
			int shifts = std::abs(exp_diff);
			detail << "shift " << (exp_diff > 0 ? "B" : "A") << " right by "
			       << shifts << " hex digit" << (shifts > 1 ? "s" : "")
			       << " (= " << (shifts * 4) << " bits)\n"
			       << "           (hex float shifts by whole hex digits, not single bits)";
		}
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 3: Add hex fractions
	int common_exp = std::max(ca.hex_exp, cb.hex_exp);
	double frac_a = ca.hex_fraction * std::pow(16.0, ca.hex_exp - common_exp);
	double frac_b = cb.hex_fraction * std::pow(16.0, cb.hex_exp - common_exp);
	if (ca.sign) frac_a = -frac_a;
	if (cb.sign) frac_b = -frac_b;
	double frac_sum = frac_a + frac_b;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = is_subtraction ? "Subtract hex fractions" : "Add hex fractions";
		std::ostringstream detail;
		detail << "0x0." << ca.hex_str << (frac_b >= 0 ? " + 0x0." : " - 0x0.") << cb.hex_str
		       << " (hex digit arithmetic)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 4: Hex-normalize (THE KEY INSIGHT)
	bool result_sign = (frac_sum < 0.0);
	double abs_frac = std::abs(frac_sum);
	int result_exp = common_exp;
	int hex_shifts = 0;
	if (abs_frac >= 1.0) {
		while (abs_frac >= 1.0) { abs_frac /= 16.0; result_exp++; hex_shifts++; }
	} else if (abs_frac > 0.0 && abs_frac < 1.0 / 16.0) {
		while (abs_frac < 1.0 / 16.0) { abs_frac *= 16.0; result_exp--; hex_shifts++; }
	}
	// Compute leading hex digit to show wobble
	int leading_digit = static_cast<int>(abs_frac * 16.0);
	int leading_bits = 0;
	if (leading_digit >= 8) leading_bits = 4;
	else if (leading_digit >= 4) leading_bits = 3;
	else if (leading_digit >= 2) leading_bits = 2;
	else if (leading_digit >= 1) leading_bits = 1;
	int wasted_bits = 4 - leading_bits;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Hex-normalize (source of wobbling precision!)";
		std::ostringstream detail;
		if (hex_shifts == 0) {
			detail << "already hex-normalized (leading hex digit non-zero)";
		} else {
			detail << "shift left by " << hex_shifts << " hex digit(s)";
		}
		detail << "\n           leading hex digit: "
		       << "0123456789ABCDEF"[leading_digit]
		       << " (" << leading_bits << " significant bits)\n"
		       << "           wasted bits: " << wasted_bits << " of 4"
		       << " (IEEE binary normalization would waste 0)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Truncate (hfloat never rounds up)
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Truncate (hfloat uses truncation, never rounds up)";
		s.detail = "truncate to " + std::to_string(ndigits) + " hex fraction digits";
		steps.push_back(std::move(s));
	}

	// Step 6: Result
	double result = (result_sign ? -1.0 : 1.0) * abs_frac * std::pow(16.0, result_exp);
	auto cr = decompose_hfloat(result, ndigits);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Result";
		std::ostringstream detail;
		detail << cr.format_value() << " = " << std::setprecision(10) << result;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Explain hfloat multiplication
inline std::vector<StepDescription> explain_hfloat_mul(
    double a_val, double b_val, int ndigits)
{
	std::vector<StepDescription> steps;
	int step = 0;

	auto ca = decompose_hfloat(a_val, ndigits);
	auto cb = decompose_hfloat(b_val, ndigits);

	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decompose hex float operands";
		std::ostringstream detail;
		detail << "A = " << ca.format_value() << "\n"
		       << "           B = " << cb.format_value();
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	bool result_sign = (ca.sign != cb.sign);
	int result_exp = ca.hex_exp + cb.hex_exp;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Add hex exponents";
		std::ostringstream detail;
		detail << ca.hex_exp << " + " << cb.hex_exp << " = " << result_exp;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	double frac_product = ca.hex_fraction * cb.hex_fraction;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Multiply hex fractions";
		std::ostringstream detail;
		detail << "0x0." << ca.hex_str << " * 0x0." << cb.hex_str;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Hex-normalize
	if (frac_product < 1.0 / 16.0 && frac_product > 0.0) {
		frac_product *= 16.0;
		result_exp--;
	}
	int leading_digit = static_cast<int>(frac_product * 16.0);
	int leading_bits = 0;
	if (leading_digit >= 8) leading_bits = 4;
	else if (leading_digit >= 4) leading_bits = 3;
	else if (leading_digit >= 2) leading_bits = 2;
	else if (leading_digit >= 1) leading_bits = 1;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Hex-normalize and truncate";
		std::ostringstream detail;
		detail << "leading hex digit: " << "0123456789ABCDEF"[leading_digit]
		       << " (" << leading_bits << " significant bits, " << (4 - leading_bits) << " wasted)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	double result = (result_sign ? -1.0 : 1.0) * frac_product * std::pow(16.0, result_exp);
	auto cr = decompose_hfloat(result, ndigits);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Result";
		std::ostringstream detail;
		detail << cr.format_value() << " = " << std::setprecision(10) << result;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Main dispatch
inline std::vector<StepDescription> explain_hfloat(
    const Value& a, const Value& b, const std::string& op, int ndigits)
{
	if (op == "add") return explain_hfloat_add(a.num, b.num, ndigits);
	if (op == "sub") return explain_hfloat_add(a.num, b.num, ndigits, true);
	if (op == "mul") return explain_hfloat_mul(a.num, b.num, ndigits);
	if (op == "div") {
		StepDescription s;
		s.step_number = 1;
		s.label = "Division";
		s.detail = "hex float division step-by-step not yet implemented";
		return { s };
	}
	return {};
}

}} // namespace sw::ucalc
