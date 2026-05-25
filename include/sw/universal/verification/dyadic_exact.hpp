#pragma once
// dyadic_exact.hpp: an exact dyadic-rational reference oracle (epic #987).
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
// Backed by einteger (adaptive-precision integer, verified sound). It is
// deliberately NOT backed by erational, whose double conversion is broken
// (see issue #986).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <cstdint>
#include <universal/number/einteger/einteger.hpp>

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

}}  // namespace sw::universal
