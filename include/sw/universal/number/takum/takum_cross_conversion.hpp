#pragma once
// takum_cross_conversion.hpp: exact conversion between the linear and logarithmic takum
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// takum<> and takum_log<> share an encoding but not a value map, so the same bit
// pattern denotes different numbers and converting between them is an arithmetic
// operation, not a reinterpretation:
//
//     takum<>      |value| = (1 + f) * 2^E
//     takum_log<>  |value| = sqrt(e)^l = e^(l/2)
//
// Equating the two gives  l * log2(sqrt(e)) = E + log2(1 + f).  Both directions
// therefore need a transcendental, and neither result is exact for a rational
// input -- log2(sqrt(e)) is irrational, so no non-trivial encoding maps onto
// another exactly.  The only question is how accurately the rounding is decided.
//
// A double is not accurate enough.  Measured against a 113-bit reference, taking
// the obvious route -- takum<n,3>{ double(takum_log<n,3>) } -- is correctly
// rounded through 48 bits and then collapses:
//
//     nbits 16, 24, 32, 48    correctly rounded
//     nbits 64                WRONG for 96.75% of inputs, by up to 4 ulp
//
// because takum<64,3> reaches p = 59 and a double carries 53 significant bits.
//
// So the work is done in dd_cascade, whose ~106 bits leave thirteen orders of
// margin: its exp2 is accurate to 1.0e-32 and its log1p to 7.6e-31 against the
// same reference, where 2^-60 is 8.7e-19.  The logarithmic value is carried in
// and out as an exact integer pair rather than as a double, for the same reason
// the math library does it -- see takum_log_domain.hpp.
//
// DEPENDENCY: this is the only takum header that reaches outside the takum family,
// and it is deliberately NOT included by takum.hpp or takum_log.hpp.  Include it
// explicitly when you need cross-conversion, and pay for dd_cascade then.
#include <cstdint>
#include <universal/number/takum/takum.hpp>
#include <universal/number/takum/takum_log.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>

namespace sw { namespace universal {

namespace takum_xc {

// Fraction width used to hand values to encode_fraction().  Above the widest
// trailing field the codec can produce (p = 59 at nbits = 64), so every supported
// width narrows rather than widens.
inline constexpr unsigned qbits = 62;

// Build c + M/2^p exactly.
//
// M reaches 2^59, past a double's 53 bits, so it is split into halves that are
// each exactly representable; scaling by a power of two is exact, and dd_cascade
// addition is exact while the result fits its ~106 bits.  It always does here:
// the characteristic contributes at most 33 bits and the fraction at most 59.
inline dd_cascade exact_value(int64_t c, uint64_t M, unsigned p) {
	dd_cascade result(static_cast<double>(c));
	if (p == 0 || M == 0) return result;
	const uint64_t hi = M >> 32;
	const uint64_t lo = M & 0xFFFFFFFFull;
	const double scale = std::ldexp(1.0, -static_cast<int>(p));
	if (hi != 0) result = result + dd_cascade(static_cast<double>(hi) * std::ldexp(1.0, 32) * scale);
	if (lo != 0) result = result + dd_cascade(static_cast<double>(lo) * scale);
	return result;
}

// Split a value into an integer part and a qbits fraction numerator, ROUNDED TO
// ODD.
//
// The obvious thing -- round the fraction to nearest at 2^-62 and let
// encode_fraction() round again to the layout's p bits -- rounds twice, and the
// two roundings disagree whenever the exact value sits near a tie at the p-bit
// boundary.  That is not a rare corner: measured at nbits = 64, plain
// round-to-nearest here left 1.8% of conversions off by an ulp, which is the only
// error that survived moving the arithmetic to dd_cascade at all.
//
// Round-to-odd fixes it.  Truncate, and if anything was discarded set the low bit;
// a subsequent round-to-nearest-even to any width at least two bits narrower then
// gives the same answer as rounding the exact value directly.  qbits is 62 and p
// reaches 59, so there are three bits of headroom.
struct split {
	int64_t  c;
	uint64_t N;
};
inline split to_integer_fraction(const dd_cascade& v) {
	dd_cascade fl = floor(v);
	int64_t c = static_cast<int64_t>(fl[0]) + static_cast<int64_t>(fl[1]);
	dd_cascade frac = v - fl;                                  // in [0,1), exact
	dd_cascade scaled = frac * dd_cascade(std::ldexp(1.0, static_cast<int>(qbits)));

	// truncate: floor() of a dd_cascade is exact, and both limbs are then integers
	dd_cascade sfl = floor(scaled);
	int64_t N = static_cast<int64_t>(sfl[0]) + static_cast<int64_t>(sfl[1]);
	dd_cascade rem = scaled - sfl;
	const bool sticky = (rem[0] != 0.0) || (rem[1] != 0.0);

	const int64_t limit = static_cast<int64_t>(1ull << qbits);
	if (N < 0) N = 0;
	if (N >= limit) { N -= limit; ++c; }
	if (sticky) N |= 1;                                        // round to odd
	return split{ c, static_cast<uint64_t>(N) };
}

// log2(sqrt(e)) == log2(e)/2, exactly halved.
inline dd_cascade log2_of_sqrt_e() {
	dd_cascade r = ddc_lge;
	r = r * dd_cascade(0.5);      // exact: scaling by a power of two
	return r;
}

} // namespace takum_xc

// ---------------------------------------------------------------------------
// logarithmic -> linear
// ---------------------------------------------------------------------------

// |x| = e^(l/2), so log2|x| = l * log2(sqrt(e)); the linear takum wants that split
// into an integer characteristic and a fraction f with 1 + f = 2^(frac).
template<typename TargetTakum, unsigned nbits, unsigned rbits, typename bt>
inline TargetTakum takum_convert(const takum_log<nbits, rbits, bt>& x) {
	using Codec = typename TargetTakum::Codec;
	TargetTakum result;
	if (x.isnar())  { result.setnar();  return result; }
	if (x.iszero()) { result.setzero(); return result; }

	auto d = takum_log<nbits, rbits, bt>::Codec::decode(x.magnitude_bits());
	dd_cascade l     = takum_xc::exact_value(d.c, d.M_bits, d.p);
	dd_cascade log2v = l * takum_xc::log2_of_sqrt_e();

	dd_cascade E = floor(log2v);
	dd_cascade t = log2v - E;                       // in [0,1)
	dd_cascade f = exp2(t) - dd_cascade(1.0);       // in [0,1), the linear fraction

	// exp2 can land a hair outside [0,1) at the endpoints; fold that into E rather
	// than handing encode_fraction() an out-of-range numerator.
	int64_t Ei = static_cast<int64_t>(double(E));
	auto s = takum_xc::to_integer_fraction(f);
	Ei += s.c;                                      // s.c is 0, or 1 if f reached 1

	auto enc = Codec::encode_fraction(Ei, s.N, takum_xc::qbits);
	if (enc.overflowed())  { if (x.sign()) result.maxneg(); else result.maxpos(); return result; }
	if (enc.underflowed()) { result.setzero(); return result; }
	uint64_t raw = x.sign() ? (((~enc.magnitude) + 1ull) & Codec::nbits_mask()) : enc.magnitude;
	result.setbits(raw);
	return result;
}

// ---------------------------------------------------------------------------
// linear -> logarithmic
// ---------------------------------------------------------------------------

// |x| = (1 + f) * 2^E, so l = 2 ln|x| = 2 ln(1 + f) + 2 E ln2.
template<typename TargetTakumLog, unsigned nbits, unsigned rbits, typename bt>
inline TargetTakumLog takum_convert(const takum<nbits, rbits, bt>& x) {
	using Codec = typename TargetTakumLog::Codec;
	TargetTakumLog result;
	if (x.isnar())  { result.setnar();  return result; }
	if (x.iszero()) { result.setzero(); return result; }

	auto d = takum<nbits, rbits, bt>::Codec::decode(x.magnitude_bits());
	// the fraction alone, exactly: characteristic zero, trailing field as given
	dd_cascade f = takum_xc::exact_value(0, d.M_bits, d.p);
	dd_cascade l = (log1p(f) + dd_cascade(static_cast<double>(d.c)) * ddc_ln2) * dd_cascade(2.0);

	auto s = takum_xc::to_integer_fraction(l);
	auto enc = Codec::encode_fraction(s.c, s.N, takum_xc::qbits);
	if (enc.overflowed())  { if (x.sign()) result.maxneg(); else result.maxpos(); return result; }
	if (enc.underflowed()) { result.setzero(); return result; }
	uint64_t raw = x.sign() ? (((~enc.magnitude) + 1ull) & Codec::nbits_mask()) : enc.magnitude;
	result.setbits(raw);
	return result;
}

}} // namespace sw::universal
