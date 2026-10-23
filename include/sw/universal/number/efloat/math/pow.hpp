// pow.hpp: power functions for efloat
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
#include <universal/number/efloat/math/exponent.hpp>
#include <universal/number/efloat/math/logarithm.hpp>
#include <universal/number/efloat/math/classify.hpp>
#include <universal/number/efloat/math/truncate.hpp>

namespace sw { namespace universal {

namespace efloat_detail {

	// x^y = exp(y log x) carries log x's error through the product, and exp's
	inline constexpr unsigned pow_error_bits = exp_error_bits + 8u;

	// x^n by binary powering at `target` bits plus guard bits, unrounded: each squaring doubles the
	// relative error, so the n-th power carries bit_width(n) more bits; a negative exponent adds a
	// reciprocal
	inline constexpr unsigned integer_power_error_bits = 8u;

	template<unsigned nlimbs>
	inline efloat<nlimbs> integer_power_working(const efloat<nlimbs>& x, uint64_t n, bool reciprocal, unsigned target) {
		const unsigned working = target + exp_log_guard_bits + static_cast<unsigned>(std::bit_width(n)) + 2u;
		efloat<nlimbs> base(x);
		base.set_precision(working);
		efloat<nlimbs> power(1.0);
		power.set_precision(working);
		for (;;) {
			if (n & 1u) power *= base;
			n >>= 1;
			if (n == 0) break;
			base *= base;
		}
		if (reciprocal) power = 1 / power;
		return power;
	}

	// The largest exact power computed outright: x^n has at most n significant_bits(x) bits, and
	// one wider than this is left to Ziv's strategy, which separates it from the rounding
	// boundaries once its target passes the power's width
	inline unsigned pow_exact_bits(unsigned precision) { return 16u * precision + 1024u; }

	// x^n's exponent lies within n (|scale(x)| + 1) of 0: an exact power is computed only while that
	// stays inside exp's range, where the int64_t exponent cannot overflow
	template<unsigned nlimbs>
	inline bool power_in_range(const efloat<nlimbs>& x, uint64_t n) {
		return static_cast<double>(n) * (std::fabs(static_cast<double>(x.scale())) + 1.0) < exp_range_limit;
	}

	// |x|^y for a finite x != 0 and a finite y != 0 when it may lie on a rounding boundary of
	// `precision` bits, exactly: x = 2^e and an integer e y; an integer y = n > 0 with
	// n (significant_bits(x) - 1) <= precision, below which x^n has more than precision + 1
	// significant bits; or y = m / 2^k, m > 0 odd, with |x| a perfect 2^k-th power. Every other
	// |x|^y is irrational, wider than a boundary or, at a y < 0, not a binary fraction.
	template<unsigned nlimbs>
	inline bool exact_pow(const efloat<nlimbs>& x, const efloat<nlimbs>& y, unsigned precision, efloat<nlimbs>& result) {
		efloat<nlimbs> ax(x);
		ax.setsign(false);
		const unsigned ybits = significant_bits(y);
		if (significant_bits(ax) == 1) {   // 2^(e y)
			if (y.scale() >= 62) return false;
			efloat<nlimbs> ey;
			ey.set_precision(ybits + 64);
			ey = ax.scale();
			ey *= y;
			if (!is_integer(ey) || ey.scale() >= 62) return false;
			result = efloat<nlimbs>(1.0);
			result.setexponent(integer_value(ey));
			return true;
		}
		if (y.sign() == -1) return false;
		// y = m / 2^k: |x|^(1/2^k) by k exact roots, each checked by squaring back
		const int64_t k = std::max<int64_t>(0, static_cast<int64_t>(ybits) - 1 - y.scale());
		if (k == 0) {
			if (y.scale() >= 62) return false;
			const uint64_t n = static_cast<uint64_t>(double(y));
			if (static_cast<double>(n) * (significant_bits(ax) - 1) > precision || !power_in_range(ax, n)) return false;
			result = exact_power(ax, n);
			return true;
		}
		efloat<nlimbs> root(ax);
		for (int64_t i = 0; i < k; ++i) {
			const unsigned bits = significant_bits(root);
			// a square r^2 = m 2^2e, m odd, has m = 1 mod 8: its last three significant bits are 001, at an even exponent
			const std::vector<uint32_t> limbs = root.bits();
			const unsigned last = bits - 1;   // the position of the last 1 bit
			auto bit = [&limbs](unsigned p) -> uint32_t { return (limbs[p / 32] >> (31 - p % 32)) & 1u; };
			if (bits < 4 || bit(last - 1) || bit(last - 2) || ((root.scale() - static_cast<int64_t>(last)) & 1)) return false;
			const efloat<nlimbs> r = sqrt_to(root, bits + 32);
			efloat<nlimbs> square(r);
			square.set_precision(2 * (bits + 32));
			square *= r;
			if (square != root) return false;
			root = r;
		}
		efloat<nlimbs> m(y);
		m.setexponent(y.scale() + k);   // the odd integer m
		if (m.scale() >= 62) return false;
		const uint64_t n = static_cast<uint64_t>(double(m));
		if (static_cast<double>(n) * significant_bits(root) > pow_exact_bits(precision) || !power_in_range(root, n)) return false;
		result = exact_power(root, n);
		return true;
	}

	// a[0, na) = the top w limbs of p[0, np), shifted up to p's leading bit, and sh, the weight of
	// p's bit 0, moved to a's; returns whether a dropped bit was 1. p is a product of two
	// operands with their top bits set, so its own top bit is bit 63 or 62 of its top limb (#1804).
	inline bool keep_top(efloat_mpn::limb_t* a, std::size_t& na, const efloat_mpn::limb_t* p, std::size_t np, std::size_t w, int64_t& sh) {
		const unsigned c = static_cast<unsigned>(std::countl_zero(p[np - 1]));   // 0 or 1
		if (np <= w) {
			for (std::size_t i = 0; i < np; ++i) a[i] = p[i];
			if (c != 0u) efloat_mpn::lshift(a, a, np, c);
			na = np;
			sh -= c;
			return false;
		}
		const std::size_t lo = np - w;   // the first limb kept
		bool dropped = false;
		for (std::size_t i = 0; !dropped && i + 1 < lo; ++i) dropped = (p[i] != 0u);
		if (c == 0u) {
			dropped = dropped || (p[lo - 1] != 0u);
			for (std::size_t i = 0; i < w; ++i) a[i] = p[lo + i];
		}
		else {
			dropped = dropped || ((p[lo - 1] << 1) != 0u);
			for (std::size_t i = 0; i < w; ++i) a[i] = (p[lo + i] << 1) | (p[lo + i - 1] >> 63);
		}
		na = w;
		sh += 64 * static_cast<int64_t>(lo) - c;
		return dropped;
	}

	// can_round (#1763) on a nonzero value a[0, n) of 64-bit limbs, least significant first,
	// within 2^(scale - correct) of the exact result: whether its bits from position precision + 1
	// to correct - 2, counting its leading bit as 0, are neither all 0 nor all 1 (#1804)
	inline bool limbs_can_round(const efloat_mpn::limb_t* a, std::size_t n, unsigned precision, unsigned correct) {
		if (correct < precision + 4) return false;
		while (a[n - 1] == 0u) --n;
		const int64_t lead = 64 * static_cast<int64_t>(n) - 1 - std::countl_zero(a[n - 1]);   // the leading bit's index
		auto bit = [a, lead](unsigned p) -> unsigned {
			const int64_t q = lead - static_cast<int64_t>(p);
			return (q < 0) ? 0u : static_cast<unsigned>((a[q / 64] >> (q % 64)) & 1u);
		};
		const unsigned first = bit(precision + 1);
		for (unsigned p = precision + 2; p <= correct - 2; ++p) {
			if (bit(p) != first) return true;
		}
		return false;
	}

	// x^n, or x^-n with `reciprocal`, rounded once at `precision` bits, for a finite x != 0 of at
	// least 2 significant bits and an integer n >= 2 (#1804) whose power's exponent stays far
	// inside the range: binary powering on 64-bit limbs, each square and product kept to its top w limbs
	// with its leading bit at the top. A kept product is within 2^-(64 w - 1) of its exact value,
	// relative to it, and binary powering scales the error of each step by the squarings after
	// it, so x^n is within 2 n 2^-(64 w - 1) of exact: bit_width(n) + 2 bits, where efloat's own
	// arithmetic carried 69 (#1763). The reciprocal divides once more. Ziv's test (`limbs_can_round`)
	// then decides. A power that keeps every bit is exact and rounds from the limbs outright; one
	// that does not has more bits than the frame's precision + 21, so it lies on no rounding
	// boundary, and Ziv's test passes once the frame is wide enough.
	// Returns false where it does not apply, or past three widenings; pow then takes its general
	// path. Before, the general path's preamble, the exact-power attempt and efloat arithmetic at
	// 69 extra bits made pow(b, 7) at 128 bits cost about twelve multiplications.
	template<unsigned nlimbs>
	inline bool integer_power_limbs(efloat<nlimbs>& result, const efloat<nlimbs>& x, uint64_t n, bool reciprocal, unsigned precision, bool carries) {
		using efloat_mpn::limb_t;
		if (n < 2 || x.iszero() || x.isinf() || x.isnan()) return false;
		const unsigned bits = significant_bits(x);
		if (bits < 2) return false;   // a power of 2: its reciprocal is exact too, and Ziv's test never passes on it
		const int64_t ex = x.scale();
		if (static_cast<double>(n) * (std::fabs(static_cast<double>(ex)) + 1.0) >= 1152921504606846976.0) return false;   // 2^60
		const unsigned bw = static_cast<unsigned>(std::bit_width(n));
		const bool negative = (x.sign() == -1) && (n & 1u);
		const std::size_t m = (x.limb_vector().size() + 1) / 2;
		std::size_t w = (static_cast<std::size_t>(precision) + bw + 5 + 16 + 63) / 64;
		for (int attempt = 0; attempt < 3; ++attempt, w *= 2) {
			limb_buffer buffer;
			const std::size_t wq = w + 1;   // the reciprocal's limbs
			limb_t* px   = buffer.get(m + w + 2 * w + 2 * w + wq + std::max(efloat_mpn::div_scratch_limbs(w), efloat_mpn::scratch_limbs(w)));
			limb_t* a    = px + m;
			limb_t* prod = a + w;
			limb_t* num  = prod + 2 * w;
			limb_t* q    = num + 2 * w;
			limb_t* work = q + wq;
			pack_significand(px, x);
			// |x| = X 2^(ex - 64 m + 1), and the power A 2^sh, from n's leading bit down
			std::size_t na = m;
			for (std::size_t i = 0; i < m; ++i) a[i] = px[i];
			int64_t sh = ex - 64 * static_cast<int64_t>(m) + 1;
			const int64_t sx = sh;
			bool truncated = false;
			for (int b = static_cast<int>(bw) - 2; b >= 0; --b) {
				efloat_mpn::mul_n(prod, a, a, na, work, true);
				sh *= 2;
				truncated = keep_top(a, na, prod, 2 * na, w, sh) || truncated;
				if ((n >> b) & 1u) {
					efloat_mpn::mul(prod, a, na, px, m, work);
					sh += sx;
					truncated = keep_top(a, na, prod, na + m, w, sh) || truncated;
				}
			}
			const limb_t* frame   = a;
			std::size_t   len     = na;
			int64_t       top     = sh + 64 * static_cast<int64_t>(na) - 1;
			unsigned      correct = static_cast<unsigned>(64 * w) - bw - 5;
			if (reciprocal) {
				// 1 / (A 2^sh) = floor(2^(64 (na + w) - 1) / A) 2^(-sh - 64 (na + w) + 1): w quotient limbs and a top one
				const std::size_t nn = na + w;
				for (std::size_t i = 0; i + 1 < nn; ++i) num[i] = 0u;
				num[nn - 1] = limb_t(1) << 63;
				q[w]      = efloat_mpn::div_qr(q, num, nn, a, na, work);
				frame     = q;
				len       = wq;
				top       = -sh - 64 * static_cast<int64_t>(nn) + 1 + 64 * static_cast<int64_t>(wq) - 1;
				correct  -= 1;
				truncated = true;   // 1 / x^n is not a binary fraction: x is not a power of 2
			}
			if (truncated && !limbs_can_round(frame, len, 32 * ((precision + 31) / 32), correct)) continue;
			result.assign_rounded_limbs(negative, frame, len, top, truncated, (precision + 31) / 32);
			if (!carries) round_result(result, precision, carries);
			return true;
		}
		return false;
	}

	// |x|^y = exp(y log |x|) for a finite x != 0, |x| != 1, and a finite y != 0, unrounded, at
	// `target` bits plus guard bits, or, beyond exp's range, inf or 0, with `range` set to
	// +1 or -1. log x carries the scale(y log x) bits the product moves above its absolute error.
	template<unsigned nlimbs>
	inline efloat<nlimbs> pow_working(const efloat<nlimbs>& x, const efloat<nlimbs>& y, unsigned target, int& range) {
		efloat<nlimbs> ax(x);
		ax.setsign(false);
		const int64_t magnitude = y.scale() + log_magnitude(ax) + 4;   // |y log x| < 2^magnitude
		const unsigned working  = target + exp_log_guard_bits + log_error_bits + static_cast<unsigned>(std::max<int64_t>(0, magnitude));
		efloat<nlimbs> t(y);
		t.set_precision(working);
		t *= log_working(ax, working);
		range = 0;
		if (exp_beyond_range(t, log2_e)) {
			range = t.sign();
			efloat<nlimbs> limit(0.0);
			if (range > 0) limit.setinf(false);
			return limit;
		}
		return exp_working(t, target);
	}

}  // namespace efloat_detail

// Fast integer power using binary exponentiation, fully safe from INT_MIN overflow
template<unsigned nlimbs>
constexpr efloat<nlimbs> integer_power(efloat<nlimbs> base, int exponent) {
	if (exponent == 0) return efloat<nlimbs>(1.0);
	uint64_t exp_mag = 0;
	if (exponent < 0) {
		base = efloat<nlimbs>(1.0) / base;
		// Safe negation to prevent INT_MIN overflow undefined behavior
		exp_mag = 0ULL - static_cast<uint64_t>(exponent);
	} else {
		exp_mag = static_cast<uint64_t>(exponent);
	}

	efloat<nlimbs> power(1.0);
	while (exp_mag > 1) {
		if (exp_mag & 0x1) {
			power = base * power;
			base *= base;
			exp_mag = (exp_mag - 1) / 2;
		} else {
			base *= base;
			exp_mag /= 2;
		}
	}
	return base * power;
}

// pow(x, y), correctly rounded (#1763), with an integer y converted exactly
template<unsigned nlimbs>
constexpr efloat<nlimbs> pow(const efloat<nlimbs>& x, int y) {
	if (y < -1 || y > 1) {   // an integer power on limbs (#1804); y carries no precision
		const uint64_t n = (y < 0) ? 0ull - static_cast<uint64_t>(static_cast<int64_t>(y)) : static_cast<uint64_t>(y);
		efloat<nlimbs> result(x);
		if (efloat_detail::integer_power_limbs(result, x, n, y < 0, x.get_precision(), x.carries_precision())) return result;
	}
	return pow(x, efloat<nlimbs>(y));
}

// pow(x, y): correctly rounded (#1763). An exact power -- a binary fraction, which a rounding
// boundary may be -- is computed outright and rounded once; every other one is irrational or
// not a binary fraction, and rounds by Ziv's strategy. The special values follow IEEE-754.
template<unsigned nlimbs>
constexpr efloat<nlimbs> pow(const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	const unsigned precision = efloat_detail::result_precision(x, y);
	const bool     carries   = x.carries_precision() || y.carries_precision();
	// an integer y below 2^53 in magnitude: binary powering on limbs, where it applies (#1804)
	if (!y.iszero() && !y.isinf() && !y.isnan() && y.scale() < 53 && efloat_detail::is_integer(y)) {
		efloat<nlimbs> result(x);
		if (efloat_detail::integer_power_limbs(result, x, static_cast<uint64_t>(std::fabs(double(y))), y.sign() == -1, precision, carries)) return result;
	}
	// the special values below carry the result's precision, as the computed ones do
	auto special = [precision, carries](efloat<nlimbs> v) {
		efloat_detail::round_result(v, precision, carries);
		return v;
	};
	// pow(x, +/-0) is 1 for any x (including NaN)
	if (y.iszero()) {
		return special(efloat<nlimbs>(1.0));
	}
	// pow(1, y) is 1 for any y (including NaN)
	if (x == 1.0) {
		return special(efloat<nlimbs>(1.0));
	}
	if (x.isnan() || y.isnan()) {
		efloat<nlimbs> nan; nan.setnan();
		return special(nan);
	}
	const bool     y_neg     = (y.sign() == -1);
	const bool     y_integer = !y.isinf() && efloat_detail::is_integer(y);
	// whether y is an odd integer: its last significant bit is its units bit
	const bool     y_odd     = y_integer && static_cast<int64_t>(efloat_detail::significant_bits(y)) == y.scale() + 1;
	efloat<nlimbs> ax(x);
	ax.setsign(false);

	if (y.isinf()) {   // pow(-1, +/-inf) = 1; |x| > 1 grows to +inf at +inf, |x| < 1 at -inf
		if (ax == 1.0) return special(efloat<nlimbs>(1.0));
		efloat<nlimbs> limit(0.0);
		if ((ax > efloat<nlimbs>(1.0)) != y_neg) limit.setinf(false);
		return special(limit);
	}
	// a zero or infinite x: +/-0 or +/-inf, signed as x for an odd integer y
	if (x.iszero() || x.isinf()) {
		const bool negative = (x.sign() == -1) && y_odd;
		efloat<nlimbs> limit(0.0);
		if (x.iszero() == y_neg) limit.setinf(negative);
		else limit.setsign(negative);
		if (x.iszero() && y_neg && !std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		}
		return special(limit);
	}
	// a negative x has a real power only at an integer y, negative at an odd one
	if (x.sign() == -1 && !y_integer) {
		efloat<nlimbs> nan; nan.setnan();
		if (!std::is_constant_evaluated()) {
			efloat_exception_flags.set(ExceptionFlag::InvalidOperation);
		}
		return special(nan);
	}
	const bool negative = (x.sign() == -1) && y_odd;
	if (ax == 1.0) {   // (-1)^y for an integer y
		return special(efloat<nlimbs>(negative ? -1.0 : 1.0));
	}

	// an exact power is returned wherever the exponent holds it, also past exp's range
	efloat<nlimbs> result;
	if (efloat_detail::exact_pow(ax, y, precision, result)) {
		if (negative) result.setsign(true);
		efloat_detail::round_result(result, precision, carries);
		return result;
	}
	// out of exp's range where |y log2 x| >= 2^62, as exp is: from the scales, and closer to the
	// limit by the evaluation's range check
	const int64_t magnitude = y.scale() + efloat_detail::log_magnitude(ax);   // 2^magnitude <= |y log x| < 2^(magnitude + 4)
	if (magnitude >= 63) {
		efloat<nlimbs> limit = efloat_detail::exp_out_of_range<nlimbs>(y_neg != (ax < efloat<nlimbs>(1.0)), precision, carries);
		limit.setsign(negative);
		return limit;
	}
	// x^y = 1 + y log x + ..., within 2^-(P + 4) of 1 for |y log x| < 2^-(P + 5), on its side of it
	if (magnitude + 4 < -static_cast<int64_t>(precision) - 5) {
		const int direction = (y_neg == (ax < efloat<nlimbs>(1.0))) ? 1 : -1;
		return efloat_detail::round_near(efloat<nlimbs>(negative ? -1.0 : 1.0), negative ? -direction : direction, precision, carries);
	}
	// an integer y by binary powering, where the power's exponent, below 2^61 log2 e, cannot pass
	// exp's range; closer to it, the evaluation's range check decides
	if (y_integer && y.scale() < 53 && magnitude < 57) {
		const uint64_t n = static_cast<uint64_t>(std::fabs(double(y)));
		return efloat_detail::round_ziv<nlimbs>([&](unsigned target) {
			efloat<nlimbs> v = efloat_detail::integer_power_working(ax, n, y_neg, target);
			if (negative) v.setsign(true);
			return v;
		}, precision, efloat_detail::integer_power_error_bits, carries);
	}
	int range = 0;
	result = efloat_detail::round_ziv<nlimbs>([&](unsigned target) {
		efloat<nlimbs> v = efloat_detail::pow_working(ax, y, target, range);
		if (negative) v.setsign(true);
		return v;
	}, precision, efloat_detail::pow_error_bits, carries);
	if (range != 0 && !std::is_constant_evaluated()) {
		efloat_exception_flags.set((range > 0 ? ExceptionFlag::Overflow : ExceptionFlag::Underflow) | ExceptionFlag::Inexact);
	}
	return result;
}

}} // namespace sw::universal
