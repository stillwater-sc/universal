#pragma once
// wide_range.hpp: math functions for takum formats whose range exceeds double's
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// At rbits = 4 and 5 a takum spans 2^+/-65535 or 2^+/-2^32 against double's 2^+/-1024,
// and the elementary functions evaluate in a double.  Three things go wrong (#1626):
//
//   the result leaves double's range   exp(800) is an ordinary takum<32,4>, but
//                                      std::exp overflows and the takum is NaR
//   the argument leaves double's range double(2^2000) is infinity, so log(2^2000),
//                                      whose true value is ~1386, comes back NaR
//   the argument is tiny               double(2^-2000) is 0, so sin(2^-2000) is 0
//                                      and floor(-2^-2000) is 0 instead of -1
//
// Everything here rests on two primitives that never leave double's range:
//
//   ln_abs(x)   ln|x| as a double-double.  The codec hands over the characteristic
//               exactly, so the linear takum gives c ln2 + log1p(f) and the
//               logarithmic one gives l/2 outright -- a moderate number even when
//               |x| is 2^(2^32).
//   from_ln(L)  the takum whose magnitude is e^L.  L is split into an integer power
//               of two (or, logarithmic, an integer characteristic) and a bounded
//               remainder that a double evaluates; the codec assembles the result
//               from an int64_t characteristic, and saturates where the true value
//               is beyond maxpos or below minpos -- which, unlike in a double, is
//               then a statement about the takum's own range.
//
// The functions below return std::nullopt whenever the plain double evaluation is
// sound -- arguments and result inside double's normal range -- so for those inputs
// nothing changes.  Formats whose range fits double (every rbits <= 3) never call in
// at all; the callers test range_fits_double at compile time.
//
// Deliberately NOT handled, left as the double path returns them:
//   sin, cos, tan, sec, csc, cot of a huge argument -- NaR.  Reducing 2^65535 modulo
//     pi exactly needs ~65000 bits of 2/pi (Payne-Hanek); at rbits = 5, 2^32 times
//     that.  The value would also carry no information at the precision such an
//     argument has.
//   takum_log fmod and remainder with a huge quotient -- NaR.  Logarithmic values are
//     not dyadic, so there is no exact integer remainder to take.  The linear takum
//     gets one (fmod_exact below).

#include <algorithm>   // std::max, std::min
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <universal/number/takum/takum_traits.hpp>
#include <universal/number/takum/takum_wide_arithmetic.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>

namespace sw { namespace universal { namespace takum_wide_range {

// Constants split as hi + lo, each hi the nearest double and lo the rest.
inline constexpr double ln2_hi               = 0x1.62e42fefa39efp-1;
inline constexpr double ln2_lo               = 0x1.abc9e3b39803fp-56;
inline constexpr double inv_ln2_hi           = 0x1.71547652b82fep+0;
inline constexpr double inv_ln2_lo           = 0x1.777d0ffda0d24p-56;
inline constexpr double log2_10_hi           = 0x1.a934f0979a371p+1;
inline constexpr double log2_10_lo           = 0x1.7f2495fb7fa6dp-53;
inline constexpr double inv_ln10_hi          = 0x1.bcb7b1526e50ep-2;
inline constexpr double inv_ln10_lo          = 0x1.95355baaafad3p-57;
inline constexpr double half_ln_pi_hi        = 0x1.250d048e7a1bdp-1;   // ln(pi) / 2
inline constexpr double half_ln_pi_lo        = 0x1.7abf2ad8d5088p-58;
inline constexpr double ln_2_over_sqrt_pi_hi = 0x1.eeb95b094c191p-4;   // ln(2 / sqrt(pi))
inline constexpr double ln_2_over_sqrt_pi_lo = 0x1.346863f58b075p-58;

// ---------------------------------------------------------------------------
// double-double, just what the reductions need
// ---------------------------------------------------------------------------
struct dd { double hi; double lo; };

inline dd two_sum(double a, double b) {
	const double s = a + b, bb = s - a;
	return dd{ s, (a - (s - bb)) + (b - bb) };
}
inline dd add(const dd& a, const dd& b) {
	dd s = two_sum(a.hi, b.hi);
	s.lo += a.lo + b.lo;
	return two_sum(s.hi, s.lo);
}
inline dd mul(const dd& a, const dd& b) {
	const double p = a.hi * b.hi;
	const double e = std::fma(a.hi, b.hi, -p) + (a.hi * b.lo + a.lo * b.hi);
	return two_sum(p, e);
}
inline dd neg(const dd& a) { return dd{ -a.hi, -a.lo }; }

// ---------------------------------------------------------------------------
// classification
// ---------------------------------------------------------------------------
template<typename T>
int64_t characteristic(const T& x) { return T::Codec::characteristic_of(x.magnitude_bits()); }

// Outside the band the double path handles: characteristics beyond +/-510 (#1627).
template<typename T>
bool is_tiny(const T& x) { return !x.iszero() && !x.isnar() && characteristic(x) < -T::double_safe_characteristic; }
template<typename T>
bool is_huge(const T& x) { return !x.iszero() && !x.isnar() && characteristic(x) >  T::double_safe_characteristic; }
template<typename T>
bool is_far(const T& x) { return is_tiny(x) || is_huge(x); }

template<typename T>
T saturated(bool high, bool negative) {
	T r;
	if (high) { if (negative) r.maxneg(); else r.maxpos(); }
	else      { if (negative) r.minneg(); else r.minpos(); }
	return r;
}

// ---------------------------------------------------------------------------
// the two primitives
// ---------------------------------------------------------------------------

// ln|x| for finite, nonzero x.  Error: one rounding of the trailing field to 53
// bits, plus a double's log1p for the linear takum -- far below the precision any
// result built from it can carry.
template<typename T>
dd ln_abs(const T& x) {
	const auto d = T::Codec::decode(x.magnitude_bits());
	const double c = static_cast<double>(d.c);
	const double f = d.fraction();
	if constexpr (is_takum_log<T>) {
		return two_sum(c * 0.5, f * 0.5);                       // l / 2, l = c + f
	}
	else {
		return add(mul(dd{ c, 0.0 }, dd{ ln2_hi, ln2_lo }), dd{ std::log1p(f), 0.0 });
	}
}

template<typename T>
T assemble(const typename T::Codec::encoded& enc, bool negative) {
	if (enc.overflowed())  return saturated<T>(true,  negative);
	if (enc.underflowed()) return saturated<T>(false, negative);
	T r;
	r.setbits(negative ? (((~enc.magnitude) + 1ull) & T::Codec::nbits_mask()) : enc.magnitude);
	return r;
}

template<typename T> T from_ln(const dd& L, bool negative);

// The takum whose magnitude is 2^t.
template<typename T>
T from_log2(const dd& t, bool negative) {
	using Codec = typename T::Codec;
	if constexpr (is_takum_log<T>) {
		return from_ln<T>(mul(t, dd{ ln2_hi, ln2_lo }), negative);
	}
	else {
		if (!(t.hi < static_cast<double>(Codec::max_characteristic()) + 2.0)) return saturated<T>(true,  negative);
		if (t.hi < static_cast<double>(Codec::min_characteristic()) - 2.0)    return saturated<T>(false, negative);
		const double k = std::nearbyint(t.hi);
		const double r = (t.hi - k) + t.lo;                     // |r| <= ~1/2, exact subtraction
		int j = 0;
		const double fr = std::frexp(std::exp2(r), &j);          // 2^r in [0.70, 1.42]
		return assemble<T>(Codec::encode_rounded(static_cast<int64_t>(k) + j - 1, 2.0 * fr - 1.0), negative);
	}
}

// The takum whose magnitude is e^L.
template<typename T>
T from_ln(const dd& L, bool negative) {
	using Codec = typename T::Codec;
	if constexpr (is_takum_log<T>) {
		const dd l{ 2.0 * L.hi, 2.0 * L.lo };                   // l = 2 ln|v|, exact doubling
		if (!(l.hi < static_cast<double>(Codec::max_characteristic()) + 2.0)) return saturated<T>(true,  negative);
		if (l.hi < static_cast<double>(Codec::min_characteristic()) - 2.0)    return saturated<T>(false, negative);
		double c = std::floor(l.hi);
		double m = (l.hi - c) + l.lo;
		if (m < 0.0)  { m += 1.0; c -= 1.0; }
		if (m >= 1.0) { m -= 1.0; c += 1.0; }
		return assemble<T>(Codec::encode_rounded(static_cast<int64_t>(c), m), negative);
	}
	else {
		return from_log2<T>(mul(L, dd{ inv_ln2_hi, inv_ln2_lo }), negative);
	}
}

// r * 2^k for a finite, nonzero double r: one rounding, from the double straight to
// the takum, however far 2^k reaches.
template<typename T>
T scaled(double r, int64_t k) {
	const bool negative = (r < 0.0);
	const double a = std::fabs(r);
	if constexpr (is_takum_log<T>) {
		return from_ln<T>(add(dd{ std::log(a), 0.0 }, mul(dd{ static_cast<double>(k), 0.0 }, dd{ ln2_hi, ln2_lo })), negative);
	}
	else {
		int j = 0;
		const double fr = std::frexp(a, &j);
		return assemble<T>(T::Codec::encode_rounded(static_cast<int64_t>(j) - 1 + k, 2.0 * fr - 1.0), negative);
	}
}

// floor(log2|x|), or close to it for the logarithmic takum: a scale that brings x
// near 1.  Pre: x finite, nonzero.
template<typename T>
int64_t binary_exponent(const T& x) {
	if constexpr (is_takum_log<T>) return static_cast<int64_t>(std::floor(ln_abs(x).hi * inv_ln2_hi));
	else                           return characteristic(x);
}

// x * 2^-k as a double.  Pre: x finite, nonzero, and the result in double's range.
template<typename T>
double scaled_down(const T& x, int64_t k) {
	double v = 0.0;
	if constexpr (is_takum_log<T>) {
		const dd L = add(ln_abs(x), neg(mul(dd{ static_cast<double>(k), 0.0 }, dd{ ln2_hi, ln2_lo })));
		v = std::exp(L.hi + L.lo);
	}
	else {
		const auto d = T::Codec::decode(x.magnitude_bits());
		const int64_t s = d.c - k;
		v = (s < -1100) ? 0.0 : std::ldexp(1.0 + d.fraction(), static_cast<int>(s));
	}
	return x.sign() ? -v : v;
}

template<typename T>
T absolute(const T& x) { return x.sign() ? -x : x; }

// ---------------------------------------------------------------------------
// exponentials
// ---------------------------------------------------------------------------
template<typename T>
std::optional<T> exp(const T& x) {
	if (x.isnar() || x.iszero()) return std::nullopt;
	if (is_tiny(x)) return T(1.0);
	if (is_huge(x)) return saturated<T>(!x.sign(), false);   // e^(+/-2^511) is past either end
	const double dx = double(x);
	if (dx > -708.0 && dx < 709.0) return std::nullopt;     // a normal double result
	return from_ln<T>(dd{ dx, 0.0 }, false);
}

template<typename T>
std::optional<T> exp2(const T& x) {
	if (x.isnar() || x.iszero()) return std::nullopt;
	if (is_tiny(x)) return T(1.0);
	if (is_huge(x)) return saturated<T>(!x.sign(), false);
	const double dx = double(x);
	if (dx > -1022.0 && dx < 1023.0) return std::nullopt;
	return from_log2<T>(dd{ dx, 0.0 }, false);
}

template<typename T>
std::optional<T> exp10(const T& x) {
	if (x.isnar() || x.iszero()) return std::nullopt;
	if (is_tiny(x)) return T(1.0);
	if (is_huge(x)) return saturated<T>(!x.sign(), false);
	const double dx = double(x);
	if (dx > -307.0 && dx < 308.0) return std::nullopt;
	return from_log2<T>(mul(dd{ dx, 0.0 }, dd{ log2_10_hi, log2_10_lo }), false);
}

template<typename T>
std::optional<T> expm1(const T& x) {
	if (x.isnar() || x.iszero()) return std::nullopt;
	if (is_tiny(x)) return x;                                // e^x - 1 = x + x^2/2 + ...
	if (is_huge(x)) return x.sign() ? std::optional<T>{} : std::optional<T>{ saturated<T>(true, false) };   // -inf -> -1 in double
	const double dx = double(x);
	if (dx < 709.0) return std::nullopt;
	return from_ln<T>(dd{ dx, 0.0 }, false);                 // e^x - 1 = e^x at this size
}

// ---------------------------------------------------------------------------
// hyperbolic
// ---------------------------------------------------------------------------
template<typename T>
std::optional<T> sinh(const T& x) {
	if (x.isnar() || x.iszero()) return std::nullopt;
	if (is_tiny(x)) return x;
	if (is_huge(x)) return saturated<T>(true, x.sign());
	const double ax = std::fabs(double(x));
	if (ax < 709.0) return std::nullopt;
	return from_ln<T>(add(dd{ ax, 0.0 }, dd{ -ln2_hi, -ln2_lo }), x.sign());   // e^|x| / 2
}

template<typename T>
std::optional<T> cosh(const T& x) {
	if (x.isnar() || x.iszero()) return std::nullopt;
	if (is_tiny(x)) return T(1.0);
	if (is_huge(x)) return saturated<T>(true, false);
	const double ax = std::fabs(double(x));
	if (ax < 709.0) return std::nullopt;
	return from_ln<T>(add(dd{ ax, 0.0 }, dd{ -ln2_hi, -ln2_lo }), false);
}

template<typename T>
std::optional<T> asinh(const T& x) {
	if (x.isnar() || x.iszero()) return std::nullopt;
	if (is_tiny(x)) return x;
	if (!is_huge(x)) return std::nullopt;
	const dd L = add(ln_abs(x), dd{ ln2_hi, ln2_lo });        // ln(2|x|)
	return T(x.sign() ? -L.hi : L.hi);
}

template<typename T>
std::optional<T> acosh(const T& x) {
	if (!is_huge(x) || x.sign()) return std::nullopt;        // tiny or negative: NaR via double
	return T(add(ln_abs(x), dd{ ln2_hi, ln2_lo }).hi);
}

// ---------------------------------------------------------------------------
// tiny arguments: an odd function with f'(0) = 1 returns x, an even one 1.  For
// |x| < 2^-510 the next term is below 2^-1020 relative, past any takum's precision.
// ---------------------------------------------------------------------------
template<typename T>
std::optional<T> identity_if_tiny(const T& x) { return is_tiny(x) ? std::optional<T>{ x } : std::nullopt; }

template<typename T>
std::optional<T> one_if_tiny(const T& x) { return is_tiny(x) ? std::optional<T>{ T(1.0) } : std::nullopt; }

// 1/x for tiny x (cot, csc): the reciprocal is an ordinary huge takum
template<typename T>
std::optional<T> reciprocal_if_tiny(const T& x) { return is_tiny(x) ? std::optional<T>{ T(1.0) / x } : std::nullopt; }

// ---------------------------------------------------------------------------
// logarithms: the result is always a moderate number
// ---------------------------------------------------------------------------
template<typename T>
std::optional<T> log(const T& x) {
	if (!is_far(x) || x.sign()) return std::nullopt;
	return T(ln_abs(x).hi);
}
template<typename T>
std::optional<T> log2(const T& x) {
	if (!is_far(x) || x.sign()) return std::nullopt;
	return T(mul(ln_abs(x), dd{ inv_ln2_hi, inv_ln2_lo }).hi);
}
template<typename T>
std::optional<T> log10(const T& x) {
	if (!is_far(x) || x.sign()) return std::nullopt;
	return T(mul(ln_abs(x), dd{ inv_ln10_hi, inv_ln10_lo }).hi);
}
template<typename T>
std::optional<T> log1p(const T& x) {
	if (is_tiny(x)) return x;
	if (is_huge(x) && !x.sign()) return T(ln_abs(x).hi);    // 1 + x == x at 2^511
	return std::nullopt;
}

// ---------------------------------------------------------------------------
// pow
// ---------------------------------------------------------------------------

// |x|^y with y given as a double plus its classification, for any nonzero x.
template<typename T>
std::optional<T> pow_core(const T& x, double dy, bool yHuge, bool yTiny, bool yNegative) {
	if (x.isnar() || x.iszero()) return std::nullopt;
	const bool far = is_far(x) || yHuge || yTiny;
	if (!far) {
		const double r = std::pow(double(x), dy);
		if (std::isnan(r) || std::fabs(r) >= std::numeric_limits<double>::min()) {
			if (!std::isinf(r)) return std::nullopt;               // a normal double result, or NaN
		}
	}
	// a negative base needs an integer exponent; huge exponents are even integers
	bool negative = false;
	if (x.sign()) {
		if (yTiny) { T nar; nar.setnar(); return nar; }
		if (!yHuge) {
			if (dy != std::trunc(dy)) { T nar; nar.setnar(); return nar; }
			negative = (std::fmod(dy, 2.0) != 0.0);
		}
	}
	if (yTiny) return T(1.0);                                     // |x|^tiny: ln|x| <= 2^32, times 2^-511
	const dd L = ln_abs(x);
	if (yHuge) {
		if (L.hi == 0.0 && L.lo == 0.0) return T(1.0);            // |x| == 1
		return saturated<T>((L.hi > 0.0) != yNegative, false);
	}
	return from_ln<T>(mul(L, dd{ dy, 0.0 }), negative);
}

template<typename T>
std::optional<T> pow(const T& x, const T& y) {
	if (y.isnar() || y.iszero()) return std::nullopt;
	return pow_core(x, double(y), is_huge(y), is_tiny(y), y.sign());
}
template<typename T>
std::optional<T> pow(const T& x, double y) {
	if (y == 0.0 || std::isnan(y) || std::isinf(y)) return std::nullopt;
	return pow_core(x, y, false, false, y < 0.0);
}

// ---------------------------------------------------------------------------
// error functions
// ---------------------------------------------------------------------------
template<typename T>
std::optional<T> erf(const T& x) {
	if (!is_tiny(x)) return std::nullopt;                         // huge: +/-1 in double
	return from_ln<T>(add(ln_abs(x), dd{ ln_2_over_sqrt_pi_hi, ln_2_over_sqrt_pi_lo }), x.sign());   // 2x / sqrt(pi)
}

// erfc(x) = e^(-x^2) / (x sqrt(pi)) * S,  S = sum_n (-1)^n (2n-1)!! / (2x^2)^n.
// Asymptotic, and used only where std::erfc leaves double's normal range (x > 26.5):
// there 2x^2 > 1400 and eight terms are accurate to ~1e-19.
template<typename T>
std::optional<T> erfc(const T& x) {
	if (x.isnar() || x.iszero()) return std::nullopt;
	if (is_tiny(x)) return T(1.0);
	if (x.sign()) return std::nullopt;                            // erfc(-x) -> 2
	if (is_huge(x)) return saturated<T>(false, false);
	const double dx = double(x);
	if (std::erfc(dx) >= std::numeric_limits<double>::min()) return std::nullopt;
	const double u = 1.0 / (2.0 * dx * dx);
	double term = 1.0, s = 0.0;
	for (int n = 1; n <= 8; ++n) { term *= -static_cast<double>(2 * n - 1) * u; s += term; }   // S - 1
	const double xx = dx * dx;
	dd L = dd{ -xx, -std::fma(dx, dx, -xx) };                    // -x^2, exactly
	L = add(L, dd{ -std::log(dx), 0.0 });
	L = add(L, dd{ -half_ln_pi_hi, -half_ln_pi_lo });
	L = add(L, dd{ std::log1p(s), 0.0 });
	return from_ln<T>(L, false);
}

// ---------------------------------------------------------------------------
// hypot and atan2: scale both arguments by the same power of two
// ---------------------------------------------------------------------------
template<typename T>
std::optional<T> hypot(const T& x, const T& y) {
	if (x.isnar() || y.isnar()) return std::nullopt;
	if (!is_far(x) && !is_far(y)) return std::nullopt;
	if (x.iszero()) return absolute(y);
	if (y.iszero()) return absolute(x);
	const int64_t k = std::max(binary_exponent(x), binary_exponent(y));
	return scaled<T>(std::hypot(scaled_down(x, k), scaled_down(y, k)), k);
}

// When |y/x| < 2^-30 the angle is y/x for x > 0 (atan q = q - q^3/3, and q^2/3 is
// below a double's precision) or +/-pi for x < 0.  Taking that branch matters: the
// common scaling would push the smaller argument below double's range, and
// atan2(2^-2000, 2^2000) would come back 0 rather than 2^-4000.
template<typename T>
std::optional<T> atan2(const T& y, const T& x) {
	if (x.isnar() || y.isnar() || x.iszero() || y.iszero()) return std::nullopt;
	if (!is_far(x) && !is_far(y)) return std::nullopt;
	const T q = y / x;
	if (!q.iszero() && !q.isnar() && binary_exponent(q) < -30) {
		if (!x.sign()) return q;
		constexpr double pi = 3.141592653589793;
		return T(y.sign() ? -pi : pi);
	}
	const int64_t k = std::max(binary_exponent(x), binary_exponent(y));
	return T(std::atan2(scaled_down(y, k), scaled_down(x, k)));
}

// ---------------------------------------------------------------------------
// rounding to an integer
// ---------------------------------------------------------------------------
enum class integral_mode { trunc, round, floor, ceil };

// A huge takum is already an integer; a tiny one rounds to 0 or +/-1.
template<typename T>
std::optional<T> integral(const T& x, integral_mode mode) {
	if (is_huge(x)) return x;
	if (!is_tiny(x)) return std::nullopt;
	switch (mode) {
	case integral_mode::floor: return x.sign() ? T(-1.0) : T(0.0);
	case integral_mode::ceil:  return x.sign() ? T(0.0)  : T(1.0);
	default:                   return T(0.0);
	}
}

// frac(x) = x - trunc(x)
template<typename T>
std::optional<T> frac(const T& x) {
	if (is_huge(x)) return T(0.0);
	return identity_if_tiny(x);
}

// ---------------------------------------------------------------------------
// fmod and remainder
// ---------------------------------------------------------------------------

// (a * b) mod m and 2^e mod m, for m < 2^63, in 64-bit arithmetic.
inline uint64_t mulmod(uint64_t a, uint64_t b, uint64_t m) {
	uint64_t r = 0;
	a %= m;
	while (b != 0) {
		if (b & 1ull) { r += a; if (r >= m) r -= m; }
		a <<= 1; if (a >= m) a -= m;
		b >>= 1;
	}
	return r;
}
inline uint64_t pow2mod(uint64_t e, uint64_t m) {
	uint64_t result = 1ull % m, base = 2ull % m;
	while (e != 0) {
		if (e & 1ull) result = mulmod(result, base, m);
		base = mulmod(base, base, m);
		e >>= 1;
	}
	return result;
}

// Sign of A 2^ea - B 2^eb, exactly, for A, B < 2^62.
inline int compare_scaled(uint64_t A, int64_t ea, uint64_t B, int64_t eb) {
	using namespace takum_wide;
	const int64_t d = ea - eb;
	if (d >= 64)  return 1;                                   // A >= 1, so A 2^d > B
	if (d <= -64) return -1;
	const u128 a = (d >= 0) ? shift_left(make_u128(A), static_cast<unsigned>(d)) : make_u128(A);
	const u128 b = (d <  0) ? shift_left(make_u128(B), static_cast<unsigned>(-d)) : make_u128(B);
	return less(a, b) ? -1 : (less(b, a) ? 1 : 0);
}

// Exact fmod / IEEE remainder for the linear takum, whose values are dyadic:
// x = Sx 2^ex, y = Sy 2^ey.  With g = min(ex, ey) both are integers times 2^g, and
// x mod y = ((Sx 2^(ex-g)) mod Y) 2^g with Y = Sy 2^(ey-g).  When |x| >= |y|, Y fits
// 62 bits either way, and the huge factor 2^(ex-g) is reduced by modular
// exponentiation, so 2^65535 mod 3 costs ~16 squarings.  The result is exact before
// the single rounding into the takum.
template<typename T>
std::optional<T> fmod_exact(const T& x, const T& y, bool ieee_remainder) {
	if constexpr (is_takum_log<T>) {
		// not dyadic: only the cases that need no quotient
		if (x.isnar() || y.isnar() || y.iszero() || x.iszero()) return std::nullopt;
		if (!is_far(x) && !is_far(y)) return std::nullopt;
		const T ax = absolute(x), ay = absolute(y);
		if (!ieee_remainder && ax < ay) return x;
		if (ieee_remainder && (ax + ax) < ay) return x;
		T nar; nar.setnar(); return nar;
	}
	else {
		if (x.isnar() || y.isnar() || y.iszero() || x.iszero()) return std::nullopt;
		if (!is_far(x) && !is_far(y)) return std::nullopt;
		const auto dx = T::Codec::decode(x.magnitude_bits());
		const auto dy = T::Codec::decode(y.magnitude_bits());
		const uint64_t Sx = (1ull << dx.p) | dx.M_bits, Sy = (1ull << dy.p) | dy.M_bits;
		const int64_t ex = dx.c - static_cast<int64_t>(dx.p), ey = dy.c - static_cast<int64_t>(dy.p);
		const bool negative = x.sign();
		auto make = [](uint64_t R, int64_t e, bool sgn) -> T {
			T out;
			out.setzero();                                         // the default constructor leaves storage as is
			if (R == 0) return out;                                // takum has one zero
			return out.assign_wide(takum_wide::wide_value{ takum_wide::make_u128(R), e, sgn, false });
		};
		const T ax = absolute(x), ay = absolute(y);
		if (ax < ay) {
			if (!ieee_remainder) return x;
			// remainder: n = 0 unless 2|x| > |y| (a tie keeps the even n = 0)
			if (compare_scaled(Sx, ex + 1, Sy, ey) <= 0) return x;
			return x - (negative ? -ay : ay);
		}
		const int64_t g = std::min(ex, ey);
		const uint64_t Y = Sy << static_cast<unsigned>(ey - g);     // fits: Y <= |x| / 2^g < 2^62
		const uint64_t M = ieee_remainder ? 2 * Y : Y;              // mod 2Y also yields the quotient's parity
		uint64_t r = (ex > g) ? mulmod(Sx % M, pow2mod(static_cast<uint64_t>(ex - g), M), M) : (Sx % M);
		if (!ieee_remainder) return make(r, g, negative);
		const bool odd = (r >= Y);
		if (odd) r -= Y;
		if (2 * r > Y || (2 * r == Y && odd)) return make(Y - r, g, !negative);   // round the quotient up
		return make(r, g, negative);
	}
}

}}} // namespace sw::universal::takum_wide_range
