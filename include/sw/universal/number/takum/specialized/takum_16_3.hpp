#pragma once
// takum_16_3.hpp: fast specialization of the linear takum<16, 3, uint16_t>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// DO NOT USE DIRECTLY!
// Included by <universal/number/takum/specializations.hpp> when TAKUM_FAST_TAKUM_16 is set.
//
// Only the two codec members are replaced: conversion from double/float
// (convert_ieee754) and conversion to double/float (to_ieee754).  The generic
// class evaluates + - * / sqrt, the integer conversions and the compound
// assignments by decoding to double, operating, and converting back, so all of
// those inherit the speedup while running the generic code around them.  Every
// other member is untouched and therefore identical.
//
// Why a double carries takum16 exactly
//   For nbits = 16 the C field never exceeds r <= 7 < 11 bits, so no
//   characteristic bit is dropped and a value is (1 + M/2^p) 2^c with p <= 11 and
//   c in [-255, 254]: a significand of at most 12 bits, exactly a double.
//   The product of two such significands has at most 24 bits and is exact.
//   For + - / sqrt, rounding first to double and then to takum16 is the correct
//   rounding: every takum16 value and every midpoint between neighbours (<= 13
//   significant bits) is a double, so the double rounding can only go wrong if an
//   inexact result lies within half a double ulp (2^-53 relative) of a midpoint.
//   A nonzero gap between a quotient or square root of 12-bit operands and a
//   13-bit midpoint is at least ~2^-27 relative, and an inexact sum is within
//   2^-40 of a takum16 value, never of a midpoint (Figueroa: 53 >= 2*12 + 2).
//   This only matters for the generic code's correctness; the fast code below
//   replicates the generic rounding exactly, midpoints included.
//
// Encoding
//   Within a binade the positive patterns are contiguous and evenly spaced, and
//   binades follow each other, so the magnitude of a positive double is
//   base[E] + RNE(top p bits of its fraction), E its biased exponent.  A carry
//   out of the fraction lands on base[E+1]; one past maxpos saturates.  The
//   lowest binade (c = -255) has base 0: the generic encoder rounds its M field
//   to nearest-even and M = 0 is the zero pattern, so its 15 patterns and its
//   rounding to zero fall out of the same formula.  Below 2^-255 is zero.
//
// Decoding
//   Within a DR field, the 11 C|M bits shifted left by 52 - p put C in the
//   double's exponent field and M at the top of its fraction, so a magnitude
//   decodes to  bits = K[DR] + ((mag & 0x7FF) << (52 - p)).  Two 16-entry tables.
//   At run time a 512 KB table of all 65536 decodes, built from that formula on
//   first use, is faster still: on an i7-8700B a multiply-add loop runs at 11.9 ns
//   (15.6 ns computed) and, with operands spread over 2^+/-60 so the table is
//   touched widely, 14.9 ns (15.7 ns).  A function-local static rather than a
//   namespace-scope one, so a takum built during another TU's static
//   initialization cannot read the table before it is filled.  Constant
//   evaluation uses the formula.

#ifndef TAKUM_FAST_TAKUM_16
#define TAKUM_FAST_TAKUM_16 0
#endif

#include <cstdint>
#include <type_traits>
#include <universal/utility/directives.hpp>

namespace sw { namespace universal {

#if TAKUM_FAST_TAKUM_16
UNIVERSAL_COMPILER_MESSAGE("Fast specialization of takum<16,3,uint16_t>")

namespace takum16_fast {

using Codec = takum_codec<16, 3>;

struct tables {
	std::uint64_t dec_k[16];        // (c_bias + 1023) << 52, per DR
	std::uint8_t  dec_s[16];        // 52 - p, per DR
	std::uint16_t enc_base[2048];   // magnitude of 2^c, per biased double exponent
	std::uint8_t  enc_shift[2048];  // 52 - p; 63 rounds any fraction to 0
};

constexpr tables make_tables() noexcept {
	tables t{};
	for (unsigned dr = 0; dr < 16; ++dr) {
		t.dec_k[dr] = static_cast<std::uint64_t>(Codec::dr_to_c_bias(dr) + 1023) << 52;
		t.dec_s[dr] = static_cast<std::uint8_t>(52 - Codec::layout_of(dr).p);
	}
	for (int E = 0; E < 2048; ++E) {
		std::int64_t c = E - 1023;
		if (c < Codec::min_characteristic())      { t.enc_base[E] = 0;      t.enc_shift[E] = 63; }
		else if (c > Codec::max_characteristic()) { t.enc_base[E] = 0x7FFF; t.enc_shift[E] = 63; }
		else {
			t.enc_base[E]  = static_cast<std::uint16_t>(Codec::encode_exact(c, 0).magnitude);
			t.enc_shift[E] = static_cast<std::uint8_t>(52 - Codec::layout_of(Codec::find_dr(c)).p);
		}
	}
	return t;
}

inline constexpr tables tbl = make_tables();

// raw pattern of the rounded double; NaN and +/-inf -> NaR
inline CONSTEXPRESSION std::uint16_t encode(double v) noexcept {
	std::uint64_t u = sw::bit_cast<std::uint64_t>(v);
	unsigned E = static_cast<unsigned>(u >> 52) & 0x7FFu;
	if (E == 0x7FFu) return 0x8000u;
	std::uint64_t f = u & 0x000F'FFFF'FFFF'FFFFull;
	unsigned s = tbl.enc_shift[E];
	std::uint64_t mag = tbl.enc_base[E] + ((f + (1ull << (s - 1)) - 1ull + ((f >> s) & 1ull)) >> s);
	if (mag > 0x7FFFu) mag = 0x7FFFu;
	mag |= (mag == 0) & ((u << 1) != 0);          // nonzero never rounds to zero: minpos (#1615)
	std::uint64_t sgn = u >> 63;                  // branch-free two's-complement negate
	return static_cast<std::uint16_t>((mag ^ (0 - sgn)) + sgn);
}

// value of a raw pattern: zero -> 0.0, NaR -> quiet NaN, so arithmetic needs no special cases
inline CONSTEXPRESSION double decode_formula(std::uint16_t raw) noexcept {
	std::uint64_t sgn = raw >> 15;
	unsigned mag = static_cast<std::uint16_t>((raw ^ (0u - unsigned(sgn))) + unsigned(sgn));
	unsigned dr  = (mag >> 11) & 0xFu;              // NaR has magnitude 0x8000
	std::uint64_t u = (tbl.dec_k[dr] + (static_cast<std::uint64_t>(mag & 0x7FFu) << tbl.dec_s[dr])) | (sgn << 63);
	if ((mag & 0x7FFFu) == 0) u = sgn ? 0x7FF8'0000'0000'0000ull : 0ull;
	return sw::bit_cast<double>(u);
}

struct decode_table {
	double v[65536];
	decode_table() noexcept { for (unsigned i = 0; i < 65536; ++i) v[i] = decode_formula(static_cast<std::uint16_t>(i)); }
};
inline const decode_table& table() noexcept { static const decode_table t; return t; }

inline CONSTEXPRESSION double decode(std::uint16_t raw) noexcept {
	if (std::is_constant_evaluated()) return decode_formula(raw);
	return table().v[raw];
}

} // namespace takum16_fast

using takum16_fast_t = takum<16, 3, std::uint16_t>;

template<> template<>
inline CONSTEXPRESSION takum16_fast_t& takum16_fast_t::convert_ieee754<double>(double rhs) noexcept {
	setbits(takum16_fast::encode(rhs));
	return *this;
}
// float -> double is exact and the generic encoder sees the same (c, m) either way
template<> template<>
inline CONSTEXPRESSION takum16_fast_t& takum16_fast_t::convert_ieee754<float>(float rhs) noexcept {
	setbits(takum16_fast::encode(static_cast<double>(rhs)));
	return *this;
}

template<> template<>
inline CONSTEXPRESSION double takum16_fast_t::to_ieee754<double>() const noexcept {
	return takum16_fast::decode(static_cast<std::uint16_t>(raw_bits()));
}
// takum16 -> double is exact, so one double -> float rounding is the correctly
// rounded float, subnormals included -- what the generic path computes (#1622).
template<> template<>
inline CONSTEXPRESSION float takum16_fast_t::to_ieee754<float>() const noexcept {
	return static_cast<float>(takum16_fast::decode(static_cast<std::uint16_t>(raw_bits())));
}

// Compound arithmetic: the generic operators minus their NaR and zero branches,
// which the decode above makes redundant (NaR -> NaN -> NaR, x/0 -> inf -> NaR).
template<>
inline CONSTEXPRESSION takum16_fast_t& takum16_fast_t::operator+=(const takum16_fast_t& rhs) {
	setbits(takum16_fast::encode(double(*this) + double(rhs)));
	return *this;
}
template<>
inline CONSTEXPRESSION takum16_fast_t& takum16_fast_t::operator-=(const takum16_fast_t& rhs) {
	setbits(takum16_fast::encode(double(*this) - double(rhs)));
	return *this;
}
template<>
inline CONSTEXPRESSION takum16_fast_t& takum16_fast_t::operator*=(const takum16_fast_t& rhs) {
	setbits(takum16_fast::encode(double(*this) * double(rhs)));
	return *this;
}
template<>
inline CONSTEXPRESSION takum16_fast_t& takum16_fast_t::operator/=(const takum16_fast_t& rhs) {
#if TAKUM_THROW_ARITHMETIC_EXCEPTION
	if (rhs.iszero() && !isnar() && !std::is_constant_evaluated()) throw takum_divide_by_zero();
#endif
	setbits(takum16_fast::encode(double(*this) / double(rhs)));
	return *this;
}

#endif // TAKUM_FAST_TAKUM_16

}} // namespace sw::universal
