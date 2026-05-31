#pragma once
// transfer.hpp: forward and backward error transfer functions for POP precision tuning
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// POP (Precision-Optimized Programs) uses transfer functions to propagate
// precision information through arithmetic expressions. Each operation has:
//
//   Forward: Given input precisions, compute output precision
//   Backward: Given required output precision, compute required input precisions
//
// Precision is expressed as (ufp, nsb) pairs:
//   ufp = unit in the first place = floor(log2(|x|))
//   nsb = number of significant bits = -log2(relative_error)
//
// The lsb (least significant bit) position is: lsb = ufp - nsb + 1
//
// Reference: Dorra Ben Khalifa, "Fast and Efficient Bit-Level Precision Tuning,"
//            PhD thesis, Universite de Perpignan, 2021, Chapter 4.

#include <algorithm>

namespace sw { namespace universal {

// Precision descriptor for a value
struct precision_info {
	int ufp;    // unit in the first place: floor(log2(|x|))
	int nsb;    // number of significant bits

	constexpr int lsb() const { return ufp - nsb + 1; }
};

// ============================================================================
// Forward transfer functions
// Given input precisions, compute output precision
// ============================================================================

// Forward transfer for addition: z = x + y
// The result ufp is provided externally (from range analysis)
// carry: 1 (conservative default) or 0 (refined via carry analysis)
constexpr precision_info forward_add(precision_info x, precision_info y, int ufp_z, int carry = 1) {
	// lsb(z) = min(lsb(x), lsb(y))  (the finest granularity of either operand)
	int lsb_z = std::min(x.lsb(), y.lsb());
	// nsb(z) = ufp(z) - lsb(z) + 1 + carry
	int nsb_z = ufp_z - lsb_z + 1 + carry;
	return { ufp_z, nsb_z };
}

// Forward transfer for subtraction: z = x - y
// Identical to addition (subtraction has same error propagation)
constexpr precision_info forward_sub(precision_info x, precision_info y, int ufp_z, int carry = 1) {
	return forward_add(x, y, ufp_z, carry);
}

// Forward transfer for multiplication: z = x * y
// carry: 1 (conservative default) or 0 (refined via carry analysis)
constexpr precision_info forward_mul(precision_info x, precision_info y, int carry = 1) {
	// ufp(z) = ufp(x) + ufp(y) (may differ slightly; use range-based ufp if available)
	int ufp_z = x.ufp + y.ufp;
	// nsb(z) = nsb(x) + nsb(y) + carry
	int nsb_z = x.nsb + y.nsb + carry;
	return { ufp_z, nsb_z };
}

// Forward transfer for multiplication with explicit ufp_z (from range analysis)
constexpr precision_info forward_mul(precision_info x, precision_info y, int ufp_z, int carry) {
	int nsb_z = x.nsb + y.nsb + carry;
	return { ufp_z, nsb_z };
}

// Forward transfer for division: z = x / y
// carry: 1 (conservative default) or 0 (refined via carry analysis)
constexpr precision_info forward_div(precision_info x, precision_info y, int carry = 1) {
	int ufp_z = x.ufp - y.ufp;
	int nsb_z = x.nsb + y.nsb + carry;
	return { ufp_z, nsb_z };
}

// Forward transfer for division with explicit ufp_z
constexpr precision_info forward_div(precision_info x, precision_info y, int ufp_z, int carry) {
	int nsb_z = x.nsb + y.nsb + carry;
	return { ufp_z, nsb_z };
}

// Forward transfer for negation: z = -x (precision unchanged)
constexpr precision_info forward_neg(precision_info x) {
	return x;
}

// Forward transfer for absolute value: z = |x| (precision unchanged)
constexpr precision_info forward_abs(precision_info x) {
	return x;
}

// Forward transfer for square root: z = sqrt(x)
// nsb(z) = nsb(x) + carry, ufp(z) ~= ufp(x)/2
constexpr precision_info forward_sqrt(precision_info x, int ufp_z, int carry = 1) {
	int nsb_z = x.nsb + carry;
	return { ufp_z, nsb_z };
}

// ============================================================================
// Backward transfer functions
// Given required output precision, compute required input precisions
// ============================================================================

// Backward for addition z = x + y: required nsb for x given nsb(z)
// nsb(x) >= nsb(z) + ufp(z) - ufp(x) + carry
constexpr int backward_add_lhs(int nsb_z, int ufp_z, int ufp_x, int carry = 1) {
	return nsb_z + ufp_z - ufp_x + carry;
}

// Backward for addition z = x + y: required nsb for y
constexpr int backward_add_rhs(int nsb_z, int ufp_z, int ufp_y, int carry = 1) {
	return nsb_z + ufp_z - ufp_y + carry;
}

// Backward for subtraction z = x - y: identical to addition
constexpr int backward_sub_lhs(int nsb_z, int ufp_z, int ufp_x, int carry = 1) {
	return backward_add_lhs(nsb_z, ufp_z, ufp_x, carry);
}

constexpr int backward_sub_rhs(int nsb_z, int ufp_z, int ufp_y, int carry = 1) {
	return backward_add_rhs(nsb_z, ufp_z, ufp_y, carry);
}

// Backward for multiplication z = x * y: required nsb for x
// nsb(x) >= nsb(z) + carry  (independent of ufp values for multiplication)
constexpr int backward_mul_lhs(int nsb_z, int carry = 1) {
	return nsb_z + carry;
}

// Backward for multiplication z = x * y: required nsb for y
constexpr int backward_mul_rhs(int nsb_z, int carry = 1) {
	return nsb_z + carry;
}

// Backward for division z = x / y: required nsb for x
constexpr int backward_div_lhs(int nsb_z, int carry = 1) {
	return nsb_z + carry;
}

// Backward for division z = x / y: required nsb for y
constexpr int backward_div_rhs(int nsb_z, int carry = 1) {
	return nsb_z + carry;
}

// Backward for negation z = -x: nsb(x) = nsb(z)
constexpr int backward_neg(int nsb_z) {
	return nsb_z;
}

// Backward for absolute value z = |x|: nsb(x) = nsb(z)
constexpr int backward_abs(int nsb_z) {
	return nsb_z;
}

// Backward for square root z = sqrt(x): nsb(x) >= nsb(z) + carry
constexpr int backward_sqrt(int nsb_z, int carry = 1) {
	return nsb_z + carry;
}

}} // namespace sw::universal
