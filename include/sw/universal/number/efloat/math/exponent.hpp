// exponent.hpp: exponential functions (exp, exp2, exp10, expm1) for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace sw { namespace universal {

namespace efloat_detail {

	// Guard bits the exp/log family carries past the precision of its result (#1712): the
	// range reductions multiply ln 2 by k < 2^18, the series add a few hundred rounded
	// terms, and the result is rounded once to the caller's precision at the end.
	inline constexpr unsigned exp_log_guard_bits = 64u;

	// exp_working(x, P) is within 2^(scale - P - exp_log_guard_bits + exp_error_bits) of exp x
	inline constexpr unsigned exp_error_bits = 16u;

	// The precision of the result of a function of x: x's own, what the arithmetic would give.
	// The literals a function combines x with carry no precision and do not widen it (#1757).
	template<unsigned nlimbs>
	inline unsigned result_precision(const efloat<nlimbs>& x) {
		return x.get_precision();
	}

	// round a function's result once to its precision; a function of arguments that carry no
	// precision carries none either, as their arithmetic would (#1757)
	template<unsigned nlimbs>
	inline void round_result(efloat<nlimbs>& result, unsigned precision, bool carries) {
		result.set_precision(precision);
		if (!carries) result.clear_precision();
	}

	// Ziv's test (#1763): y, within 2^(scale(y) - correct) of the exact result, rounds to the same
	// `precision` bits as the result in every mode when y's bits from position precision + 1 to
	// correct - 2, counting its leading bit as 0, are neither all 0 nor all 1. The rounding
	// boundaries -- the precision-bit values and the midpoints between them -- are the values
	// whose bits past position precision are all 0, so y then lies more than 2^(scale(y) - correct)
	// from the nearest one, and so does the exact result from y's side of it.
	template<unsigned nlimbs>
	inline bool can_round(const efloat<nlimbs>& y, unsigned precision, unsigned correct) {
		if (correct < precision + 4) return false;
		const std::vector<uint32_t> limbs = y.bits();   // most significant first, leading bit at 31
		auto bit = [&limbs](unsigned p) -> uint32_t {
			const std::size_t i = p / 32;
			return (i < limbs.size()) ? (limbs[i] >> (31 - p % 32)) & 1u : 0u;
		};
		const uint32_t first = bit(precision + 1);
		for (unsigned p = precision + 2; p <= correct - 2; ++p) {
			if (bit(p) != first) return true;
		}
		return false;
	}

	// Ziv's strategy (#1763): evaluate(target) returns a function's value unrounded, within
	// 2^(scale - target - exp_log_guard_bits + error_bits) of the exact result. A value too near a
	// rounding boundary to round from is evaluated again at twice the target, and the first one
	// that can be rounded is. The functions that use it are transcendental at every argument they
	// pass here, so the exact result never lies on a boundary.
	template<unsigned nlimbs, typename Evaluate>
	inline efloat<nlimbs> round_ziv(Evaluate evaluate, unsigned precision, unsigned error_bits, bool carries) {
		unsigned target = precision;
		efloat<nlimbs> y = evaluate(target);
		for (int retry = 0; retry < 8 && !y.iszero() && !y.isinf() && !y.isnan() &&
		                    !can_round(y, precision, target + exp_log_guard_bits - error_bits); ++retry) {
			target *= 2;
			y = evaluate(target);
		}
		round_result(y, precision, carries);
		return y;
	}

	// The rounding of value + c, for a value of at most `precision` bits and a correction c of the
	// sign `direction` with |c| < 2^-(precision + 4) |value|: every rounding boundary next to value
	// lies at least 2^-(precision + 1) |value| from it, so value + c rounds as value moved 2^-(precision
	// + 32) of itself toward c does. sin, tan and exp of a tiny x, whose leading term is exact and
	// whose next one lies below any working precision, round this way.
	template<unsigned nlimbs>
	inline efloat<nlimbs> round_near(efloat<nlimbs> value, int direction, unsigned precision, bool carries) {
		const int64_t s = value.scale();
		value.set_precision(precision + 64);
		efloat<nlimbs> tick(1.0);
		tick.set_precision(precision + 64);
		tick.setexponent(s - static_cast<int64_t>(precision) - 32);
		if (direction > 0) value += tick; else value -= tick;   // exact: both fit in precision + 64 bits
		round_result(value, precision, carries);
		return value;
	}

	// atanh(1/m) = sum_k 1 / ((2k + 1) m^(2k + 1)) for an integer m >= 2, at `bits` bits.
	// The terms fall by m^2 each, so the series converges by 2 log2(m) bits per term.
	template<unsigned nlimbs>
	inline efloat<nlimbs> atanh_reciprocal(unsigned m, unsigned bits) {
		efloat<nlimbs> one(1.0);
		one.set_precision(bits);
		const efloat<nlimbs> m2(static_cast<double>(m) * static_cast<double>(m));
		efloat<nlimbs> power = one / efloat<nlimbs>(static_cast<double>(m));   // 1 / m^(2k + 1)
		efloat<nlimbs> sum = power;
		for (unsigned k = 1; ; ++k) {
			power = power / m2;
			const efloat<nlimbs> term = power / efloat<nlimbs>(static_cast<double>(2 * k + 1));
			if (term.iszero() || term.scale() < sum.scale() - static_cast<int64_t>(bits) - 2) break;
			sum += term;
		}
		return sum;
	}

	// ln 2 and ln 10 at `bits` bits, computed rather than parsed: a literal stops being right
	// at its last digit (the 59-digit ones did at ~196 bits, #1712).  ln 2 = 2 atanh(1/3) and
	// ln 10 = 3 ln 2 + ln(5/4) = 3 ln 2 + 2 atanh(1/9), each summed with 32 guard bits.  The
	// widest value computed so far is kept per thread, and narrower requests are rounded from it.
	template<unsigned nlimbs>
	inline efloat<nlimbs> ln2(unsigned bits) {
		thread_local efloat<nlimbs> cached;
		thread_local unsigned cached_bits = 0;
		if (cached_bits < bits) {
			cached = atanh_reciprocal<nlimbs>(3, bits + 32);
			cached.setexponent(cached.scale() + 1);
			cached_bits = bits;
		}
		efloat<nlimbs> value(cached);
		value.set_precision(bits);
		return value;
	}

	template<unsigned nlimbs>
	inline efloat<nlimbs> ln10(unsigned bits) {
		thread_local efloat<nlimbs> cached;
		thread_local unsigned cached_bits = 0;
		if (cached_bits < bits) {
			efloat<nlimbs> a = atanh_reciprocal<nlimbs>(9, bits + 32);
			a.setexponent(a.scale() + 1);
			cached = ln2<nlimbs>(bits + 32) * efloat<nlimbs>(3.0) + a;
			cached_bits = bits;
		}
		efloat<nlimbs> value(cached);
		value.set_precision(bits);
		return value;
	}

}  // namespace efloat_detail

namespace efloat_detail {

	// exp(x) for a finite x, |x| < 2^17, unrounded, at `precision` bits plus guard bits.
	//
	// Range reduction: x = k ln 2 + r with |r| <= ln 2 / 2, at the working precision: the result's
	// precision plus guard bits, with ln 2 computed at that precision (#1712).  r is then halved
	// h times, and the series for exp(r / 2^h) squared h times: each halving saves the series
	// about W / (h + 1.5)^2 terms for one squaring, and each squaring doubles the relative error,
	// so the working precision carries h more guard bits.
	template<unsigned nlimbs>
	inline efloat<nlimbs> exp_working(const efloat<nlimbs>& x, unsigned precision) {
		const unsigned halvings  = static_cast<unsigned>(std::sqrt(static_cast<double>(precision)));
		const unsigned working   = precision + efloat_detail::exp_log_guard_bits + halvings;
		const efloat<nlimbs> ln2 = efloat_detail::ln2<nlimbs>(working);

		double x_dbl = double(x);
		int64_t k = static_cast<int64_t>(std::round(x_dbl / 0.6931471805599453));
		efloat<nlimbs> r = x - efloat<nlimbs>(k) * ln2;
		if (!r.iszero()) r.setexponent(r.scale() - static_cast<int64_t>(halvings));

		// Taylor series for exp(r / 2^h), until the terms fall below the working precision
		efloat<nlimbs> sum(1.0);
		sum.set_precision(working);
		efloat<nlimbs> term(sum);

		// Each term lies further below the sum than the last, so it needs that many fewer bits: the
		// running term and r are carried at a precision that falls with the terms (#1714). Both are
		// cut, because a product takes the wider of its operands' precisions.
		efloat<nlimbs> r_taper = r;
		unsigned taper = working;
		for (unsigned n = 1; ; ++n) {
			const int64_t below = sum.scale() - term.scale();   // bits the term sits below the sum
			const unsigned need = (below + 64 < static_cast<int64_t>(working)) ? static_cast<unsigned>(static_cast<int64_t>(working) - below) : 64u;
			if (need + 32u <= taper) {
				taper = need;
				term.set_precision(taper);
				r_taper = r;
				r_taper.set_precision(taper);
			}
			// n carries no precision, so the quotient stays at the taper (#1757)
			term = term * r_taper / n;
			if (term.iszero() || term.scale() < sum.scale() - static_cast<int64_t>(working)) break;
			sum += term;
		}
		for (unsigned i = 0; i < halvings; ++i) sum = sum * sum;

		// Multiply by 2^k using exponent scaling
		if (k != 0) {
			sum.setexponent(sum.scale() + k);
		}
		return sum;
	}

}  // namespace efloat_detail

// Natural exponentiation function for efloat using range-reduced Taylor series
template<unsigned nlimbs>
constexpr efloat<nlimbs> exp(const efloat<nlimbs>& x) {
	if (x.iszero()) return efloat<nlimbs>(1.0);
	if (x.isnan()) return x;
	if (x.isinf()) {
		return (x.sign() == -1 ? efloat<nlimbs>(0.0) : x);
	}

	// Protect against extremely large inputs that would overflow double/int64_t during range reduction
	if (x.scale() >= 17) {
		if (x.isneg()) {
			if (!std::is_constant_evaluated()) {
				efloat_exception_flags.set(ExceptionFlag::Underflow | ExceptionFlag::Inexact);
			}
			return efloat<nlimbs>(0.0);
		} else {
			efloat<nlimbs> inf;
			inf.setinf(false);
			if (!std::is_constant_evaluated()) {
				efloat_exception_flags.set(ExceptionFlag::Overflow | ExceptionFlag::Inexact);
			}
			return inf;
		}
	}

	const unsigned precision = efloat_detail::result_precision(x);
	// exp(x) = 1 + x + ... is within 2^-(P + 4) of 1 for |x| < 2^-(P + 4), on x's side of it
	if (x.scale() < -static_cast<int64_t>(precision) - 5) {
		return efloat_detail::round_near(efloat<nlimbs>(1.0), x.sign(), precision, x.carries_precision());
	}
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::exp_working(x, target); },
	                                        precision, efloat_detail::exp_error_bits, x.carries_precision());
}

// exp2(x): exponential base 2
template<unsigned nlimbs>
constexpr efloat<nlimbs> exp2(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return efloat<nlimbs>(1.0);
	if (x.isinf()) {
		if (x.sign() == -1) return efloat<nlimbs>(0.0);
		return x;
	}

	// 2^x = exp(x ln 2), with ln 2 and the product carried at the working precision (#1712)
	const unsigned precision = efloat_detail::result_precision(x);
	efloat<nlimbs> result = exp(x * efloat_detail::ln2<nlimbs>(precision + efloat_detail::exp_log_guard_bits));
	efloat_detail::round_result(result, precision, x.carries_precision());
	return result;
}

// exp10(x): exponential base 10
template<unsigned nlimbs>
constexpr efloat<nlimbs> exp10(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return efloat<nlimbs>(1.0);
	if (x.isinf()) {
		if (x.sign() == -1) return efloat<nlimbs>(0.0);
		return x;
	}

	// 10^x = exp(x ln 10), with ln 10 and the product carried at the working precision (#1712)
	const unsigned precision = efloat_detail::result_precision(x);
	efloat<nlimbs> result = exp(x * efloat_detail::ln10<nlimbs>(precision + efloat_detail::exp_log_guard_bits));
	efloat_detail::round_result(result, precision, x.carries_precision());
	return result;
}

// expm1(x): exp(x) - 1 with Taylor series for small inputs to avoid cancellation
template<unsigned nlimbs>
constexpr efloat<nlimbs> expm1(const efloat<nlimbs>& x) {
	if (x.isnan() || x.iszero()) return x;
	if (x.isinf()) {
		if (x.sign() == -1) return efloat<nlimbs>(-1.0);
		return x;
	}

	// both paths are carried at the working precision and rounded once at the end (#1712)
	const unsigned precision = efloat_detail::result_precision(x);
	efloat<nlimbs> xw(x);
	xw.set_precision(precision + efloat_detail::exp_log_guard_bits);

	// For inputs with magnitude >= 0.375, use std formula exp(x) - 1.0
	efloat<nlimbs> abs_x(x);
	abs_x.setsign(false);
	if (abs_x >= efloat<nlimbs>(0.375)) {
		efloat<nlimbs> result = exp(xw) - efloat<nlimbs>(1.0);
		efloat_detail::round_result(result, precision, x.carries_precision());
		return result;
	}

	// Taylor series for small x: x + x^2/2! + x^3/3! + ...
	efloat<nlimbs> sum = xw;
	efloat<nlimbs> term = xw;
	efloat<nlimbs> k(1.0);
	efloat<nlimbs> one(1.0);

	while (true) {
		k = k + one;
		term = (term * xw) / k;
		if (term.iszero()) break;
		sum = sum + term;

		if (term.scale() < sum.scale() - sum.get_precision()) {
			break;
		}
	}
	efloat_detail::round_result(sum, precision, x.carries_precision());
	return sum;
}

}} // namespace sw::universal
