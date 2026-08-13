#pragma once
// takum_log_constants.hpp: mathematical constants for the logarithmic takum
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A logarithmic takum stores |value| = sqrt(e)^l, so a constant X is held as its
// logarithmic value l_X = 2 ln X.  The table below carries each l_X as an exact
// binary fraction rather than as a real literal, and the accessor encodes it at
// whatever width the target type has.
//
// Why not simply write TakumLog(3.14159...)?  Because a double literal caps the
// constant at 53 significant bits of l, and takum_log<64,3> has 58 or 59 fraction
// bits.  Constructing from a double is then wrong in the low bits -- measured
// against a 113-bit reference, at nbits = 64:
//
//     sqrt(2)   50 ulp off      ln(2)   24 ulp off
//     phi       27 ulp off      pi       6 ulp off
//
// Carrying (c, N) as integers and letting takum_codec::encode_fraction() do the
// single rounding gives the correctly rounded constant at every width.  This is
// the same reason the math library works in the logarithmic domain; see
// takum_log_domain.hpp.
//
// A few constants are EXACT here, which no binary floating-point format manages:
// the value base is sqrt(e), so e is sqrt(e)^2, 1/e is sqrt(e)^-2 and sqrt(e) is
// sqrt(e)^1.  Their table entries have a zero fraction and encode without any
// rounding at all, at every width.
//
// The bit patterns are disjoint from the linear takum's: the two types share an
// encoding but not a value map, so the same mathematical constant lands on
// different bits in each.
#include <cstdint>
#include <universal/number/takum/takum_codec.hpp>

namespace sw { namespace universal {

// A constant expressed as its exact logarithmic value l = c + N / 2^qbits.
// Post: N < 2^qbits.
struct takum_log_constant {
	int64_t  c;   // integer part of l
	uint64_t N;   // fraction numerator
};

// Fraction width of the table.  Chosen above the widest trailing field the codec
// can produce (p = 59 at nbits = 64) so that every supported width narrows rather
// than widens, and verified against a 113-bit reference at every width -- see
// static/tapered/takum_log/math/constants.cpp, which rejects a table entry that
// does not reproduce the correctly rounded encoding.
inline constexpr unsigned takum_log_constant_qbits = 62;

// ---------------------------------------------------------------------------
// The table.  l = 2 ln X, rounded to nearest-even at 2^-62, computed at 80
// decimal digits.  a_b reads a over b, as in 1_pi being 1 over pi.
// ---------------------------------------------------------------------------

inline constexpr takum_log_constant tkml_pi_4     = {   -1,  2383646992120049331ull };  // pi/4
inline constexpr takum_log_constant tkml_pi_3     = {    0,   425359756248847267ull };  // pi/3
inline constexpr takum_log_constant tkml_pi_2     = {    0,  4165115296293989257ull };  // pi/2
inline constexpr takum_log_constant tkml_3pi_4    = {    1,  3293184817911743342ull };  // 3*pi/4
inline constexpr takum_log_constant tkml_pi       = {    2,  1334897582040541279ull };  // pi
inline constexpr takum_log_constant tkml_2pi      = {    3,  3116365886214481205ull };  // 2*pi
inline constexpr takum_log_constant tkml_3pi      = {    4,  2244435407832235290ull };  // 3*pi
inline constexpr takum_log_constant tkml_4pi      = {    5,   286148171961033226ull };  // 4*pi

inline constexpr takum_log_constant tkml_4_pi     = {    0,  2228039026307338573ull };  // 4/pi
inline constexpr takum_log_constant tkml_3_pi     = {   -1,  4186326262178540637ull };  // 3/pi
inline constexpr takum_log_constant tkml_2_pi     = {   -1,   446570722133398647ull };  // 2/pi
inline constexpr takum_log_constant tkml_1_pi     = {   -3,  3276788436386846625ull };  // 1/pi
inline constexpr takum_log_constant tkml_2_sqrtpi = {    0,  1114019513153669287ull };  // 2/sqrt(pi)

inline constexpr takum_log_constant tkml_sqrt2    = {    0,  3196577161300663915ull };  // sqrt(2)
inline constexpr takum_log_constant tkml_1_sqrt2  = {   -1,  1415108857126723989ull };  // 1/sqrt(2)
inline constexpr takum_log_constant tkml_sqrt3    = {    1,   454768912895847006ull };  // sqrt(3)
inline constexpr takum_log_constant tkml_sqrt5    = {    1,  2810536299871915436ull };  // sqrt(5)

inline constexpr takum_log_constant tkml_phi      = {    0,  4438395691058598697ull };  // golden ratio
inline constexpr takum_log_constant tkml_1_phi    = {   -1,   173290327368789207ull };  // 1/phi

// Exact in this format: powers of the value base sqrt(e) have an integer l.
inline constexpr takum_log_constant tkml_e        = {    2,                    0ull };  // e       == sqrt(e)^2
inline constexpr takum_log_constant tkml_1_e      = {   -2,                    0ull };  // 1/e     == sqrt(e)^-2
inline constexpr takum_log_constant tkml_sqrt_e   = {    1,                    0ull };  // sqrt(e) == sqrt(e)^1

inline constexpr takum_log_constant tkml_e_gamma  = {   -2,  4154766504347488262ull };  // Euler-Mascheroni gamma
inline constexpr takum_log_constant tkml_log2e    = {    0,  3380485022838897986ull };  // log2(e)
inline constexpr takum_log_constant tkml_log10e   = {   -2,  1530780503525168441ull };  // log10(e)
inline constexpr takum_log_constant tkml_ln2      = {   -1,  1231200995588489918ull };  // ln(2)
inline constexpr takum_log_constant tkml_ln3      = {    0,   867438103366800040ull };  // ln(3)
inline constexpr takum_log_constant tkml_ln4      = {    0,  3012669299762429844ull };  // ln(4) == 2 ln(2)
inline constexpr takum_log_constant tkml_ln10     = {    1,  3080905514902219463ull };  // ln(10)

// ---------------------------------------------------------------------------
// Accessor
// ---------------------------------------------------------------------------

// Encode a tabulated constant at the target type's width, correctly rounded.
//
// Every constant here is positive, so the sign bit is never set; negate the
// result for -pi and friends.  A constant outside a narrow configuration's range
// saturates the way conversion from a native float does -- maxpos on overflow,
// zero on underflow -- rather than trapping.
template<typename TakumLog>
inline TakumLog takum_log_constant_cast(const takum_log_constant& k) noexcept {
	using Codec = typename TakumLog::Codec;
	TakumLog result;
	auto enc = Codec::encode_fraction(k.c, k.N, takum_log_constant_qbits);
	if (enc.overflowed())  { result.maxpos();  return result; }
	if (enc.underflowed()) { result.setzero(); return result; }
	result.setbits(enc.magnitude);
	return result;
}

}} // namespace sw::universal
