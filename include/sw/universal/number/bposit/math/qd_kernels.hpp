#pragma once
// qd_kernels.hpp: quad-double evaluations of the elementary functions at an exact b-posit argument
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Each kernel returns the function's value with a relative error far below 2^-170, the
// budget of correctly_rounded.hpp.  quad-double's own functions meet it where their result
// does not cancel; the kernels below take over where it would:
//
//   log near 1             log1p of the exact x - 1, as 2 atanh(u / (2 + u))
//   1 - x, 1 + x, x - 1    formed exactly from the b-posit's fields (one_plus)
//   sin, cos, tan          Payne-Hanek reduction (reduce_pio2.hpp), then qd on |r| <= pi/4
//   asin, acos             atan2 of exact differences, never 1 - x^2
//   sinh, tanh, near 0     from expm1, whose Taylor series is relative-accurate
//   asinh, acosh, atanh    log1p forms, with the asymptotic correction for huge arguments
//   cbrt                   Newton in qd from a double seed
//   hypot                  scaled by a power of two, so the squares stay in range
//
// Callers have screened out NaR, the domain errors and the results that saturate.
#include <algorithm>
#include <cmath>
#include <universal/number/bposit/math/correctly_rounded.hpp>
#include <universal/number/bposit/math/reduce_pio2.hpp>

namespace sw { namespace universal { namespace bposit_math {

// log(1 + u) for |u| <= 1/4: 2 atanh(s), s = u / (2 + u), |s| <= 1/7
inline qd log1p_small(const qd& u) {
	const qd s = u / (qd(2.0) + u);
	const qd s2 = sw::universal::sqr(s);
	qd term = s, sum = s;
	for (int k = 3; k < 200; k += 2) {
		term *= s2;
		const qd t = term / qd(static_cast<double>(k));
		sum += t;
		if (std::fabs(t[0]) < std::ldexp(std::fabs(sum[0]), -220)) break;
	}
	return sw::universal::mul_pwr2(sum, 2.0);
}

// log(1 + u) for u > -1, where 1 + u involves no cancellation beyond |u| <= 1/4
inline qd log1p_qd(const qd& u) {
	if (std::fabs(u[0]) <= 0.25) return log1p_small(u);
	return sw::universal::log(qd(1.0) + u);
}

// log(x) for x > 0
template<typename B>
qd log_kernel(const B& x) {
	const double v = double(x);
	if (v > 0.75 && v < 1.25) return log1p_small(-one_plus(x, -1));    // x - 1, exact
	return sw::universal::log(to_qd(x));
}

// log(1 + x) for x > -1
template<typename B>
qd log1p_kernel(const B& x) {
	const double v = double(x);
	if (std::fabs(v) <= 0.25) return log1p_small(to_qd(x));
	return sw::universal::log(one_plus(x, 1));                        // exact 1 + x
}

// e^t - 1, relative-accurate near 0
inline qd expm1_qd(const qd& t) {
	if (std::fabs(t[0]) < 0.5) return sw::universal::expm1(t);       // Taylor series
	return sw::universal::exp(t) - qd(1.0);
}

// sin and cos of x, from the exact reduction
template<typename B>
void sincos_kernel(const B& x, qd& s, qd& c) {
	auto [q, r] = reduce_pio2(x);
	qd sr, cr;
	sw::universal::sincos(r, sr, cr);
	switch (q & 3) {
	case 0: s = sr;  c = cr;  break;
	case 1: s = cr;  c = -sr; break;
	case 2: s = -sr; c = -cr; break;
	default: s = -cr; c = sr; break;
	}
}

template<typename B>
qd sin_kernel(const B& x) { qd s, c; sincos_kernel(x, s, c); return s; }

template<typename B>
qd cos_kernel(const B& x) { qd s, c; sincos_kernel(x, s, c); return c; }

template<typename B>
qd tan_kernel(const B& x) { qd s, c; sincos_kernel(x, s, c); return s / c; }

// asin(x), |x| < 1: atan2(x, sqrt((1 - x)(1 + x)))
template<typename B>
qd asin_kernel(const B& x) {
	return sw::universal::atan2(to_qd(x), sw::universal::sqrt(one_plus(x, -1) * one_plus(x, 1)));
}

// acos(x), |x| < 1: 2 atan2(sqrt(1 - x), sqrt(1 + x))
template<typename B>
qd acos_kernel(const B& x) {
	const qd a = sw::universal::atan2(sw::universal::sqrt(one_plus(x, -1)), sw::universal::sqrt(one_plus(x, 1)));
	return sw::universal::mul_pwr2(a, 2.0);
}

// sinh(x), |x| inside the range screen
inline qd sinh_qd(const qd& x) {
	if (std::fabs(x[0]) < 0.5) {
		const qd e = sw::universal::expm1(x);
		return sw::universal::mul_pwr2(e + e / (e + qd(1.0)), 0.5);
	}
	return sw::universal::mul_pwr2(sw::universal::exp(x) - sw::universal::exp(-x), 0.5);
}

inline qd cosh_qd(const qd& x) {
	return sw::universal::mul_pwr2(sw::universal::exp(x) + sw::universal::exp(-x), 0.5);
}

// tanh(x) = expm1(2x) / (expm1(2x) + 2), and 1 - 2 e^(-2|x|) once that is all that is left
inline qd tanh_qd(const qd& x) {
	const bool negative = x[0] < 0.0;
	const qd a = negative ? -x : x;
	qd t;
	if (a[0] > 40.0) {
		t = qd(1.0) - sw::universal::mul_pwr2(sw::universal::exp(sw::universal::mul_pwr2(-a, 2.0)), 2.0);
	}
	else {
		const qd e = expm1_qd(sw::universal::mul_pwr2(a, 2.0));
		t = e / (e + qd(2.0));
	}
	return negative ? -t : t;
}

// asinh(x) = sign(x) log1p(|x| + x^2 / (1 + sqrt(1 + x^2))), log(2|x|) + 1/(4x^2) when huge
inline qd asinh_qd(const qd& x) {
	const bool negative = x[0] < 0.0;
	const qd a = negative ? -x : x;
	qd r;
	if (a[0] > 0x1p60) {
		r = sw::universal::log(sw::universal::mul_pwr2(a, 2.0)) + qd(0.25) / sw::universal::sqr(a);
	}
	else {
		const qd a2 = sw::universal::sqr(a);
		r = log1p_qd(a + a2 / (qd(1.0) + sw::universal::sqrt(qd(1.0) + a2)));
	}
	return negative ? -r : r;
}

// acosh(x), x > 1: log1p(t + sqrt(t (t + 2))) with t = x - 1 exact, log(2x) - 1/(4x^2) when huge
template<typename B>
qd acosh_kernel(const B& x) {
	const qd xq = to_qd(x);
	if (xq[0] > 0x1p60) return sw::universal::log(sw::universal::mul_pwr2(xq, 2.0)) - qd(0.25) / sw::universal::sqr(xq);
	const qd t = -one_plus(x, -1);
	return log1p_qd(t + sw::universal::sqrt(t * (t + qd(2.0))));
}

// atanh(x), |x| < 1: sign(x) log1p(2|x| / (1 - |x|)) / 2, 1 - |x| exact
template<typename B>
qd atanh_kernel(const B& x) {
	const B a = abs(x);
	const qd aq = to_qd(a);
	const qd r = sw::universal::mul_pwr2(log1p_qd(sw::universal::mul_pwr2(aq, 2.0) / one_plus(a, -1)), 0.5);
	return x.isneg() ? -r : r;
}

// the cube root, by Newton from a double seed: 53 -> 106 -> 212 bits, and one more for margin
inline qd cbrt_qd(const qd& a) {
	const bool negative = a[0] < 0.0;
	const qd m = negative ? -a : a;
	qd c(std::cbrt(m[0]));
	for (int i = 0; i < 3; ++i) {
		const qd c2 = sw::universal::sqr(c);
		c = c - (c2 * c - m) / (qd(3.0) * c2);
	}
	return negative ? -c : c;
}

// sqrt(x^2 + y^2 [+ z^2]), scaled so that the squares neither overflow nor underflow
inline qd hypot_qd(const qd& x, const qd& y, const qd& z = qd(0.0)) {
	int k = -100000;
	for (const qd* v : { &x, &y, &z }) if ((*v)[0] != 0.0) k = std::max(k, std::ilogb((*v)[0]));
	if (k == -100000) return qd(0.0);
	const qd xs = sw::universal::ldexp(x, -k), ys = sw::universal::ldexp(y, -k), zs = sw::universal::ldexp(z, -k);
	return sw::universal::ldexp(sw::universal::sqrt(sw::universal::sqr(xs) + sw::universal::sqr(ys) + sw::universal::sqr(zs)), k);
}

// atan2(y, x) for x > 0 and |y / x| < 2^-85.  atan(q) = q (1 - q^2/3 + ...) lies within
// 2^-170 of q, inside the engine's ambiguity band, so the general rule (ambiguous means on a
// midpoint) would be wrong whenever q itself is exactly a midpoint: atan(q) is then just
// below it in magnitude, and rounds toward zero.  Otherwise q is at least 2^-120 (relative)
// from every midpoint, being a ratio of two b-posits, and atan(q) rounds as q does.
template<typename B>
B atan2_tiny(const B& y, const B& x) {
	const qd q = to_qd(y) / to_qd(x);
	const B c = round_to<B>(q);
	// the midpoints on both sides of c, as odd integer m times 2^e
	struct dyadic { std::uint64_t m; int e; };
	auto value = [](const B& v) {
		const auto d = v.decode_fields();
		return dyadic{ (std::uint64_t(1) << d.nf) | d.frac, d.scale - static_cast<int>(d.nf) };
	};
	auto odd = [](dyadic v) { while (v.m != 0u && (v.m & 1u) == 0u) { v.m >>= 1; ++v.e; } return v; };
	const dyadic dy = odd(value(abs(y))), dx = odd(value(x));
	const B ac = abs(c);
	for (int side = 0; side < 2; ++side) {
		B nb(ac);
		if (side == 0) { if (ac == B(SpecificValue::maxpos)) continue; ++nb; }
		else           { if (ac == B(SpecificValue::minpos)) continue; --nb; }
		const dyadic a = value(ac), b = value(nb);
		const int e = std::min(a.e, b.e) - 1;
		const std::uint64_t sum = (a.m << (a.e - e)) + (b.m << (b.e - e));   // 2 mid / 2^e, below 2^63
		const dyadic mid = odd(dyadic{ sum, e - 1 });
		// |y| == mid * x exactly?
		std::uint64_t hi, lo;
		mul64(mid.m, dx.m, hi, lo);
		if (hi == 0u && lo == dy.m && mid.e + dx.e == dy.e) {
			const B toward_zero = (nb < ac) ? nb : ac;
			return y.isneg() ? -toward_zero : toward_zero;
		}
	}
	return c;
}

// -1 when y is not an integer, 0 when it is even, 1 when it is odd
template<typename B>
int integer_parity(const B& y) {
	if (y.iszero()) return 0;
	const auto d = y.decode_fields();
	const std::uint64_t sig = (std::uint64_t(1) << d.nf) | d.frac;
	const int e = d.scale - static_cast<int>(d.nf);              // y = sig 2^e
	if (e > 0) return 0;
	if (e == 0) return static_cast<int>(sig & 1u);
	if (-e > 62) return -1;                                      // |y| < 1
	const std::uint64_t mask = (std::uint64_t(1) << (-e)) - 1u;
	if ((sig & mask) != 0u) return -1;
	return static_cast<int>((sig >> (-e)) & 1u);
}

inline int integer_parity(double y) {
	if (std::isinf(y)) return 0;
	if (std::floor(y) != y) return -1;
	return (std::fmod(y, 2.0) != 0.0) ? 1 : 0;
}

}}} // namespace sw::universal::bposit_math
