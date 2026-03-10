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

// Forward declarations for all number types that have quire_traits specializations.
// These ensure the traits header can be included independently of the number type headers.
#include <universal/number/posit/posit_fwd.hpp>
#include <universal/number/cfloat/cfloat_fwd.hpp>
#include <universal/number/fixpnt/fixpnt_fwd.hpp>
#include <universal/number/lns/lns_fwd.hpp>
#include <universal/number/dbns/dbns_fwd.hpp>

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
// For IEEE-754-style floating-point, the quire must cover the full range of
// products. A product of two floats with e exponent bits and m mantissa bits
// has dynamic range:
//   escale = 2 * (2^es + mbits + 1)   where mbits = nbits - es
//   range  = escale
//
// For float (32,8): escale = 2*(256+24+1) = 562, qbits = 592
// For double (64,11): escale = 2*(2048+53+1) = 4204, qbits = 4234
// ============================================================================
template<unsigned nbits, unsigned es, typename bt,
         bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
struct quire_traits<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>> {
	static constexpr unsigned mbits       = nbits - es;  // mantissa bits (including hidden bit)
	static constexpr unsigned escale      = 2u * ((1u << es) + mbits + 1u);
	static constexpr unsigned range       = escale;
	static constexpr unsigned half_range  = range >> 1;
	static constexpr unsigned radix_point = half_range;
	static constexpr unsigned upper_range = half_range + 1u;
	static constexpr unsigned capacity    = 30u;

	// fraction bits of the cfloat (mantissa minus hidden bit)
	static constexpr unsigned fbits       = mbits - 1u;
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
	static constexpr unsigned half_range  = nbits;
	static constexpr unsigned radix_point = 2u * rbits;  // product has 2*rbits fractional bits
	static constexpr unsigned upper_range = range - radix_point;
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

}} // namespace sw::universal
