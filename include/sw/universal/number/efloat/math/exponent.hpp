// exponent.hpp: exponential functions (exp, exp2, exp10, expm1) for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace sw { namespace universal {

namespace efloat_detail {

	// Guard bits the exp/log family carries past the precision of its result (#1712): the
	// series add a few hundred rounded terms, and the result is rounded once to the caller's
	// precision at the end. exp's range reduction carries ln 2 at 64 more bits, so that k ln 2,
	// |k| < 2^62, does not draw on them.
	inline constexpr unsigned exp_log_guard_bits = 64u;

	// exp_working(x, P) is within 2^(scale - P - exp_log_guard_bits + exp_error_bits) of exp x
	inline constexpr unsigned exp_error_bits = 16u;

	// The precision of the result of a function of x: x's own, what the arithmetic would give.
	// The literals a function combines x with carry no precision and do not widen it (#1757).
	template<unsigned nlimbs>
	inline unsigned result_precision(const efloat<nlimbs>& x) {
		return x.get_precision();
	}

	// The precision of a function of x and y: the wider of the precisions they carry, as their
	// arithmetic would give; an argument that carries none does not widen it (#1757)
	template<unsigned nlimbs>
	inline unsigned result_precision(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
		if (!y.carries_precision()) return x.get_precision();
		if (!x.carries_precision()) return y.get_precision();
		return std::max(x.get_precision(), y.get_precision());
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
		const std::vector<uint32_t>& limbs = y.limb_vector();   // most significant first, leading bit at 31
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
	// that can be rounded is. The premise is about the exact result: it must differ from every
	// rounding boundary, a binary fraction. Each caller returns the arguments whose result is a
	// binary fraction before it gets here (exp2 at an integer, log2 of a power of 2, pow at an exact
	// power, ...); what reaches round_ziv is irrational, transcendental or algebraic (exp2(1/2) is
	// sqrt(2)), or rational but not a binary fraction, so never on a boundary.
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

	// The number of significant bits of a finite x != 0: from its leading bit to its last 1 bit
	template<unsigned nlimbs>
	inline unsigned significant_bits(const efloat<nlimbs>& x) {
		const std::vector<uint32_t>& limbs = x.limb_vector();   // most significant first, leading bit at 31
		for (std::size_t i = limbs.size(); i-- > 0; ) {
			if (limbs[i] != 0u) return static_cast<unsigned>(32 * i + 32 - static_cast<std::size_t>(std::countr_zero(limbs[i])));
		}
		return 0;
	}

	// whether a finite x is an integer
	template<unsigned nlimbs>
	inline bool is_integer(const efloat<nlimbs>& x) {
		return x.iszero() || (x.scale() >= 0 && static_cast<int64_t>(significant_bits(x)) <= x.scale() + 1);
	}

	// a finite integer x, |x| < 2^62, as an int64_t: its significand's bits down to the units bit
	template<unsigned nlimbs>
	inline int64_t integer_value(const efloat<nlimbs>& x) {
		if (x.iszero()) return 0;
		const std::vector<uint32_t>& limbs = x.limb_vector();   // most significant first, leading bit at 31
		uint64_t v = 0;
		for (int64_t p = 0; p <= x.scale(); ++p) {
			const std::size_t i = static_cast<std::size_t>(p / 32);
			const uint32_t b = (i < limbs.size()) ? (limbs[i] >> (31 - p % 32)) & 1u : 0u;
			v = (v << 1) | b;
		}
		return (x.sign() == -1) ? -static_cast<int64_t>(v) : static_cast<int64_t>(v);
	}

	// x^n for a finite x != 0 and n >= 1, exactly: carried at the n significant_bits(x) bits the
	// power needs, so that no partial product rounds
	template<unsigned nlimbs>
	inline efloat<nlimbs> exact_power(const efloat<nlimbs>& x, uint64_t n) {
		const unsigned bits = static_cast<unsigned>(n * significant_bits(x)) + 32u;
		efloat<nlimbs> base(x);
		base.set_precision(bits);
		efloat<nlimbs> power(1.0);
		power.set_precision(bits);
		for (;;) {
			if (n & 1u) power *= base;
			n >>= 1;
			if (n == 0) break;
			base *= base;
		}
		return power;
	}

	// the weight of a finite x != 0's last 1 bit: x is an integer multiple of 2^lowest_bit(x)
	template<unsigned nlimbs>
	inline int64_t lowest_bit(const efloat<nlimbs>& x) {
		return x.scale() - static_cast<int64_t>(significant_bits(x)) + 1;
	}

	// x carried at `bits` bits or wider, unchanged: widening only appends zeros
	template<unsigned nlimbs>
	inline efloat<nlimbs> widened(const efloat<nlimbs>& x, unsigned bits) {
		efloat<nlimbs> w(x);
		w.set_precision(std::max(bits, x.get_precision()));
		return w;
	}

	// x y exactly, for finite x and y (#1783): the product of a and b significant bits has a + b
	template<unsigned nlimbs>
	inline efloat<nlimbs> exact_product(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
		if (x.iszero() || y.iszero()) return x * y;
		const unsigned bits = significant_bits(x) + significant_bits(y);
		return widened(x, bits) * widened(y, bits);
	}

	// a + b for finite a and b, exactly, or rounding as a + b does (#1783). When the smaller one
	// lies wholly below the last bit 2^L of the larger carried at `pad` bits, the sum is the larger
	// plus a proxy of the smaller's sign and magnitude 2^(L - 2). Both a + b and the proxied sum
	// then lie strictly between the larger and its neighbour 2^(L - 1) away, so they round alike to
	// any precision whose rounding boundaries are multiples of 2^(L - 1): every precision up to
	// pad - 2 bits, and, for a square root, every precision up to (pad - 4) / 2.
	template<unsigned nlimbs>
	inline efloat<nlimbs> exact_sum(const efloat<nlimbs>& a, const efloat<nlimbs>& b, unsigned pad) {
		if (a.iszero() || b.iszero()) {
			const unsigned bits = std::max(a.get_precision(), b.get_precision());
			return widened(a, bits) + widened(b, bits);   // a zero adds exactly, with IEEE's zero signs
		}
		const bool a_larger = a.scale() >= b.scale();
		const efloat<nlimbs>& big   = a_larger ? a : b;
		const efloat<nlimbs>& small = a_larger ? b : a;
		const int64_t big_bits = std::max<int64_t>(significant_bits(big), pad);
		const int64_t last = big.scale() - big_bits + 1;   // L
		if (small.scale() < last - 2) {
			// |small| < 2^(scale(small) + 1) <= 2^(L - 2): replaced by the proxy
			const unsigned bits = static_cast<unsigned>(big.scale() - (last - 2) + 2);
			efloat<nlimbs> proxy(1);
			proxy.set_precision(bits);
			proxy.setexponent(last - 2);
			if (small.sign() == -1) proxy = -proxy;
			return widened(big, bits) + proxy;
		}
		// the sum is a multiple of 2^lo below 2^(scale(big) + 2)
		const int64_t lo = std::min(lowest_bit(a), lowest_bit(b));
		const unsigned bits = static_cast<unsigned>(big.scale() + 2 - lo);
		return widened(a, bits) + widened(b, bits);
	}

	// exp's range: exp x = 2^k exp(r), k = round(x / ln 2), for |k| < 2^62, where the result's
	// int64_t exponent holds 2^k with room to spare. b^x = exp(x ln b) is out of it where
	// |x log2 b| >= 2^62; x is converted to double only below 2^63, where it is finite.
	inline constexpr double exp_range_limit = 4611686018427387904.0;   // 2^62

	template<unsigned nlimbs>
	inline bool exp_beyond_range(const efloat<nlimbs>& x, double log2_base) {
		return x.scale() >= 63 || std::fabs(double(x)) * log2_base >= exp_range_limit;
	}

	inline constexpr double log2_e  = 1.4426950408889634;
	inline constexpr double log2_10 = 3.321928094887362;

	// exp of an argument beyond exp's range: 0 below and inf above, inexact, at the result's
	// precision
	template<unsigned nlimbs>
	inline efloat<nlimbs> exp_out_of_range(bool negative, unsigned precision, bool carries) {
		efloat<nlimbs> result(0.0);
		if (!negative) result.setinf(false);
		round_result(result, precision, carries);
		efloat_exception_flags.set((negative ? ExceptionFlag::Underflow : ExceptionFlag::Overflow) | ExceptionFlag::Inexact);
		return result;
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

	// exp(x) for a finite x in exp's range, unrounded, at `precision` bits plus guard bits.
	//
	// Range reduction: x = k ln 2 + r with |r| <= ln 2 / 2, at the working precision: the result's
	// precision plus guard bits, with ln 2 computed at that precision (#1712).  r is then halved
	// h times, and the series for exp(r / 2^h) squared h times: each halving saves the series
	// about W / (h + 1.5)^2 terms for one squaring, and each squaring doubles the relative error,
	// so the working precision carries h more guard bits. An r already below 2^-h is halved only
	// by what brings it there, as the versine is (#1763).
	template<unsigned nlimbs>
	inline efloat<nlimbs> exp_working(const efloat<nlimbs>& x, unsigned precision) {
		const unsigned halvings  = static_cast<unsigned>(std::sqrt(static_cast<double>(precision)));
		const unsigned working   = precision + efloat_detail::exp_log_guard_bits + halvings;

		// ln 2 carries 64 more bits, so that r = x - k ln 2 keeps the working precision's absolute
		// error at any |k| < 2^62, and one width serves every k from the cache. x / ln 2 in double
		// is within 2^-53 |x| of the exact quotient, so past 2^52 k is corrected once from the r it
		// leaves, keeping |r| <= ln 2 / 2 as the halvings assume.
		int64_t k = static_cast<int64_t>(std::round(double(x) * log2_e));
		const efloat<nlimbs> ln2 = efloat_detail::ln2<nlimbs>(working + 64u);
		efloat<nlimbs> r = x - efloat<nlimbs>(k) * ln2;
		if (!r.iszero() && r.scale() >= 0) {
			k += static_cast<int64_t>(std::round(double(r) * log2_e));
			r = x - efloat<nlimbs>(k) * ln2;
		}
		r.set_precision(working);
		const int64_t  h_signed = r.iszero() ? 0 : static_cast<int64_t>(halvings) + r.scale() + 1;
		const unsigned h        = (h_signed > 0) ? static_cast<unsigned>(std::min<int64_t>(h_signed, halvings)) : 0u;
		if (!r.iszero()) r.setexponent(r.scale() - static_cast<int64_t>(h));

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
		for (unsigned i = 0; i < h; ++i) sum = sum * sum;

		// Multiply by 2^k using exponent scaling
		if (k != 0) {
			sum.setexponent(sum.scale() + k);
		}
		return sum;
	}

	// 2^x = exp(x ln 2) and 10^x = exp(x ln 10) for a finite x in exp's range, unrounded, at
	// `precision` bits plus guard bits: the product carries scale(x) + 3 more bits than exp's
	// working precision, so that its absolute error stays below 2^-(precision + guard bits)
	template<unsigned nlimbs>
	inline efloat<nlimbs> exp_base_working(const efloat<nlimbs>& x, unsigned precision, bool ten) {
		const unsigned working = precision + exp_log_guard_bits + static_cast<unsigned>(std::max<int64_t>(0, x.scale() + 3));
		efloat<nlimbs> t(x);
		t.set_precision(working);
		t *= ten ? ln10<nlimbs>(working) : ln2<nlimbs>(working);
		return exp_working(t, precision);
	}

	// exp(x) - 1 for a finite x in exp's range, unrounded, at `precision` bits plus guard bits. For
	// |x| >= 1/2, exp x - 1 cancels at most 2 bits. Below, the series for u = exp(r) - 1 is summed
	// without its leading 1, for r = x / 2^h, and doubled back h times by exp(2r) - 1 = u (u + 2),
	// so that u keeps its relative precision as x -> 0, as the versine does for cos (#1756).
	template<unsigned nlimbs>
	inline efloat<nlimbs> expm1_working(const efloat<nlimbs>& x, unsigned precision) {
		if (x.scale() >= -1) {
			efloat<nlimbs> e = exp_working(x, precision + 2);
			e -= 1;
			return e;
		}
		const unsigned halvings = static_cast<unsigned>(std::sqrt(static_cast<double>(precision)));
		const int64_t  h_signed = static_cast<int64_t>(halvings) + x.scale();   // halvings that bring r below 2^-halvings
		const unsigned h        = (h_signed > 0) ? static_cast<unsigned>(h_signed) : 0u;
		const unsigned working  = precision + exp_log_guard_bits + h;
		efloat<nlimbs> r(x);
		r.set_precision(working);
		r.setexponent(r.scale() - static_cast<int64_t>(h));

		// u = r + r^2/2! + ..., the running term and r tapered with the terms, as in exp_working
		efloat<nlimbs> sum(r);
		efloat<nlimbs> term(r);
		efloat<nlimbs> r_taper = r;
		unsigned taper = working;
		for (unsigned n = 2; ; ++n) {
			const int64_t below = sum.scale() - term.scale();   // bits the term sits below the sum
			const unsigned need = (below + 64 < static_cast<int64_t>(working)) ? static_cast<unsigned>(static_cast<int64_t>(working) - below) : 64u;
			if (need + 32u <= taper) {
				taper = need;
				term.set_precision(taper);
				r_taper = r;
				r_taper.set_precision(taper);
			}
			term = term * r_taper / n;
			if (term.iszero() || term.scale() < sum.scale() - static_cast<int64_t>(working)) break;
			sum += term;
		}
		for (unsigned i = 0; i < h; ++i) sum *= sum + 2;
		return sum;
	}

	// the derived exponentials carry their argument's rounding and a subtraction past exp's error
	inline constexpr unsigned exp_derived_error_bits = exp_error_bits + 4u;

}  // namespace efloat_detail

// Natural exponentiation function for efloat using range-reduced Taylor series
template<unsigned nlimbs>
constexpr efloat<nlimbs> exp(const efloat<nlimbs>& x) {
	if (x.iszero()) return efloat<nlimbs>(1.0);
	if (x.isnan()) return x;
	if (x.isinf()) {
		return (x.sign() == -1 ? efloat<nlimbs>(0.0) : x);
	}

	// 0 or inf beyond exp's range, where the result's exponent would not hold 2^k
	if (efloat_detail::exp_beyond_range(x, efloat_detail::log2_e)) return efloat_detail::exp_out_of_range<nlimbs>(x.sign() == -1, efloat_detail::result_precision(x), x.carries_precision());

	const unsigned precision = efloat_detail::result_precision(x);
	// exp(x) = 1 + x + ... is within 2^-(P + 4) of 1 for |x| < 2^-(P + 4), on x's side of it
	if (x.scale() < -static_cast<int64_t>(precision) - 5) {
		return efloat_detail::round_near(efloat<nlimbs>(1.0), x.sign(), precision, x.carries_precision());
	}
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::exp_working(x, target); },
	                                        precision, efloat_detail::exp_error_bits, x.carries_precision());
}

// exp2(x): exponential base 2, correctly rounded (#1763): exact at an integer x, and by Ziv's
// strategy elsewhere, where 2^x is irrational
template<unsigned nlimbs>
constexpr efloat<nlimbs> exp2(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return efloat<nlimbs>(1.0);
	if (x.isinf()) {
		if (x.sign() == -1) return efloat<nlimbs>(0.0);
		return x;
	}
	const unsigned precision = efloat_detail::result_precision(x);
	// 2^k for an integer k is exact wherever the exponent holds it, also past exp's range
	if (efloat_detail::is_integer(x) && x.scale() < 62) {
		efloat<nlimbs> power(1.0);
		power.setexponent(efloat_detail::integer_value(x));
		efloat_detail::round_result(power, precision, x.carries_precision());
		return power;
	}
	// out of exp's range where |x| >= 2^62
	if (efloat_detail::exp_beyond_range(x, 1.0)) {
		return efloat_detail::exp_out_of_range<nlimbs>(x.sign() == -1, precision, x.carries_precision());
	}
	// 2^x = 1 + x ln 2 + ..., within 2^-(P + 4) of 1 for |x| < 2^-(P + 4), on x's side of it
	if (x.scale() < -static_cast<int64_t>(precision) - 5) {
		return efloat_detail::round_near(efloat<nlimbs>(1.0), x.sign(), precision, x.carries_precision());
	}
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::exp_base_working(x, target, false); },
	                                        precision, efloat_detail::exp_derived_error_bits, x.carries_precision());
}

// exp10(x): exponential base 10, correctly rounded (#1763): exact at an integer x >= 0 whose power
// may lie on a rounding boundary, and by Ziv's strategy elsewhere, where 10^x is irrational, wider
// than a boundary or, at a negative integer, not a binary fraction
template<unsigned nlimbs>
constexpr efloat<nlimbs> exp10(const efloat<nlimbs>& x) {
	if (x.isnan()) return x;
	if (x.iszero()) return efloat<nlimbs>(1.0);
	if (x.isinf()) {
		if (x.sign() == -1) return efloat<nlimbs>(0.0);
		return x;
	}
	// out of exp's range where |x log2 10| >= 2^62
	if (efloat_detail::exp_beyond_range(x, efloat_detail::log2_10)) {
		return efloat_detail::exp_out_of_range<nlimbs>(x.sign() == -1, efloat_detail::result_precision(x), x.carries_precision());
	}

	const unsigned precision = efloat_detail::result_precision(x);
	// 10^n = 5^n 2^n lies on a rounding boundary only while 5^n has at most P + 1 significant
	// bits; a wider one is left to Ziv's strategy
	if (x.sign() == 1 && efloat_detail::is_integer(x) && double(x) * 2.321928094887362 <= static_cast<double>(precision) + 1.0) {
		efloat<nlimbs> power = efloat_detail::exact_power(efloat<nlimbs>(10.0), static_cast<uint64_t>(double(x)));
		efloat_detail::round_result(power, precision, x.carries_precision());
		return power;
	}
	// 10^x = 1 + x ln 10 + ..., within 2^-(P + 4) of 1 for |x| < 2^-(P + 6), on x's side of it
	if (x.scale() < -static_cast<int64_t>(precision) - 7) {
		return efloat_detail::round_near(efloat<nlimbs>(1.0), x.sign(), precision, x.carries_precision());
	}
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::exp_base_working(x, target, true); },
	                                        precision, efloat_detail::exp_derived_error_bits, x.carries_precision());
}

// expm1(x): exp(x) - 1, correctly rounded (#1763), with exp carried at the bits that cancel
template<unsigned nlimbs>
constexpr efloat<nlimbs> expm1(const efloat<nlimbs>& x) {
	if (x.isnan() || x.iszero()) return x;
	if (x.isinf()) {
		if (x.sign() == -1) return efloat<nlimbs>(-1.0);
		return x;
	}

	const unsigned precision = efloat_detail::result_precision(x);
	const bool carries = x.carries_precision();
	// exp(x) - 1 = x + x^2/2 + ..., within 2^-(P + 4) of x for |x| < 2^-(P + 4), above it
	if (x.scale() < -static_cast<int64_t>(precision) - 4) {
		return efloat_detail::round_near(x, 1, precision, carries);
	}
	// -1 + e^x, within 2^-(P + 6) of -1 for x < -(P + 6) ln 2, above it
	if (x.sign() == -1 && (x.scale() >= 63 || double(x) < -static_cast<double>(precision + 6) * 0.6931471805599453)) {
		return efloat_detail::round_near(efloat<nlimbs>(-1.0), 1, precision, carries);
	}
	if (efloat_detail::exp_beyond_range(x, efloat_detail::log2_e)) return efloat_detail::exp_out_of_range<nlimbs>(false, precision, carries);
	return efloat_detail::round_ziv<nlimbs>([&x](unsigned target) { return efloat_detail::expm1_working(x, target); },
	                                        precision, efloat_detail::exp_derived_error_bits, carries);
}

}} // namespace sw::universal
