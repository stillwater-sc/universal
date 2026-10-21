// trigonometry.hpp: trigonometric and inverse trigonometric functions for efloat
//                   (sin, cos, tan, asin, acos, atan, atan2)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <universal/number/efloat/math/constants/efloat_constants.hpp>
#include <universal/number/efloat/math/exponent.hpp>   // result_precision, exp_log_guard_bits
#include <universal/number/efloat/math/truncate.hpp>   // round, floor
#include <universal/number/efloat/math/sqrt.hpp>       // sqrt

namespace sw { namespace universal {

// ----------------------------------------------------------------------------
// Derived constants
//
// efloat_pi<nlimbs>() comes from efloat_constants.hpp (a ~1000-digit literal,
// converted exactly to the type's precision). pi/2 and pi/4 are derived by
// exact multiplication by 0.5 / 0.25 (both exactly representable).
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
inline efloat<nlimbs> efloat_pi_2() {
	return efloat_pi<nlimbs>() * efloat<nlimbs>(0.5);
}
template<unsigned nlimbs>
inline efloat<nlimbs> efloat_pi_4() {
	return efloat_pi<nlimbs>() * efloat<nlimbs>(0.25);
}

namespace efloat_detail {

	// atan(1/m) = sum_k (-1)^k / ((2k + 1) m^(2k + 1)) for an integer m >= 2, at `bits` bits
	template<unsigned nlimbs>
	inline efloat<nlimbs> atan_reciprocal(unsigned m, unsigned bits) {
		efloat<nlimbs> one(1.0);
		one.set_precision(bits);
		const efloat<nlimbs> m2(static_cast<double>(m) * static_cast<double>(m));
		efloat<nlimbs> power = one / efloat<nlimbs>(static_cast<double>(m));   // 1 / m^(2k + 1)
		efloat<nlimbs> sum = power;
		for (unsigned k = 1; ; ++k) {
			power = power / m2;
			const efloat<nlimbs> term = power / efloat<nlimbs>(static_cast<double>(2 * k + 1));
			if (term.iszero() || term.scale() < sum.scale() - static_cast<int64_t>(bits) - 2) break;
			if (k % 2) sum -= term; else sum += term;
		}
		return sum;
	}

	// atan(1/2) at `bits` bits, computed rather than parsed (the 64-digit literal it replaces
	// was right to ~210 bits, #1712): atan(1/2) = atan(1/3) + atan(1/7), since
	// (1/3 + 1/7) / (1 - 1/21) = 1/2.  Cached per thread like ln 2.
	template<unsigned nlimbs>
	inline efloat<nlimbs> atan_half(unsigned bits) {
		thread_local efloat<nlimbs> cached;
		thread_local unsigned cached_bits = 0;
		if (cached_bits < bits) {
			cached = atan_reciprocal<nlimbs>(3, bits + 32) + atan_reciprocal<nlimbs>(7, bits + 32);
			cached_bits = bits;
		}
		efloat<nlimbs> value(cached);
		value.set_precision(bits);
		return value;
	}

	// pi at `bits` bits, computed rather than taken from efloat_pi<nlimbs>(), which carries the
	// type's own width (nlimbs * 32 bits, at most ~3300): a value widened by set_precision()
	// reduced its argument with a pi of that narrower width (#1712).  Machin's formula,
	// pi = 16 atan(1/5) - 4 atan(1/239), summed with 32 guard bits and cached like ln 2.
	template<unsigned nlimbs>
	inline efloat<nlimbs> pi(unsigned bits) {
		thread_local efloat<nlimbs> cached;
		thread_local unsigned cached_bits = 0;
		if (cached_bits < bits) {
			cached = atan_reciprocal<nlimbs>(5, bits + 32) * efloat<nlimbs>(16.0) - atan_reciprocal<nlimbs>(239, bits + 32) * efloat<nlimbs>(4.0);
			cached_bits = bits;
		}
		efloat<nlimbs> value(cached);
		value.set_precision(bits);
		return value;
	}

	// pi * 2^e at `bits` bits: pi/2 and pi/4 are pi with its exponent lowered
	template<unsigned nlimbs>
	inline efloat<nlimbs> pi_scaled(unsigned bits, int e) {
		efloat<nlimbs> value = pi<nlimbs>(bits);
		value.setexponent(value.scale() + e);
		return value;
	}

	// x = k pi/2 + r with |r| <= pi/4, r accurate to `bits` bits relative to itself, and
	// quadrant = k mod 4 (#1712).  x - k pi/2 cancels the scale(x) - scale(r) leading bits, and
	// x can lie as close to a multiple of pi/2 as its own precision allows, so the reduction is
	// retried with x and pi/2 widened by the cancellation until r keeps `bits` + guard correct bits,
	// as many as the rest of the evaluation carries, so that Ziv's test can rely on them (#1763).
	// A P-bit x cannot lie much closer to k pi/2 than 2^-2(P + scale(x)), so the retries stop at
	// three times that width.
	template<unsigned nlimbs>
	inline efloat<nlimbs> reduce_half_pi(const efloat<nlimbs>& x, unsigned bits, unsigned& quadrant) {
		const unsigned magnitude = static_cast<unsigned>(std::max<int64_t>(0, x.scale()));
		const unsigned limit     = 3 * (bits + magnitude) + 2 * exp_log_guard_bits;
		unsigned working         = bits + exp_log_guard_bits + magnitude;
		// |x| clearly below pi/4 (0.78539816...): k = 0 and r = x at the working precision, without
		// the division by pi/2 that would only confirm it (#1761)
		if (x.scale() < 0 && std::fabs(double(x)) < 0.7853) {
			efloat<nlimbs> r(x);
			r.set_precision(working);
			quadrant = 0;
			return r;
		}
		while (true) {
			efloat<nlimbs> xw(x);
			xw.set_precision(working);
			const efloat<nlimbs> half_pi = pi_scaled<nlimbs>(working, -1);
			const efloat<nlimbs> k = round(xw / half_pi);
			const efloat<nlimbs> r = xw - k * half_pi;
			const int64_t lost = r.iszero() ? static_cast<int64_t>(working) : std::max<int64_t>(0, x.scale() - r.scale());
			const unsigned needed = bits + exp_log_guard_bits + static_cast<unsigned>(lost);
			if (k.iszero() || working >= needed || working >= limit) {
				const efloat<nlimbs> four(4.0);
				const efloat<nlimbs> k_mod_4 = k - floor(k / four) * four;   // k is an integer: exact
				quadrant = static_cast<unsigned>(double(k_mod_4)) & 3u;
				return r;
			}
			working = std::min(limit, needed + exp_log_guard_bits);
		}
	}

	// The versine v = 1 - cos(r) for |r| <= pi/4, at r's precision (#1756).  sin and cos both
	// follow from it: cos r = 1 - v, |sin r| = sqrt(v (2 - v)), neither of which cancels, since
	// v <= 1 - cos(pi/4) < 0.3 and v keeps its relative precision as r -> 0.
	//
	// r is halved h times first, and the series for v(r / 2^h) doubled back h times by
	// v(2y) = 2 (2 v(y) - v(y)^2): each doubling costs one squaring, and it keeps the relative
	// error of v, so the working precision carries h guard bits.
	//
	// The series v(y) = y^2/2! - y^4/4! + ... is summed by rectangular splitting (#1761): the
	// terms are dealt round-robin into u partial sums S_i, each term without its factor y^2i, so
	// that only every u-th term needs a full multiplication (by y^2u) and the others a division
	// by a small integer; v = S_0 + S_1 y^2 + ... + S_{u-1} y^2(u-1) at the end.  The running term
	// and y^2u fall in precision with the terms, as in exp, with exact integer divisors (#1757).
	template<unsigned nlimbs>
	inline efloat<nlimbs> versine_with(const efloat<nlimbs>& r, unsigned halvings, unsigned u) {
		const unsigned precision = r.get_precision();
		const int64_t  h_signed  = static_cast<int64_t>(halvings) + r.scale();   // halvings that bring y below 2^-halvings
		const unsigned h         = (h_signed > 0) ? static_cast<unsigned>(h_signed) : 0u;
		const unsigned working   = precision + h;
		if (u < 1u) u = 1u;

		efloat<nlimbs> y(r);
		y.setsign(false);
		y.set_precision(working);
		y.setexponent(y.scale() - static_cast<int64_t>(h));

		// powers[i] = y^2i for i = 1..u
		std::vector<efloat<nlimbs>> powers(u + 1);
		powers[1] = y * y;
		for (unsigned i = 2; i <= u; ++i) powers[i] = powers[i - 1] * powers[1];
		std::vector<efloat<nlimbs>> sums(u);
		for (auto& sum : sums) { sum.setzero(); sum.set_precision(working); }

		// the term t_n = (-1)^(n+1) y^2n / (2n)! for n = 1 + i + u j sits in S_i as a = |t_n| / y^2i
		efloat<nlimbs> a(powers[1]);
		efloat<nlimbs> step = powers[u];   // y^2u, tapered with a
		unsigned taper = working;
		int64_t  top   = 0;                // scale of S_0 once its first term is in
		bool summed = false;
		for (uint64_t n = 1; !summed; ) {
			for (unsigned i = 0; i < u; ++i, ++n) {
				a /= (2 * n - 1) * (2 * n);
				if (n == 1) top = a.scale();
				if (a.iszero() || a.scale() < top - static_cast<int64_t>(working)) { summed = true; break; }
				if (n & 1u) sums[i] += a; else sums[i] -= a;
			}
			if (summed) break;
			const int64_t below = top - a.scale();   // bits the term sits below the sum
			const unsigned need = (below + 64 < static_cast<int64_t>(working)) ? static_cast<unsigned>(static_cast<int64_t>(working) - below) : 64u;
			if (need + 32u <= taper) {
				taper = need;
				a.set_precision(taper);
				step = powers[u];
				step.set_precision(taper);
			}
			a *= step;
		}
		efloat<nlimbs> v = sums[0];
		for (unsigned i = 1; i < u; ++i) v += sums[i] * powers[i];
		for (unsigned i = 0; i < h; ++i) {
			const efloat<nlimbs> square = v * v;
			v.setexponent(v.scale() + 1);
			v -= square;
			v.setexponent(v.scale() + 1);
		}
		return v;
	}

	// h ~ 0.25 sqrt(precision) + 6 halvings and u ~ 0.21 precision^0.35 partial sums, at least 2:
	// (h, u) = (11, 2) at 512 bits, (17, 3) at 2048 and (28, 5) at 8192, measured near the optimum,
	// which is flat (#1761)
	template<unsigned nlimbs>
	inline efloat<nlimbs> versine(const efloat<nlimbs>& r) {
		const double   precision = static_cast<double>(r.get_precision());
		const unsigned halvings  = static_cast<unsigned>(0.25 * std::sqrt(precision)) + 6u;
		const unsigned u         = std::max(2u, static_cast<unsigned>(std::lround(0.21 * std::pow(precision, 0.35))));
		return versine_with(r, halvings, u);
	}

	// sin(r) and cos(r) for |r| <= pi/4 from the versine
	template<unsigned nlimbs>
	inline efloat<nlimbs> sin_from_versine(const efloat<nlimbs>& v, const efloat<nlimbs>& r) {
		if (v.iszero()) return r;   // r below the precision's resolution of v: sin r = r
		efloat<nlimbs> w(v);        // v (2 - v) = 2v - v^2, by a squaring
		w.setexponent(w.scale() + 1);
		w -= v * v;
		efloat<nlimbs> s = sqrt_to(w, v.get_precision());   // v carries the h guard bits
		if (r.sign() == -1) s.setsign(true);
		return s;
	}

	template<unsigned nlimbs>
	inline efloat<nlimbs> cos_from_versine(const efloat<nlimbs>& v) {
		return 1 - v;
	}

	// trig_working(x, P) is within 2^(scale - P - exp_log_guard_bits + trig_error_bits) of the
	// function's value: measured within 2^-(P + 64 - 15), the reduction, the versine's doublings,
	// the root and tan's quotient included; the bound leaves 17 bits of margin.
	inline constexpr unsigned trig_error_bits = 32u;

	enum class trig_function { sine, cosine, tangent };

	// sin, cos or tan of a finite x != 0, unrounded, at `bits` bits plus guard bits: x reduced by
	// pi/2 to |r| <= pi/4, then the versine of r. tan at a pole the reduction cannot resolve is
	// returned as an infinity, +inf at pi/2 and -inf at -pi/2 (quadrant 3).
	template<unsigned nlimbs>
	inline efloat<nlimbs> trig_working(const efloat<nlimbs>& x, unsigned bits, trig_function f) {
		unsigned quadrant = 0;
		const efloat<nlimbs> r = reduce_half_pi(x, bits, quadrant);
		const efloat<nlimbs> v = versine(r);
		efloat<nlimbs> result;
		if (f == trig_function::sine) {
			result = (quadrant & 1u) ? cos_from_versine(v) : sin_from_versine(v, r);
			if (quadrant >= 2) result = efloat<nlimbs>(0.0) - result;
		}
		else if (f == trig_function::cosine) {
			result = (quadrant & 1u) ? sin_from_versine(v, r) : cos_from_versine(v);
			if (quadrant == 1 || quadrant == 2) result = efloat<nlimbs>(0.0) - result;
		}
		else {
			// tan(k pi/2 + r) = tan(r) for even k, -1 / tan(r) for odd k
			const efloat<nlimbs> s = sin_from_versine(v, r);
			const efloat<nlimbs> c = cos_from_versine(v);
			if (quadrant & 1u) {
				if (s.iszero()) {
					result.setinf(quadrant == 3u);
					return result;
				}
				result = efloat<nlimbs>(0.0) - c / s;
			}
			else {
				result = s / c;
			}
		}
		return result;
	}

}  // namespace efloat_detail

// The trigonometric functions compute at their result's precision plus guard bits, with pi at
// that precision, and round once at the end (#1712).  sin, cos and tan reduce by pi/2 to
// |r| <= pi/4, widening the reduction by its cancellation, so a result near a zero of the
// function keeps its full relative precision.

// ----------------------------------------------------------------------------
// sin: sine, computed by argument reduction to [-pi, pi] + Taylor series
//   sin(x) = x - x^3/3! + x^5/5! - ...   term_n = -term_{n-1} * x^2 / ((2n)(2n+1))
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> sin(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return x;               // sin(+/-0) = +/-0
	if (x.isinf()) {
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}
	const unsigned precision = efloat_detail::result_precision(x);
	// sin x = x - x^3/6 + ..., within x^2/6 < 2^-(P + 6) of x below |x| < 2^-(P + 6)/2, toward 0
	if (2 * x.scale() < -static_cast<int64_t>(precision) - 6) {
		return efloat_detail::round_near(x, -x.sign(), precision, x.carries_precision());
	}
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::trig_working(x, target, efloat_detail::trig_function::sine); },
	                                        precision, efloat_detail::trig_error_bits, x.carries_precision());
}

// ----------------------------------------------------------------------------
// cos: cosine, argument reduction to [-pi, pi] + Taylor series
//   cos(x) = 1 - x^2/2! + x^4/4! - ...   term_n = -term_{n-1} * x^2 / ((2n-1)(2n))
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> cos(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return efloat<nlimbs>(1.0);
	if (x.isinf()) {
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}
	const unsigned precision = efloat_detail::result_precision(x);
	// cos x = 1 - x^2/2 + ..., within x^2/2 < 2^-(P + 5) of 1 below |x| < 2^-(P + 6)/2, below it
	if (2 * x.scale() < -static_cast<int64_t>(precision) - 6) {
		return efloat_detail::round_near(efloat<nlimbs>(1.0), -1, precision, x.carries_precision());
	}
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::trig_working(x, target, efloat_detail::trig_function::cosine); },
	                                        precision, efloat_detail::trig_error_bits, x.carries_precision());
}

// ----------------------------------------------------------------------------
// tan: tan(x) = sin(x) / cos(x)
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> tan(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return x;
	if (x.isinf()) {
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}
	const unsigned precision = efloat_detail::result_precision(x);
	// tan x = x + x^3/3 + ..., within x^2/3 (1 + x^2) < 2^-(P + 5) of x below |x| < 2^-(P + 6)/2,
	// away from 0
	if (2 * x.scale() < -static_cast<int64_t>(precision) - 6) {
		return efloat_detail::round_near(x, x.sign(), precision, x.carries_precision());
	}
	efloat<nlimbs> result = efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::trig_working(x, target, efloat_detail::trig_function::tangent); },
	                                                         precision, efloat_detail::trig_error_bits, x.carries_precision());
	if (result.isinf() && !std::is_constant_evaluated()) efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
	return result;
}

namespace efloat_detail {

	// atan of a finite x at `working` bits, unrounded. |x| > 1 is reflected to pi/2 - atan(1/|x|),
	// and then halved h ~ sqrt(W / 10) times by atan(a) = 2 atan(a / (1 + sqrt(1 + a^2))) -- each
	// a root and a quotient -- to |a| < 2^-h, where the series a - a^3/3 + a^5/5 - ... gains 2h
	// bits per term. The running power and a^2 fall in precision with the terms, as in exp.
	template<unsigned nlimbs>
	inline efloat<nlimbs> atan_working(const efloat<nlimbs>& x, unsigned working) {
		efloat<nlimbs> a(x);
		a.setsign(false);
		a.set_precision(working);
		efloat<nlimbs> result;
		if (a == efloat<nlimbs>(1.0)) {
			result = pi_scaled<nlimbs>(working, -2);
		}
		else {
			const bool reflect = a > efloat<nlimbs>(1.0);
			if (reflect) a = 1 / a;
			const unsigned halvings = static_cast<unsigned>(std::sqrt(static_cast<double>(working) / 10.0)) + 1u;
			unsigned h = 0;
			while (h < halvings && a.scale() >= -static_cast<int64_t>(halvings)) {
				a /= sqrt_to(a * a + 1, working) + 1;
				++h;
			}
			const efloat<nlimbs> a2 = a * a;
			efloat<nlimbs> power = a;
			efloat<nlimbs> series = a;
			efloat<nlimbs> a2_taper = a2;
			unsigned taper = working;
			for (unsigned k = 1; ; ++k) {
				const int64_t below = series.scale() - power.scale();   // bits the power sits below the sum
				const unsigned need = (below + 64 < static_cast<int64_t>(working)) ? static_cast<unsigned>(static_cast<int64_t>(working) - below) : 64u;
				if (need + 32u <= taper) {
					taper = need;
					power.set_precision(taper);
					a2_taper = a2;
					a2_taper.set_precision(taper);
				}
				power *= a2_taper;
				const efloat<nlimbs> term = power / (2 * k + 1);   // stays at the taper (#1757)
				if (term.iszero() || term.scale() < series.scale() - static_cast<int64_t>(working) - 2) break;
				if (k & 1u) series -= term; else series += term;
			}
			series.setexponent(series.scale() + static_cast<int64_t>(h));
			result = reflect ? pi_scaled<nlimbs>(working, -1) - series : series;
		}
		if (x.sign() == -1) result.setsign(true);
		return result;
	}

	// asin x = atan(x / sqrt((1 - x)(1 + x))) for a finite 0 < |x| < 1 at `working` bits, unrounded;
	// 1 - |x| is exact for |x| >= 1/2, so the root keeps its relative precision as |x| -> 1
	template<unsigned nlimbs>
	inline efloat<nlimbs> asin_working(const efloat<nlimbs>& x, unsigned working) {
		efloat<nlimbs> a(x);
		a.setsign(false);
		a.set_precision(working);
		const efloat<nlimbs> root = sqrt_to((1 - a) * (a + 1), working);
		efloat<nlimbs> result = atan_working(a / root, working);
		if (x.sign() == -1) result.setsign(true);
		return result;
	}

	// acos x = 2 atan(sqrt((1 - x) / (1 + x))) for a finite -1 < x < 1 at `working` bits, unrounded:
	// 1 - x is exact near 1, where acos x -> 0, and 1 + x near -1, where it nears pi
	template<unsigned nlimbs>
	inline efloat<nlimbs> acos_working(const efloat<nlimbs>& x, unsigned working) {
		efloat<nlimbs> xw(x);
		xw.set_precision(working);
		efloat<nlimbs> result = atan_working(sqrt_to((1 - xw) / (xw + 1), working), working);
		result.setexponent(result.scale() + 1);
		return result;
	}

	// atan2(y, x) for finite nonzero x and y at `working` bits, unrounded: atan |y / x|, reflected
	// to pi - atan |y / x| for x < 0, with y's sign
	template<unsigned nlimbs>
	inline efloat<nlimbs> atan2_working(const efloat<nlimbs>& y, const efloat<nlimbs>& x, unsigned working) {
		efloat<nlimbs> q(y);
		q.set_precision(working);
		q /= x;
		q.setsign(false);
		efloat<nlimbs> result = atan_working(q, working);
		if (x.sign() == -1) result = pi<nlimbs>(working) - result;
		result.setsign(y.sign() == -1);
		return result;
	}

	// atan through the reflection, the halvings and the series; asin, acos and atan2 add a root and
	// a quotient before it
	inline constexpr unsigned inverse_trig_error_bits = 24u;

	// a function whose evaluate(working) is within 2^(scale - working + inverse_trig_error_bits) of
	// it, rounded to `precision` bits by Ziv's strategy
	template<unsigned nlimbs, typename Evaluate>
	inline efloat<nlimbs> round_inverse_trig(Evaluate evaluate, unsigned precision, bool carries) {
		return round_ziv<nlimbs>([&evaluate](unsigned target) { return evaluate(target + exp_log_guard_bits); },
		                         precision, inverse_trig_error_bits, carries);
	}

	// m pi 2^e with the sign `negative`, rounded to `precision` bits: the values atan2 and asin take
	// at their special arguments
	template<unsigned nlimbs>
	inline efloat<nlimbs> round_pi(unsigned m, int e, bool negative, unsigned precision, bool carries) {
		return round_inverse_trig<nlimbs>([m, e, negative](unsigned working) {
			efloat<nlimbs> v = pi_scaled<nlimbs>(working, e);
			if (m != 1u) v *= m;
			v.setsign(negative);
			return v;
		}, precision, carries);
	}

	// a tiny x, 2 scale(x) < -(P + 6): f(x) = x + c x^3 + ... lies within 2^-(P + 4) |x| of x
	template<unsigned nlimbs>
	inline bool inverse_trig_tiny(const efloat<nlimbs>& x, unsigned precision) {
		return 2 * x.scale() < -static_cast<int64_t>(precision) - 6;
	}

}  // namespace efloat_detail

// The inverse functions are correctly rounded (#1763): each evaluates at its result's precision
// plus guard bits, unrounded, and rounds by Ziv's strategy. Their values are transcendental at
// every argument that reaches it -- acos(1) = 0 is the one exact value, and is returned as such --
// and a tiny argument rounds as itself nudged toward the next term of its series.

// ----------------------------------------------------------------------------
// atan: inverse tangent
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> atan(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return x;
	const unsigned precision = efloat_detail::result_precision(x);
	if (x.isinf()) {                         // atan(+/-inf) = +/- pi/2
		return efloat_detail::round_pi<nlimbs>(1, -1, x.sign() == -1, precision, x.carries_precision());
	}
	// atan x = x - x^3/3 + ..., toward 0
	if (efloat_detail::inverse_trig_tiny(x, precision)) {
		return efloat_detail::round_near(x, -x.sign(), precision, x.carries_precision());
	}
	return efloat_detail::round_inverse_trig<nlimbs>([&x](unsigned working) { return efloat_detail::atan_working(x, working); },
	                                                 precision, x.carries_precision());
}

// ----------------------------------------------------------------------------
// asin: inverse sine; |x| > 1 -> NaN, x == +/-1 -> +/- pi/2
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> asin(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return x;

	const unsigned precision = efloat_detail::result_precision(x);
	efloat<nlimbs> ax(x); ax.setsign(false);  // |x|
	const efloat<nlimbs> one(1.0);
	if (x.isinf() || ax > one) {              // domain error
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}
	if (ax == one) {
		return efloat_detail::round_pi<nlimbs>(1, -1, x.sign() == -1, precision, x.carries_precision());
	}
	// asin x = x + x^3/6 + ..., away from 0
	if (efloat_detail::inverse_trig_tiny(x, precision)) {
		return efloat_detail::round_near(x, x.sign(), precision, x.carries_precision());
	}
	return efloat_detail::round_inverse_trig<nlimbs>([&x](unsigned working) { return efloat_detail::asin_working(x, working); },
	                                                 precision, x.carries_precision());
}

// ----------------------------------------------------------------------------
// acos: inverse cosine; |x| > 1 -> NaN, acos(1) = +0, acos(-1) = pi
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> acos(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	efloat<nlimbs> ax(x); ax.setsign(false);
	const efloat<nlimbs> one(1.0);
	if (x.isinf() || ax > one) {              // domain error
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		return nan;
	}
	const unsigned precision = efloat_detail::result_precision(x);
	if (x == one) {                           // +0 in every rounding mode
		efloat<nlimbs> zero(0.0);
		efloat_detail::round_result(zero, precision, x.carries_precision());
		return zero;
	}
	if (x == -one) {
		return efloat_detail::round_pi<nlimbs>(1, 0, false, precision, x.carries_precision());
	}
	return efloat_detail::round_inverse_trig<nlimbs>([&x](unsigned working) { return efloat_detail::acos_working(x, working); },
	                                                 precision, x.carries_precision());
}

// ----------------------------------------------------------------------------
// atan2: full-quadrant arctangent of y/x
// ----------------------------------------------------------------------------
template<unsigned nlimbs>
constexpr efloat<nlimbs> atan2(const efloat<nlimbs>& y, const efloat<nlimbs>& x) {
	if (y.isnan() || x.isnan()) {
		efloat<nlimbs> nan; nan.setnan();
		return nan;
	}
	const unsigned precision = efloat_detail::result_precision(x, y);
	const bool     carries   = x.carries_precision() || y.carries_precision();

	// Sign checks below use sign() == -1, not isneg(): isneg() is false for an
	// infinity (its state is Infinite, not Normal), and atan2 arguments may be
	// infinite in every branch. sign() reads the stored sign bit directly, so
	// it is correct for both normal and infinite operands (and equivalent to
	// isneg() for finite normals).
	const bool y_neg = (y.sign() == -1);
	const bool x_neg = (x.sign() == -1);

	// both infinite: y/x is indeterminate, but atan2 is defined on the
	// diagonal directions -> +/- pi/4 (x > 0) or +/- 3pi/4 (x < 0).
	if (y.isinf() && x.isinf()) {
		return efloat_detail::round_pi<nlimbs>(x_neg ? 3u : 1u, -2, y_neg, precision, carries);
	}
	// a zero y, or a finite y against an infinite x: +/- pi for x < 0, -0 included, else +/- 0
	if (y.iszero() || (x.isinf() && !y.isinf())) {
		if (x_neg) return efloat_detail::round_pi<nlimbs>(1, 0, y_neg, precision, carries);   // +/- pi
		efloat<nlimbs> zero(0.0);                       // +/- 0, at the result's precision
		zero.setsign(y_neg);
		efloat_detail::round_result(zero, precision, carries);
		return zero;
	}
	if (y.isinf() || x.iszero()) {
		return efloat_detail::round_pi<nlimbs>(1, -1, y_neg, precision, carries);   // +/- pi/2
	}

	// atan(y/x) = q - q^3/3 + ... for x > 0 and a tiny q = y/x: when q is exact, it rounds as q
	// nudged toward 0; an inexact q is not a binary fraction, and Ziv's strategy separates it
	if (!x_neg && 2 * (y.scale() - x.scale() + 1) < -static_cast<int64_t>(precision) - 6) {
		efloat<nlimbs> q(y);
		q.set_precision(precision + 64);
		q /= x;
		efloat<nlimbs> back(q);
		back.set_precision(2 * (precision + 64));
		back *= x;
		if (back == y) return efloat_detail::round_near(q, -q.sign(), precision, carries);
	}
	return efloat_detail::round_inverse_trig<nlimbs>([&y, &x](unsigned working) { return efloat_detail::atan2_working(y, x, working); },
	                                                 precision, carries);
}

}} // namespace sw::universal
