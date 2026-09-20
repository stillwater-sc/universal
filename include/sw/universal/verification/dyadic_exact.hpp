#pragma once
// dyadic_exact.hpp: an exact dyadic-rational reference oracle.
//
// Every IEEE-754 binary floating-point value is a dyadic rational: a value of
// the form  numerator * 2^scale  with an integer numerator and an integer
// scale. Dyadic rationals are closed under +, -, and * with NO rounding and
// require NO division or gcd -- only big-integer arithmetic and power-of-two
// shifts. That makes them an ideal independent oracle for expansion arithmetic
// (Shewchuk/Priest error-free transformations and expansions): the oracle
// shares no code path with the algorithms under test, so it can catch a result
// that is structurally well-formed but numerically wrong.
//
// Backed by einteger, an adaptive-precision integer.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/einteger/einteger.hpp>

#include <cmath>
#include <cstdint>
#include <string>
#include <string_view>
#include <stdexcept>
#include <limits>
#include <type_traits>

namespace sw { namespace universal {

// Exact dyadic rational  value == numerator * 2^scale.
// numerator carries the sign (einteger is signed); scale is a binary exponent.
struct dyadic {
	using bigint = einteger<std::uint32_t>;

	bigint numerator;   // signed
	int    scale = 0;   // value = numerator * 2^scale

	dyadic() : numerator(0), scale(0) {}
	dyadic(const bigint& n, int s) : numerator(n), scale(s) {}

	// Exact construction from a double. frexp gives d = m * 2^e with m in
	// [0.5, 1); m * 2^53 is then an integer M with |M| < 2^53 (exact in a
	// double), so d = M * 2^(e - 53). Handles normals, subnormals, and zero;
	// inf/nan are out of scope (callers must not pass them).
	static dyadic from_double(double d) {
		if (d == 0.0) return dyadic();              // +0 and -0 both map to exact 0
		int e = 0;
		double m = std::frexp(d, &e);               // d == m * 2^e
		long long M = static_cast<long long>(std::ldexp(m, 53));  // exact integer
		return dyadic(bigint(M), e - 53);
	}

	// Exact construction from any binary floating-point type -- float, double, x87
	// extended, binary128. from_double would round a long double to 53 bits. The
	// significand is taken 32 bits at a time: each step scales by 2^32 and takes the
	// integer part of a value whose lower bits are already clear, so every step is exact
	// in T itself (#1355). inf/nan are out of scope, as for from_double.
	template<typename T>
	static dyadic from_fp(T v) {
		static_assert(std::is_floating_point_v<T> && std::numeric_limits<T>::radix == 2,
		              "dyadic::from_fp needs a binary floating-point type");
		if (v == T(0)) return dyadic();
		const bool negative = v < T(0);
		int e = 0;
		T m = std::frexp(negative ? -v : v, &e);   // v == m * 2^e, m in [0.5, 1)
		bigint M(0);
		int s = e;
		while (m != T(0)) {
			m = std::ldexp(m, 32);
			const T chunk = std::floor(m);
			m -= chunk;
			M <<= 32;
			M += bigint(static_cast<long long>(chunk));
			s -= 32;
		}
		if (negative) M = -M;
		return dyadic(M, s);
	}
	bool iszero() const { return numerator.iszero(); }
};

// Bring two dyadics to a common (minimum) scale by left-shifting the numerators.
// Left shift by k multiplies by 2^k exactly; the gap can be large (up to the
// full double exponent span) but einteger grows as needed.
inline void dyadic_align(const dyadic& a, const dyadic& b,
                         dyadic::bigint& na, dyadic::bigint& nb, int& common) {
	common = (a.scale < b.scale) ? a.scale : b.scale;
	na = a.numerator; na <<= (a.scale - common);
	nb = b.numerator; nb <<= (b.scale - common);
}

inline dyadic operator-(const dyadic& a) { return dyadic(-a.numerator, a.scale); }

inline dyadic operator+(const dyadic& a, const dyadic& b) {
	dyadic::bigint na, nb; int common;
	dyadic_align(a, b, na, nb, common);
	return dyadic(na + nb, common);
}

inline dyadic operator-(const dyadic& a, const dyadic& b) { return a + (-b); }

inline dyadic operator*(const dyadic& a, const dyadic& b) {
	// (na * 2^sa)(nb * 2^sb) == (na*nb) * 2^(sa+sb)
	return dyadic(a.numerator * b.numerator, a.scale + b.scale);
}

// Exact equality: align to a common scale and compare numerators.
inline bool operator==(const dyadic& a, const dyadic& b) {
	dyadic::bigint na, nb; int common;
	dyadic_align(a, b, na, nb, common);
	return na == nb;
}
inline bool operator!=(const dyadic& a, const dyadic& b) { return !(a == b); }


// ============================================================================
// DECIMAL AGREEMENT
// ============================================================================
// How many leading significant decimal digits a value agrees with a reference to.
// These live here, next to the dyadic they measure, so that every number system with
// an exact dyadic conversion can use them: elreal's ZBCL (elreal_reference_digits.hpp)
// and ereal's expansions (ereal_reference_digits.hpp) both do (#1566). The comparison
// is exact -- cross-multiplied big integers, no rounding and no floating-point step --
// so it shares no code path with the arithmetic under test.
//
// `cap` bounds the work and states what the reference can certify: a reference
// correctly rounded to D digits cannot certify its own last digit, so the cap belongs
// below D (the 340-digit constants in reference_constants.hpp are read at 320, and the
// long references at their own cap).

// Number of leading significant decimal digits to which the exact value V agrees
// with the positive decimal reference string `ref` (e.g. "3.14159..."). Returns
// `cap` if they agree to at least `cap` digits (or are exactly equal).
//
//   relative error  = |V - ref| / |ref|
//   with  V == Vn/Vd  and  ref == N / 10^frac, the error numerator (cross-
//   multiplied) is  |Vn*10^frac - N*Vd|  over denominator  Vd*N, so the value
//   agrees to >= d digits iff  |Vn*10^frac - N*Vd| * 10^d  <=  Vd*N.
inline int agreed_decimal_digits(const dyadic& V, std::string_view ref, int cap = 320) {
	using bigint = dyadic::bigint;

	// Parse the positive reference into numerator N and fractional-digit count.
	// Reject malformed input rather than silently skipping it: for a verification
	// oracle an unsupported reference (a typo like "3e10", a sign, whitespace)
	// must fail fast, not be reinterpreted as a different number.
	std::string digits;
	int frac = -1;                        // -1 until the '.' is seen
	for (char c : ref) {
		if (c == '.') {
			if (frac >= 0) throw std::invalid_argument("decimal reference: multiple '.'");
			frac = 0;
			continue;
		}
		if (c >= '0' && c <= '9') {
			digits.push_back(c);
			if (frac >= 0) ++frac;
			continue;
		}
		throw std::invalid_argument("decimal reference: unsupported character");
	}
	if (digits.empty()) throw std::invalid_argument("decimal reference: no digits");
	if (frac < 0) frac = 0;               // integer-only reference

	// Strip leading zeros BEFORE handing the digit string to einteger::parse.
	// That parser follows C literal conventions, so a leading '0' followed by
	// octal digits is read as OCTAL: the reference "0.75" yields the digit
	// string "075", which parses as 61 rather than 75, and the oracle then
	// reports 0 agreeing digits for a value that is exactly equal. Stripping
	// the zeros does not change the value, because `frac` -- not the digit
	// count -- carries the scale (ref == N / 10^frac). ("0.5" survived only by
	// coincidence: octal 5 == decimal 5, as it does for every single digit.)
	std::size_t firstSignificant = digits.find_first_not_of('0');
	digits = (firstSignificant == std::string::npos) ? "0" : digits.substr(firstSignificant);

	bigint N; N.assign(digits);           // ref == N / 10^frac
	// A relative-agreement measure needs a non-zero reference: with N == 0 the
	// denominator below is 0 and every comparison against it is meaningless.
	// Matches the dyadic/dyadic overload, which rejects a zero reference too.
	if (N.iszero()) throw std::invalid_argument("decimal reference: zero reference value");

	// V == Vn / Vd  (Vd a power of two, or 1).
	bigint Vn = V.numerator, Vd(1);
	if (V.scale >= 0) Vn <<= V.scale; else Vd <<= (-V.scale);

	bigint tenFrac; tenFrac.assign("1" + std::string(static_cast<std::size_t>(frac), '0'));
	bigint diff = Vn * tenFrac - N * Vd;  // error numerator (signed)
	if (diff.sign()) diff.setsign(false); // |error numerator|
	bigint denom = Vd * N;                // positive

	if (diff.iszero()) return cap;        // exact to the cap

	// largest d with diff * 10^d <= denom
	bigint cur = diff, ten; ten.assign("10");
	for (int d = 1; d <= cap; ++d) {
		cur = cur * ten;
		if (denom < cur) return d - 1;    // diff*10^d > denom: agreement lost at digit d
	}
	return cap;
}
// Number of leading significant decimal digits to which exact value A agrees with
// exact value B (relative to B). For identity residual checks where neither side
// has a closed-form decimal reference -- e.g. exp(a+b) == exp(a)*exp(b) or the
// sin(a+b) expansion (#1049): build both sides as exact dyadics and compare them
// directly, with no shared code path through elreal arithmetic.
//
//   relative error = |A - B| / |B|
//   with  A == An/Ad,  B == Bn/Bd  (denominators powers of two or 1), the cross-
//   multiplied error numerator is  |An*Bd - Bn*Ad|  over denominator  |Ad*Bn|, so
//   A agrees with B to >= d digits iff  |An*Bd - Bn*Ad| * 10^d  <=  |Ad*Bn|.
inline int agreed_decimal_digits(const dyadic& A, const dyadic& B, int cap = 320) {
	using bigint = dyadic::bigint;

	bigint An = A.numerator, Ad(1);
	if (A.scale >= 0) An <<= A.scale; else Ad <<= (-A.scale);
	bigint Bn = B.numerator, Bd(1);
	if (B.scale >= 0) Bn <<= B.scale; else Bd <<= (-B.scale);

	bigint diff = An * Bd - Bn * Ad;      // error numerator (signed)
	if (diff.sign()) diff.setsign(false); // |error numerator|
	bigint denom = Ad * Bn;               // |B| numerator (Ad > 0)
	if (denom.sign()) denom.setsign(false);
	if (denom.iszero()) throw std::invalid_argument("agreed_decimal_digits: zero reference value");

	if (diff.iszero()) return cap;        // exact to the cap

	bigint cur = diff, ten; ten.assign("10");
	for (int d = 1; d <= cap; ++d) {
		cur = cur * ten;
		if (denom < cur) return d - 1;    // diff*10^d > denom: agreement lost at digit d
	}
	return cap;
}

}}  // namespace sw::universal
