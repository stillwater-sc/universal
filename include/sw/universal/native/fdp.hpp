#pragma once
// fdp.hpp: fused dot product and quire accumulation support for native IEEE-754 types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Native float/double are primitive C++ types without member methods, so the
// quire infrastructure uses free functions for decomposition.  The quire_mul
// function decomposes operands via std::frexp, places the exact significand
// into a MUL blocktriple, and delegates to blocktriple::mul() for the
// full-precision unrounded product.
//
// Relates to #345, #547

#include <cmath>
#include <cstdint>
#include <vector>
#include <universal/number/quire/quire.hpp>

namespace sw { namespace universal {

// ============================================================================
// quire_mul for float
//
// Decomposes two float operands into MUL blocktriples and returns the exact
// full-precision product.  MUL blocktriple has bfbits = 2 + 2*23 = 48 bits.
// ============================================================================
inline blocktriple<23, BlockTripleOperator::MUL, uint32_t>
quire_mul(float lhs, float rhs) {
	constexpr unsigned fbits = 23;
	blocktriple<fbits, BlockTripleOperator::MUL, uint32_t> a, b, product;

	if (lhs == 0.0f || rhs == 0.0f) return product;
	if (std::isnan(lhs) || std::isnan(rhs) || std::isinf(lhs) || std::isinf(rhs)) {
		product.setnan();
		return product;
	}

	// Decompose lhs
	{
		int exp;
		float frac = std::frexp(std::abs(lhs), &exp);
		// frexp returns frac in [0.5, 1.0), scale = exp - 1
		// significand = frac * 2^(fbits+1) gives hidden bit at position fbits
		a.setnormal();
		a.setsign(lhs < 0.0f);
		a.setscale(exp - 1);
		uint64_t sig = static_cast<uint64_t>(std::ldexp(frac, fbits + 1));
		a.setbits(sig);
	}
	// Decompose rhs
	{
		int exp;
		float frac = std::frexp(std::abs(rhs), &exp);
		b.setnormal();
		b.setsign(rhs < 0.0f);
		b.setscale(exp - 1);
		uint64_t sig = static_cast<uint64_t>(std::ldexp(frac, fbits + 1));
		b.setbits(sig);
	}

	product.mul(a, b);
	return product;
}

// ============================================================================
// quire_mul for double
//
// MUL blocktriple has bfbits = 2 + 2*52 = 106 bits.
// ============================================================================
inline blocktriple<52, BlockTripleOperator::MUL, uint32_t>
quire_mul(double lhs, double rhs) {
	constexpr unsigned fbits = 52;
	blocktriple<fbits, BlockTripleOperator::MUL, uint32_t> a, b, product;

	if (lhs == 0.0 || rhs == 0.0) return product;
	if (std::isnan(lhs) || std::isnan(rhs) || std::isinf(lhs) || std::isinf(rhs)) {
		product.setnan();
		return product;
	}

	// Decompose lhs
	{
		int exp;
		double frac = std::frexp(std::abs(lhs), &exp);
		a.setnormal();
		a.setsign(lhs < 0.0);
		a.setscale(exp - 1);
		uint64_t sig = static_cast<uint64_t>(std::ldexp(frac, fbits + 1));
		a.setbits(sig);
	}
	// Decompose rhs
	{
		int exp;
		double frac = std::frexp(std::abs(rhs), &exp);
		b.setnormal();
		b.setsign(rhs < 0.0);
		b.setscale(exp - 1);
		uint64_t sig = static_cast<uint64_t>(std::ldexp(frac, fbits + 1));
		b.setbits(sig);
	}

	product.mul(a, b);
	return product;
}

// ============================================================================
// quire_resolve for float and double
//
// Extract the accumulated value from a quire back into a native type.
// Uses the quire's convert_to<T>() which goes through double intermediate.
// This is exact for float and for double values within the representable range.
// ============================================================================
template<unsigned capacity, typename LimbType>
float quire_resolve(const quire<float, capacity, LimbType>& q) {
	return q.template convert_to<float>();
}

template<unsigned capacity, typename LimbType>
double quire_resolve(const quire<double, capacity, LimbType>& q) {
	return q.template convert_to<double>();
}

}} // namespace sw::universal
