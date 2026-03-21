#pragma once
// quire_traits.hpp: compile-time traits for sizing and configuring quire accumulators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The quire (Kulisch super-accumulator) is a fixed-point accumulator sized to hold
// the full dynamic range of products for a given number system. Each number type has
// a different dynamic range formula, captured here as compile-time traits.
//
// Usage:
//   using Traits = quire_traits<posit<32,2>>;
//   constexpr unsigned qbits = Traits::qbits;  // total accumulator bits (default capacity)
//   constexpr unsigned custom = Traits::range + 50;  // custom capacity
//
// Relates to #345, #545

#include <cstddef>
#include <type_traits>

#include <universal/number/quire/quire_fwd.hpp>

// Forward declarations for all number types that have quire_traits specializations.
// These ensure the traits header can be included independently of the number type headers.
#include <universal/number/posit/posit_fwd.hpp>
#include <universal/number/cfloat/cfloat_fwd.hpp>
#include <universal/number/fixpnt/fixpnt_fwd.hpp>
#include <universal/number/lns/lns_fwd.hpp>
#include <universal/number/dbns/dbns_fwd.hpp>
#include <universal/number/integer/integer_fwd.hpp>

namespace sw { namespace universal {

// Primary template: unspecialized produces a compile error
// to ensure every number type that wants quire support provides a specialization
template<typename NumberType>
struct quire_traits {
	static_assert(sizeof(NumberType) == 0,
		"quire_traits<T> is not specialized for this number type. "
		"Provide a specialization to enable quire/FDP support.");
};

// ============================================================================
// posit<nbits, es, bt> specialization
//
// The posit standard defines the quire size based on the posit's tapered
// floating-point dynamic range:
//   escale     = 2^es
//   range      = escale * (4*nbits - 8)   // full dynamic range of products
//   half_range = range / 2                // radix point position
//   upper_range= half_range + 1           // maxpos^2 needs one extra bit
//   qbits      = range + capacity
//
// For posit<32,2>: escale=4, range=480, qbits=510 (with default capacity=30)
// ============================================================================
template<unsigned nbits, unsigned es, typename bt>
struct quire_traits<posit<nbits, es, bt>> {
	static constexpr unsigned escale      = (1u << es);
	static constexpr unsigned range       = escale * (4u * nbits - 8u);
	static constexpr unsigned half_range  = range >> 1;
	static constexpr unsigned radix_point = half_range;
	static constexpr unsigned upper_range = half_range + 1u;
	static constexpr unsigned capacity    = 30u;

	// fraction bits of the posit (used for quire_mul product width)
	static constexpr unsigned fbits       = nbits - 3u - es;
	// product fraction bits: 2 * (fbits + 1) for full-width unrounded multiply
	static constexpr unsigned product_fbits = 2u * (fbits + 1u);

	// total quire bits (excluding sign) with default capacity
	static constexpr unsigned qbits       = range + capacity;
};

// ============================================================================
// cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>
//
// The quire must place minpos^2 on the LSB (bit 0) and maxpos^2 on the MSB
// of the upper half. The range is generally asymmetric for cfloat because
// subnormals extend the negative exponent range far beyond the positive range.
//
// Scale bounds for individual values:
//   bias      = 2^(es-1) - 1
//   max_scale = bias     (or bias+1 if hasMaxExpValues)
//   min_scale = 1 - bias - fbits  (if hasSubnormals)
//             = 1 - bias          (if !hasSubnormals)
//
// Product scale bounds:
//   max_product_scale = 2*max_scale + 1   (maxpos^2 significand in [2,4))
//   min_product_scale = 2*min_scale       (minpos^2)
//
// Quire layout:
//   radix_point = |min_product_scale|     -> minpos^2 lands on bit 0
//   upper_range = max_product_scale + 1   -> maxpos^2 MSB lands on bit range-1
//   range       = radix_point + upper_range
//
// For cfloat<8,3>:  bias=3, range=20, radix_point=12, qbits=50
// For cfloat<32,8>: bias=127, range=554, radix_point=298, qbits=584
// ============================================================================
template<unsigned nbits, unsigned es, typename bt,
         bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
struct quire_traits<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>> {
	static constexpr unsigned mbits       = nbits - es;  // mantissa bits (including hidden bit)
	static constexpr unsigned fbits       = mbits - 1u;   // fraction bits (mantissa minus hidden bit)

	// IEEE-754 exponent bias
	static constexpr unsigned bias        = (1u << (es - 1u)) - 1u;

	// Maximum scale of a representable value
	static constexpr unsigned max_scale   = hasMaxExpValues ? (bias + 1u) : bias;

	// |min_scale|: magnitude of the most negative scale
	//   hasSubnormals:  min_scale = 1 - bias - fbits  ->  |min_scale| = bias + fbits - 1
	//   !hasSubnormals: min_scale = 1 - bias           ->  |min_scale| = bias - 1
	static constexpr unsigned abs_min_scale = hasSubnormals
		? (bias + fbits - 1u)
		: (bias >= 1u ? bias - 1u : 0u);

	// Quire geometry derived from product scale bounds
	static constexpr unsigned radix_point = 2u * abs_min_scale;   // minpos^2 at bit 0
	static constexpr unsigned upper_range = 2u * max_scale + 2u;  // maxpos^2 MSB at bit range-1
	static constexpr unsigned range       = radix_point + upper_range;

	// half_range is used for symmetric bounds checking in quire::operator=;
	// for cfloat the range is asymmetric, so use the larger of the two halves
	static constexpr unsigned half_range  = (radix_point > upper_range) ? radix_point : upper_range;

	static constexpr unsigned capacity    = 30u;

	// product fraction bits: full-width unrounded multiply
	static constexpr unsigned product_fbits = 2u * mbits;

	static constexpr unsigned qbits       = range + capacity;
};

// ============================================================================
// fixpnt<nbits, rbits, arithmetic, bt>
//
// Fixed-point products have a fully static range. The product of two n-bit
// fixed-point numbers with r fractional bits each is a (2n)-bit number with
// 2r fractional bits.
//   range      = 2 * nbits
//   half_range = nbits (= number of integer + fractional bits)
//
// For fixpnt<16,8>: range = 32, qbits = 62
// ============================================================================
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
struct quire_traits<fixpnt<nbits, rbits, arithmetic, bt>> {
	static constexpr unsigned range       = 2u * nbits;
	static constexpr unsigned radix_point = 2u * rbits;  // product has 2*rbits fractional bits
	static constexpr unsigned upper_range = range - radix_point;

	// half_range is used for symmetric bounds checking in quire::operator+=;
	// for fixpnt the range can be asymmetric when rbits != nbits/2,
	// so use the larger of the two halves (same pattern as cfloat)
	static constexpr unsigned half_range  = (radix_point > upper_range) ? radix_point : upper_range;
	static constexpr unsigned capacity    = 30u;

	// fraction bits: rbits for each operand
	static constexpr unsigned fbits       = rbits;
	// product bits: full width
	static constexpr unsigned product_fbits = 2u * nbits;

	static constexpr unsigned qbits       = range + capacity;
};

// ============================================================================
// lns<nbits, rbits, bt, xtra...>
//
// Logarithmic number system: value = (-1)^sign * 2^(integer.fraction)
// The integer part of the exponent field determines the dynamic range.
// Multiplication in LNS is addition of exponents, but for quire accumulation
// we need to materialize linear-domain values.
//
// The exponent integer field has (nbits - 1 - rbits) bits (sign bit excluded).
// Product exponent range: 2 * (2^(integer_bits) - 1)
//   range = 2 * (2^(nbits - 1 - rbits))
//
// For lns<16,8>: integer_bits=7, range=2*128=256, qbits=286
//
// NOTE: This is a provisional sizing. LNS quire support (issue #549) may
// reveal that a different formula or approach is needed.
// ============================================================================
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
struct quire_traits<lns<nbits, rbits, bt, xtra...>> {
	static constexpr unsigned sign_bits    = 1u;
	static constexpr unsigned integer_bits = nbits - sign_bits - rbits;
	static constexpr unsigned max_exponent = (1u << integer_bits);
	static constexpr unsigned range        = 2u * max_exponent;
	static constexpr unsigned half_range   = max_exponent;
	static constexpr unsigned radix_point  = half_range;
	static constexpr unsigned upper_range  = half_range + 1u;
	static constexpr unsigned capacity     = 30u;

	// LNS doesn't have traditional fraction bits; rbits is the fractional
	// part of the exponent. For quire product width we use the full range.
	static constexpr unsigned fbits        = rbits;
	static constexpr unsigned product_fbits = 2u * rbits;

	static constexpr unsigned qbits        = range + capacity;
};

// ============================================================================
// dbns<nbits, fbbits, bt, xtra...>
//
// Double-base number system: value = (-1)^sign * 2^a * 3^b
// The dynamic range depends on the ranges of both base-2 and base-3 exponents.
// For quire sizing we conservatively use the full bit width minus sign.
//
// NOTE: This is a placeholder sizing. DBNS quire support (issue #549) may
// reveal that a fundamentally different accumulation strategy is needed.
// ============================================================================
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
struct quire_traits<dbns<nbits, fbbits, bt, xtra...>> {
	// Conservative estimate: treat the full exponent range as if base-2
	static constexpr unsigned exponent_bits = nbits - 1u;  // excluding sign
	static constexpr unsigned range         = 2u * (1u << (exponent_bits > 10u ? 10u : exponent_bits));
	static constexpr unsigned half_range    = range >> 1;
	static constexpr unsigned radix_point   = half_range;
	static constexpr unsigned upper_range   = half_range + 1u;
	static constexpr unsigned capacity      = 30u;

	static constexpr unsigned fbits         = fbbits;
	static constexpr unsigned product_fbits = 2u * fbbits;

	static constexpr unsigned qbits         = range + capacity;
};

// ============================================================================
// integer<nbits, BlockType, NumberType>
//
// Integers have no fractional bits. The dynamic range of a product of two
// n-bit integers is 2*n bits. For quire accumulation we treat the full
// magnitude (nbits-1 bits, excluding sign) as the significand.
//
// For integer<32>: range = 64, fbits = 31, qbits = 94
// ============================================================================
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
struct quire_traits<integer<nbits, BlockType, NumberType>> {
	static constexpr unsigned range         = 2u * nbits;
	static constexpr unsigned half_range    = nbits;
	static constexpr unsigned radix_point   = 0u;
	static constexpr unsigned upper_range   = range;
	static constexpr unsigned capacity      = 30u;

	// all magnitude bits (sign excluded) form the significand
	static constexpr unsigned fbits         = nbits - 1u;
	static constexpr unsigned product_fbits = 2u * nbits;

	static constexpr unsigned qbits         = range + capacity;
};


// ============================================================================
// Native IEEE-754 float specialization
//
// float has 23 fraction bits, bias=127, and subnormals.
// This follows the same asymmetric sizing formula as cfloat with hasSubnormals.
//
// For float: bias=127, fbits=23, range=554, radix_point=298, qbits=584
// ============================================================================
template<>
struct quire_traits<float> {
	static constexpr unsigned fbits       = 23u;   // mantissa bits (excluding hidden bit)
	static constexpr unsigned bias        = 127u;

	static constexpr unsigned max_scale   = bias;   // float has no maxExpValues mode
	static constexpr unsigned abs_min_scale = bias + fbits - 1u;  // subnormals: 127 + 23 - 1 = 149

	static constexpr unsigned radix_point = 2u * abs_min_scale;   // 298
	static constexpr unsigned upper_range = 2u * max_scale + 2u;  // 256
	static constexpr unsigned range       = radix_point + upper_range;  // 554

	static constexpr unsigned half_range  = (radix_point > upper_range) ? radix_point : upper_range;
	static constexpr unsigned capacity    = 30u;

	static constexpr unsigned product_fbits = 2u * (fbits + 1u);  // 48
	static constexpr unsigned qbits       = range + capacity;      // 584
};

// ============================================================================
// Native IEEE-754 double specialization
//
// double has 52 fraction bits, bias=1023, and subnormals.
//
// For double: bias=1023, fbits=52, range=4196, radix_point=2148, qbits=4226
//
// NOTE: A double quire is ~4226 bits (~528 bytes). This is large but
// blockbinary handles it. BitWalk tests will be slow due to the wide range.
// ============================================================================
template<>
struct quire_traits<double> {
	static constexpr unsigned fbits       = 52u;
	static constexpr unsigned bias        = 1023u;

	static constexpr unsigned max_scale   = bias;
	static constexpr unsigned abs_min_scale = bias + fbits - 1u;  // 1023 + 52 - 1 = 1074

	static constexpr unsigned radix_point = 2u * abs_min_scale;   // 2148
	static constexpr unsigned upper_range = 2u * max_scale + 2u;  // 2048
	static constexpr unsigned range       = radix_point + upper_range;  // 4196

	static constexpr unsigned half_range  = (radix_point > upper_range) ? radix_point : upper_range;
	static constexpr unsigned capacity    = 30u;

	static constexpr unsigned product_fbits = 2u * (fbits + 1u);  // 106
	static constexpr unsigned qbits       = range + capacity;      // 4226
};

// define a trait for the generalize quire types
template<typename _Ty>
struct is_quire_trait
	: std::false_type
{
};
template<typename NumberType, unsigned capacity, typename LimbType>
struct is_quire_trait< sw::universal::quire<NumberType, capacity, LimbType> >
	: std::true_type
{
};

template<typename _Ty>
constexpr bool is_quire = is_quire_trait<_Ty>::value;

template<typename _Ty, typename Type = _Ty>
using enable_if_quire = std::enable_if_t<is_quire<_Ty>, Type>;

}} // namespace sw::universal
