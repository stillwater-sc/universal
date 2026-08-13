// pow.hpp: power functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <cstdint>
#include <universal/number/takum/math/takum_log_domain.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> pow(const takum<nbits, rbits, bt>& x, const takum<nbits, rbits, bt>& y) {
	return takum<nbits, rbits, bt>(std::pow(double(x), double(y)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> pow(const takum<nbits, rbits, bt>& x, int y) {
	return takum<nbits, rbits, bt>(std::pow(double(x), double(y)));
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> pow(const takum<nbits, rbits, bt>& x, double y) {
	return takum<nbits, rbits, bt>(std::pow(double(x), y));
}

// ---------------------------------------------------------------------------
// takum_log
// ---------------------------------------------------------------------------

// An integer power scales the logarithmic value: |x^n| == sqrt(e)^(n*l).  The
// scaling is exact whenever the products fit, so this rounds once rather than
// three times, and unlike std::pow it never routes the intermediate through a
// double.  Falls back to the real-valued path when n*l would overflow int64.
template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> pow(const takum_log<nbits, rbits, bt>& x, int n) {
	using TL = takum_log<nbits, rbits, bt>;
	TL result;
	if (x.isnar()) { result.setnar(); return result; }
	if (n == 0) return TL(1.0);                       // x^0 == 1, including 0^0 by convention
	if (x.iszero()) {
		if (n > 0) return x;                          // 0^n == 0
		result.setnar(); return result;               // 0^-n has no value
	}
	// A negative base is real only for an integer exponent, and the sign of the
	// result is the parity of that exponent.
	const bool negative = x.sign() && ((n & 1) != 0);

	auto l = to_log_value(x);
	// l scaled by n, exactly: n*(c + N/2^q) == n*c + n*N/2^q, then renormalize the
	// fraction back into [0,1) by folding its integer part into the characteristic.
	const int64_t  a  = (n < 0) ? -static_cast<int64_t>(n) : static_cast<int64_t>(n);
	const uint64_t ua = static_cast<uint64_t>(a);
	// guard the two products before forming them
	const uint64_t den = (l.q > 0) ? (1ull << l.q) : 1ull;
	if (ua != 0 && (l.N > (~0ull) / ua)) {
		return TL(std::pow(double(x), double(n)));    // fraction product would wrap
	}
	const int64_t  cmag = (l.c < 0) ? -l.c : l.c;
	if (cmag != 0 && a > (INT64_MAX - 1) / cmag) {
		return TL(std::pow(double(x), double(n)));    // characteristic product would wrap
	}
	uint64_t Nn   = l.N * ua;
	int64_t  cn   = l.c * a;
	if (l.q > 0) { cn += static_cast<int64_t>(Nn / den); Nn = Nn % den; }
	takum_log_value scaled{ cn, Nn, l.q };
	if (n < 0) scaled = negate(scaled);
	return from_log_value<TL>(scaled, negative);
}

// General real exponent: no shortcut exists, so this is the double round trip.
template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> pow(const takum_log<nbits, rbits, bt>& x,
                                       const takum_log<nbits, rbits, bt>& y) {
	return takum_log<nbits, rbits, bt>(std::pow(double(x), double(y)));
}

template<unsigned nbits, unsigned rbits, typename bt>
inline takum_log<nbits, rbits, bt> pow(const takum_log<nbits, rbits, bt>& x, double y) {
	return takum_log<nbits, rbits, bt>(std::pow(double(x), y));
}

// Exact integer power via repeated squaring; no double round-trip.
#ifndef UNIVERSAL_MATH_INTEGER_POWER_DEFINED
#define UNIVERSAL_MATH_INTEGER_POWER_DEFINED
template<typename Scalar>
Scalar integer_power(Scalar base, int exponent) {
	if (exponent < 0) {
		base = Scalar(1) / base;
		exponent = -exponent;
	}
	if (exponent == 0) return Scalar(1);
	Scalar power = Scalar(1);
	while (exponent > 1) {
		if (exponent & 0x1) {
			power = base * power;
			base *= base;
			exponent = (exponent - 1) / 2;
		}
		else {
			base *= base;
			exponent /= 2;
		}
	}
	return base * power;
}
#endif // UNIVERSAL_MATH_INTEGER_POWER_DEFINED

}} // namespace sw::universal
