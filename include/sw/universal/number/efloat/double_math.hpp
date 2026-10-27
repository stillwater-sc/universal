#pragma once
// double_math.hpp: the double arithmetic efloat estimates with, usable in constant expressions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// efloat's math library sizes its work in double: the terms a series needs, the halvings of
// an argument, the bits a subtraction cancels, the precision lgamma's parts call for (#1819).
// These are the functions those estimates take, with no <cmath> call, so that the estimates are
// the same in a constant evaluation as at run time, on every platform's libm.
//
//   fabs, floor, ceil, trunc, round, nearbyint, ilogb, isfinite, scale2 (x 2^n)
//                    from the fields: exact, the values std:: gives
//   sqrt             correctly rounded: in a constant evaluation from the integer root of the
//                    significand, at run time by the instruction. IEEE-754 rounds a square root
//                    correctly, so the two are the same value; the instruction is ~30x faster, and
//                    exp, sin and expm1 size their halvings with it on every call.
//   log2, log, log10 sw::math::constexpr_math's artanh series, within a few units in the last
//                    place
//   sin_pi           sin(pi f) for |f| <= 1/2, a Taylor series, within a few units
//   lgamma           log |gamma x|, Stirling's series past 10 and the reflection below 1/2,
//                    within 2^-40 max(1, |lgamma x|): next to its zeros the error is absolute
//
// None of them is a math library: the estimates round their results to integers, with margins
// for the last place.
#include <bit>
#include <cmath>      // std::sqrt, at run time only: see sqrt below
#include <cstdint>
#include <type_traits>
#include <limits>
#include <math/constexpr_math/detail.hpp>
#include <math/constexpr_math/log2.hpp>

namespace sw { namespace universal { namespace efloat_detail { namespace fp {

	inline constexpr double pi    = 3.141592653589793;
	inline constexpr double ln2   = 0.6931471805599453;
	inline constexpr double log10_2 = 0.30102999566398120;

	constexpr bool isfinite(double x) noexcept {
		return ((std::bit_cast<uint64_t>(x) >> 52) & 0x7FFu) != 0x7FFu;
	}

	constexpr double fabs(double x) noexcept {
		return std::bit_cast<double>(std::bit_cast<uint64_t>(x) & ~(uint64_t(1) << 63));
	}

	// x rounded toward zero to an integer: the fraction bits below the units cleared
	constexpr double trunc(double x) noexcept {
		const uint64_t bits = std::bit_cast<uint64_t>(x);
		const int e = static_cast<int>((bits >> 52) & 0x7FFu) - 1023;
		if (e >= 52) return x;                                       // an integer, inf or nan
		if (e < 0) return std::bit_cast<double>(bits & (uint64_t(1) << 63));   // +-0
		return std::bit_cast<double>(bits & ~((uint64_t(1) << (52 - e)) - 1u));
	}

	constexpr double floor(double x) noexcept {
		const double t = trunc(x);
		return (t > x) ? t - 1.0 : t;
	}

	constexpr double ceil(double x) noexcept {
		const double t = trunc(x);
		return (t < x) ? t + 1.0 : t;
	}

	// to the nearest integer, a half away from zero, as std::round: the half added to |x| is
	// exact below 2^52, where |x| has a fraction
	constexpr double round(double x) noexcept {
		const double a = fabs(x);
		if (!(a < 4503599627370496.0)) return x;                     // 2^52: an integer, inf or nan
		double r = trunc(a + 0.5);
		if (r - a > 0.5) r -= 1.0;                                   // a + 0.5 rounded up past a half
		return (x < 0.0) ? -r : r;
	}

	// to the nearest integer, a half to even, as std::nearbyint in the default mode
	constexpr double nearbyint(double x) noexcept {
		const double a = fabs(x);
		if (!(a < 4503599627370496.0)) return x;
		double r = trunc(a);
		const double d = a - r;                                      // exact
		if (d > 0.5 || (d == 0.5 && trunc(r * 0.5) * 2.0 != r)) r += 1.0;
		return (x < 0.0) ? -r : r;
	}

	// the exponent of a finite x != 0, x = m 2^e with m in [1, 2), subnormals included
	constexpr int ilogb(double x) noexcept {
		const uint64_t bits = std::bit_cast<uint64_t>(x);
		const int raw = static_cast<int>((bits >> 52) & 0x7FFu);
		if (raw != 0) return raw - 1023;
		const uint64_t fraction = bits & ((uint64_t(1) << 52) - 1u);
		return -1074 + static_cast<int>(std::bit_width(fraction)) - 1;
	}

	// x 2^n, exact while the result is normal, as every caller's is
	constexpr double scale2(double x, int n) noexcept {
		while (n > 1000)  { x *= sw::math::constexpr_math::detail::pow2(1000);  n -= 1000; }
		while (n < -1000) { x *= sw::math::constexpr_math::detail::pow2(-1000); n += 1000; }
		return x * sw::math::constexpr_math::detail::pow2(n);
	}

	// sqrt(x), correctly rounded, without the instruction: x = M 2^(2k) with M an integer of 105 or
	// 106 bits, whose integer root r has 53, and the root rounds up where M - r^2 > r, which is M
	// past (r + 1/2)^2
	constexpr double sqrt_exact(double x) noexcept {
		if (x != x || x < 0.0) return std::numeric_limits<double>::quiet_NaN();
		if (x == 0.0 || !isfinite(x)) return x;
		const uint64_t bits = std::bit_cast<uint64_t>(x);
		const int raw = static_cast<int>((bits >> 52) & 0x7FFu);
		uint64_t m = bits & ((uint64_t(1) << 52) - 1u);
		int e = -1074;                                               // x = m 2^e
		if (raw != 0) {
			m |= uint64_t(1) << 52;
			e = raw - 1075;
		}
		const int w = static_cast<int>(std::bit_width(m));
		int s = 105 - w;                                             // M = m 2^s, 105 bits
		if (((e - s) & 1) != 0) ++s;                                 // or 106, for an even exponent
		// M as (hi, lo), s >= 51
		const uint64_t hi = (s >= 64) ? (m << (s - 64)) : (m >> (64 - s));
		const uint64_t lo = (s >= 64) ? 0u : (m << s);
		// a root within a few units, from double, then exact
		const double approx = sw::math::constexpr_math::detail::pow2(64) * static_cast<double>(hi) + static_cast<double>(lo);
		double y = sw::math::constexpr_math::detail::pow2(53);         // above the root, in [2^52, 2^53)
		for (int i = 0; i < 16; ++i) {                               // Newton from above falls to it
			const double next = 0.5 * (y + approx / y);
			if (next >= y) break;
			y = next;
		}
		uint64_t r = static_cast<uint64_t>(y);
		// r^2 as (h, l)
		auto square = [](uint64_t v, uint64_t& h, uint64_t& l) {
			const uint64_t a = v >> 32, b = v & 0xFFFF'FFFFu;
			const uint64_t aa = a * a, ab = a * b, bb = b * b;
			const uint64_t mid_lo = ab << 33;
			l = bb + mid_lo;
			h = aa + (ab >> 31) + ((l < bb) ? 1u : 0u);
		};
		auto above = [&](uint64_t v) {                               // v^2 > M
			uint64_t h = 0, l = 0;
			square(v, h, l);
			return h > hi || (h == hi && l > lo);
		};
		while (above(r)) --r;
		while (!above(r + 1)) ++r;
		uint64_t h = 0, l = 0;
		square(r, h, l);
		const uint64_t rem = lo - l;                                 // M - r^2 <= 2 r, in one word
		if (rem > r) ++r;
		return scale2(static_cast<double>(r), (e - s) / 2);
	}

	constexpr double sqrt(double x) noexcept {
		if (!std::is_constant_evaluated()) return std::sqrt(x);
		return sqrt_exact(x);
	}

	constexpr double log2(double x) noexcept { return sw::math::constexpr_math::log2(x); }
	constexpr double log(double x) noexcept { return sw::math::constexpr_math::log2(x) * ln2; }
	constexpr double log10(double x) noexcept { return sw::math::constexpr_math::log2(x) * log10_2; }

	// sin(pi f) for |f| <= 1/2: t = pi f, |t| <= pi/2, the Taylor series to t^25
	constexpr double sin_pi(double f) noexcept {
		const double t  = pi * f;
		const double t2 = t * t;
		double term = t, sum = t;
		for (int k = 1; k <= 12; ++k) {
			term *= -t2 / static_cast<double>((2 * k) * (2 * k + 1));
			sum += term;
		}
		return sum;
	}

	// log |gamma x| for a finite x; +inf at 0 and the negative integers
	constexpr double lgamma(double x) noexcept {
		if (x < 0.5) {
			// gamma(x) gamma(1 - x) = pi / sin(pi x), with sin(pi x) = +-sin(pi f), f = x - n
			const double f = x - nearbyint(x);
			if (f == 0.0) return std::numeric_limits<double>::infinity();   // a pole, as std::lgamma
			return log(pi / fabs(sin_pi(f))) - lgamma(1.0 - x);
		}
		// lgamma(x) = lgamma(x + n) - log(x (x + 1) ... (x + n - 1)), x + n >= 10
		double shift = 0.0;
		double z = x;
		while (z < 10.0) {
			shift += log(z);
			z += 1.0;
		}
		const double w = 1.0 / (z * z);
		const double series = (1.0 / 12.0 - w * (1.0 / 360.0 - w * (1.0 / 1260.0 - w * (1.0 / 1680.0)))) / z;
		return (z - 0.5) * log(z) - z + 0.9189385332046728 + series - shift;   // log(2 pi) / 2
	}

}}}}  // namespace sw::universal::efloat_detail::fp
