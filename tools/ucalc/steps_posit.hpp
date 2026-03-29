#pragma once
// steps_posit.hpp: step-by-step arithmetic decomposition for posit types
//
// Decomposes posit add/sub/mul into stages showing the regime encoding:
//   1. Decode regime (run-length -> k value)
//   2. Decode exponent (es bits)
//   3. Compute scale (useed^k * 2^exponent)
//   4. Extract fraction (remaining bits)
//   5. Align/operate on fractions
//   6. Re-encode regime
//   7. Round fraction to fit
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

// Posit component decomposition from a double value
struct PositComponents {
	bool sign;
	int k;              // regime value
	int regime_bits;    // number of bits used by regime
	int exponent;       // es-bit exponent value
	int es;             // exponent size parameter
	int scale;          // total scale = k * 2^es + exponent
	int fraction_bits;  // bits available for fraction
	double significand; // 1.xxxx (normalized)
	int nbits;          // total posit width

	std::string format_value() const {
		std::ostringstream ss;
		ss << (sign ? "-" : "+") << std::setprecision(10) << significand
		   << " * 2^" << scale;
		return ss.str();
	}

	std::string regime_string() const {
		// Regime encoding: k>=0 -> (k+1) ones then zero; k<0 -> (-k) zeros then one
		std::string s;
		if (k >= 0) {
			for (int i = 0; i <= k; ++i) s += '1';
			s += '0';
		} else {
			for (int i = 0; i < -k; ++i) s += '0';
			s += '1';
		}
		return s;
	}
};

// Decompose a value into posit components
inline PositComponents decompose_posit(double v, int nbits, int es_param) {
	PositComponents c;
	c.nbits = nbits;
	c.es = es_param;
	c.sign = (v < 0.0);

	if (v == 0.0) {
		c.k = 0; c.regime_bits = 2; c.exponent = 0;
		c.scale = 0; c.fraction_bits = nbits - 3;
		c.significand = 0.0;
		return c;
	}

	double av = std::abs(v);
	int exp;
	double frac = std::frexp(av, &exp);
	c.significand = frac * 2.0;
	int binary_scale = exp - 1; // value = significand * 2^binary_scale

	// Decompose scale into regime (k) and exponent (e)
	// scale = k * 2^es + e, where 0 <= e < 2^es
	int useed_exp = (1 << es_param); // 2^es
	if (binary_scale >= 0) {
		c.k = binary_scale / useed_exp;
		c.exponent = binary_scale % useed_exp;
	} else {
		// For negative scales: floor division
		c.k = -(-binary_scale + useed_exp - 1) / useed_exp;
		c.exponent = binary_scale - c.k * useed_exp;
		if (c.exponent < 0) { c.k--; c.exponent += useed_exp; }
	}
	c.scale = binary_scale;

	// Regime bits: k>=0 -> (k+2) bits, k<0 -> (-k+1) bits
	c.regime_bits = (c.k >= 0) ? (c.k + 2) : (-c.k + 1);
	// Fraction bits = total - 1(sign) - regime_bits - es
	c.fraction_bits = nbits - 1 - c.regime_bits - es_param;
	if (c.fraction_bits < 0) c.fraction_bits = 0;

	return c;
}

// Explain posit addition step by step
inline std::vector<StepDescription> explain_posit_add(
    double a_val, double b_val, int nbits, int es_param, bool is_subtraction = false)
{
	std::vector<StepDescription> steps;
	int step = 0;

	double b = is_subtraction ? -b_val : b_val;
	auto ca = decompose_posit(a_val, nbits, es_param);
	auto cb = decompose_posit(b, nbits, es_param);

	// Step 1: Decode operands
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decode posit operands";
		std::ostringstream detail;
		detail << "A: regime=" << ca.regime_string() << " (k=" << ca.k << ")"
		       << ", exponent=" << ca.exponent
		       << ", scale=2^" << ca.scale << "\n"
		       << "       " << ca.format_value()
		       << " (" << ca.fraction_bits << " fraction bits)\n"
		       << "           B: regime=" << cb.regime_string() << " (k=" << cb.k << ")"
		       << ", exponent=" << cb.exponent
		       << ", scale=2^" << cb.scale << "\n"
		       << "       " << cb.format_value()
		       << " (" << cb.fraction_bits << " fraction bits)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 2: Align scales
	int scale_diff = ca.scale - cb.scale;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Align scales";
		std::ostringstream detail;
		if (scale_diff == 0) {
			detail << "scales equal (2^" << ca.scale << "), no shift needed";
		} else if (scale_diff > 0) {
			detail << "shift B right by " << scale_diff << " bits to align with A's scale 2^" << ca.scale;
		} else {
			detail << "shift A right by " << (-scale_diff) << " bits to align with B's scale 2^" << cb.scale;
		}
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 3: Add/subtract fractions
	int common_scale = std::max(ca.scale, cb.scale);
	double sig_a = ca.significand * std::ldexp(1.0, ca.scale - common_scale);
	double sig_b = cb.significand * std::ldexp(1.0, cb.scale - common_scale);
	if (ca.sign) sig_a = -sig_a;
	if (cb.sign) sig_b = -sig_b;
	double sig_sum = sig_a + sig_b;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = is_subtraction ? "Subtract fractions" : "Add fractions";
		std::ostringstream detail;
		detail << std::setprecision(10) << sig_a << (sig_b >= 0 ? " + " : " - ")
		       << std::abs(sig_b) << " = " << sig_sum;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 4: Normalize and determine new regime
	bool result_sign = (sig_sum < 0.0);
	double abs_sum = std::abs(sig_sum);
	int result_scale = common_scale;
	if (abs_sum >= 2.0) {
		while (abs_sum >= 2.0) { abs_sum *= 0.5; result_scale++; }
	} else if (abs_sum > 0.0 && abs_sum < 1.0) {
		while (abs_sum < 1.0) { abs_sum *= 2.0; result_scale--; }
	}

	auto cr = decompose_posit(std::ldexp(abs_sum, result_scale), nbits, es_param);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Re-encode regime";
		std::ostringstream detail;
		detail << "result scale: 2^" << result_scale
		       << " -> regime k=" << cr.k << " (" << cr.regime_string() << ")"
		       << ", exponent=" << cr.exponent << "\n"
		       << "           " << cr.fraction_bits << " fraction bits available"
		       << " (regime uses " << cr.regime_bits << " bits)";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Round
	double result = std::ldexp(abs_sum, result_scale);
	if (result_sign) result = -result;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Round to fit";
		std::ostringstream detail;
		detail << "round to " << cr.fraction_bits << " fraction bits"
		       << " (from " << nbits << "-bit posit<" << nbits << "," << es_param << ">)\n"
		       << "           result: " << std::setprecision(12) << result;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Explain posit multiplication step by step
inline std::vector<StepDescription> explain_posit_mul(
    double a_val, double b_val, int nbits, int es_param)
{
	std::vector<StepDescription> steps;
	int step = 0;

	auto ca = decompose_posit(a_val, nbits, es_param);
	auto cb = decompose_posit(b_val, nbits, es_param);

	// Step 1: Decode
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Decode posit operands";
		std::ostringstream detail;
		detail << "A: regime=" << ca.regime_string() << " (k=" << ca.k << ")"
		       << ", scale=2^" << ca.scale
		       << ", " << ca.fraction_bits << " fraction bits\n"
		       << "           B: regime=" << cb.regime_string() << " (k=" << cb.k << ")"
		       << ", scale=2^" << cb.scale
		       << ", " << cb.fraction_bits << " fraction bits";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 2: Add scales
	bool result_sign = (ca.sign != cb.sign);
	int result_scale = ca.scale + cb.scale;
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Add exponents (multiply scales)";
		std::ostringstream detail;
		detail << "2^" << ca.scale << " * 2^" << cb.scale
		       << " = 2^(" << ca.scale << "+" << cb.scale << ")"
		       << " = 2^" << result_scale;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 3: Multiply significands
	double sig_product = ca.significand * cb.significand;
	if (sig_product >= 2.0) { sig_product *= 0.5; result_scale++; }
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

	// Step 4: Re-encode regime
	double result = std::ldexp(sig_product, result_scale);
	if (result_sign) result = -result;
	auto cr = decompose_posit(std::abs(result), nbits, es_param);
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Re-encode regime";
		std::ostringstream detail;
		detail << "result scale: 2^" << result_scale
		       << " -> regime k=" << cr.k << " (" << cr.regime_string() << ")"
		       << ", exponent=" << cr.exponent << "\n"
		       << "           " << cr.fraction_bits << " fraction bits available";
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	// Step 5: Round
	{
		StepDescription s;
		s.step_number = ++step;
		s.label = "Round to fit";
		std::ostringstream detail;
		detail << "round to " << cr.fraction_bits << " fraction bits\n"
		       << "           result: " << std::setprecision(12) << result;
		s.detail = detail.str();
		steps.push_back(std::move(s));
	}

	return steps;
}

// Main dispatch for posit steps
inline std::vector<StepDescription> explain_posit(
    const Value& a, const Value& b, const std::string& op, int nbits, int es_param)
{
	if (op == "add") return explain_posit_add(a.num, b.num, nbits, es_param);
	if (op == "sub") return explain_posit_add(a.num, b.num, nbits, es_param, true);
	if (op == "mul") return explain_posit_mul(a.num, b.num, nbits, es_param);
	// div: fall back to IEEE decomposition for now
	return {};
}

}} // namespace sw::ucalc
