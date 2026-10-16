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
// It also gives quire<float> and quire<double> the accumulator_* protocol of
// quire/accumulator.hpp, so a linear-algebra library can accumulate in them.
// quire/quire.hpp includes this header, so the protocol is there for every
// quire type once the quire is included.
//
// Relates to #345, #547, stillwater-sc/mtl5#582

#include <cmath>
#include <concepts>
#include <cstdint>
#include <type_traits>
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
// Extract the accumulated value from a quire back into a native type, rounded
// to nearest, ties to even, once (quire_round_to in quire/accumulator.hpp).
// convert_to<T>() would keep only the leading 53 bits and drop the rest, so a
// sum whose deciding bits lie further down would come out truncated.
// ============================================================================
template<unsigned capacity, typename LimbType>
float quire_resolve(const quire<float, capacity, LimbType>& q) {
	return quire_round_to<float>(q);
}

template<unsigned capacity, typename LimbType>
double quire_resolve(const quire<double, capacity, LimbType>& q) {
	return quire_round_to<double>(q);
}

// ============================================================================
// The accumulator_* protocol for quire<float> and quire<double>
//
// quire/accumulator.hpp defines the protocol (MTL5's accumulator_traits finds
// it by argument-dependent lookup on the quire type) for the Universal number
// types, whose quire_mul lives in their own fdp.hpp and is found by ADL on the
// element type. float and double have no associated namespace, so the generic
// functions can't see the quire_mul above; these overloads call it directly.
// They are found the same way, by ADL on quire<float> / quire<double>, and
// the generic ones exclude arithmetic element types, so the two never compete.
//
// Both products are exact (48 and 106 significand bits, subnormals included)
// and both quires span the full product range, so a dot product accumulates
// exactly and rounds once. An infinite or NaN operand makes the quire NaR,
// which rounds out as a quiet NaN.
//
// Relates to stillwater-sc/mtl5#582
// ============================================================================
template<typename Real>
concept native_quire_element = std::same_as<Real, float> || std::same_as<Real, double>;

/// q = 0
template<native_quire_element Real, unsigned capacity, typename LimbType>
void accumulator_clear(quire<Real, capacity, LimbType>& q) {
	q.clear();
}

/// q = v, exactly
template<native_quire_element Real, unsigned capacity, typename LimbType, typename Value>
	requires std::same_as<Value, Real>
void accumulator_assign(quire<Real, capacity, LimbType>& q, const Value& v) {
	q.clear();
	q += quire_mul(v, Real(1));
}

/// q += a * b, the product exact
template<native_quire_element Real, unsigned capacity, typename LimbType, typename Value>
	requires std::same_as<Value, Real>
void accumulator_add_product(quire<Real, capacity, LimbType>& q, const Value& a, const Value& b) {
	q += quire_mul(a, b);
}

/// The round-out: once to float or double, otherwise through the correctly
/// rounded double, which may round twice.
template<native_quire_element Real, unsigned capacity, typename LimbType, typename Result>
Result accumulator_value(const quire<Real, capacity, LimbType>& q, std::type_identity<Result>) {
	if constexpr (std::is_same_v<Result, float> || std::is_same_v<Result, double>) {
		return quire_round_to<Result>(q);
	}
	else {
		return static_cast<Result>(quire_round_to<double>(q));
	}
}

/// q += other, exactly: combines the partial quires of a split reduction
template<native_quire_element Real, unsigned capacity, typename LimbType>
void accumulator_merge(quire<Real, capacity, LimbType>& q, const quire<Real, capacity, LimbType>& other) {
	q += other;
}

}} // namespace sw::universal
