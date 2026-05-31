#pragma once
// ufp.hpp: unit in the first place (UFP) computation for POP precision tuning
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// The UFP (unit in the first place) of a value x is defined as:
//   ufp(x) = 2^floor(log2(|x|))  for x != 0
//   ufp(0) = 0
//
// In integer form, compute_ufp(x) returns floor(log2(|x|)),
// which is the exponent of the most significant bit.
//
// Reference: Dorra Ben Khalifa, "Fast and Efficient Bit-Level Precision Tuning,"
//            PhD thesis, Universite de Perpignan, 2021.

#include <cstdint>
#include <cmath>
#include <cstring>
#include <limits>
#include <algorithm>

namespace sw { namespace universal {

// Forward declaration of range_analyzer (include range_analyzer.hpp for full definition)
template<typename NumberSystem> class range_analyzer;

// compute_ufp returns floor(log2(|x|)) for x != 0
// For x == 0 returns a sentinel value (INT_MIN)
inline int compute_ufp(double x) {
	if (x == 0.0) return std::numeric_limits<int>::min();
	double ax = std::abs(x);
	// Extract the IEEE 754 exponent via bit manipulation
	uint64_t bits;
	std::memcpy(&bits, &ax, sizeof(bits));
	int biased_exp = static_cast<int>((bits >> 52) & 0x7FF);
	// Normal number: exponent = biased_exp - 1023
	// Subnormal: use frexp-style fallback
	if (biased_exp == 0) {
		// subnormal
		int exp;
		std::frexp(ax, &exp);
		return exp - 1;
	}
	return biased_exp - 1023;
}

// compute_ufp for float
inline int compute_ufp(float x) {
	return compute_ufp(static_cast<double>(x));
}

// compute_ufp from a range: returns ufp of the maximum absolute value
inline int compute_ufp(double lo, double hi) {
	double abs_max = std::max(std::abs(lo), std::abs(hi));
	if (abs_max == 0.0) return std::numeric_limits<int>::min();
	return compute_ufp(abs_max);
}

// Bridge to range_analyzer: extract ufp from a range_analyzer's maxScale
// (maxScale already returns the same value as compute_ufp for the largest observed value)
// Usage: int ufp = ufp_from_analyzer(analyzer);
template<typename NumberSystem>
int ufp_from_analyzer(const range_analyzer<NumberSystem>& analyzer) {
	return analyzer.maxScale();
}

}} // namespace sw::universal
