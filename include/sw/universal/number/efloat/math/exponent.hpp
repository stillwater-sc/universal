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
#include <utility>
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
		const efloat_mpn::limb_t* w = y.limb_data();   // least significant first, leading bit at 63 of the top
		const std::size_t         m = y.limb_count();
		auto bit = [w, m](unsigned p) -> uint32_t {
			const std::size_t i = p / 64;
			return (i < m) ? static_cast<uint32_t>((w[m - 1 - i] >> (63 - p % 64)) & 1u) : 0u;
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
		const efloat_mpn::limb_t* w = x.limb_data();   // least significant first, leading bit at 63 of the top
		const std::size_t         m = x.limb_count();
		for (std::size_t i = 0; i < m; ++i) {
			if (w[i] != 0u) return static_cast<unsigned>(64 * (m - i) - static_cast<std::size_t>(std::countr_zero(w[i])));
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
		const efloat_mpn::limb_t* w = x.limb_data();   // least significant first, leading bit at 63 of the top
		const std::size_t         m = x.limb_count();
		uint64_t v = 0;
		for (int64_t p = 0; p <= x.scale(); ++p) {
			const std::size_t i = static_cast<std::size_t>(p / 64);
			const uint64_t b = (i < m) ? (w[m - 1 - i] >> (63 - p % 64)) & 1u : 0u;
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

	// Scratch limbs for the exact kernels (#1802): on the stack, or a block from the thread's
	// cache when a kernel needs more (#1820)
	using limb_buffer = efloat_storage::scratch_limbs<512>;

	// the significand of a finite x != 0 as an integer P of limb_count() 64-bit limbs, least
	// significant first, with |x| = P 2^(scale(x) - 64 m + 1) for the m limbs returned (#1802):
	// the limbs x holds (#1820)
	template<unsigned nlimbs>
	inline std::size_t pack_significand(efloat_mpn::limb_t* p, const efloat<nlimbs>& x) {
		const efloat_mpn::limb_t* w = x.limb_data();
		const std::size_t         m = x.limb_count();
		for (std::size_t j = 0; j < m; ++j) p[j] = w[j];
		return m;
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

	// The fixed-point series kernel of exp (#1788) and of sin/cos's versine (#1790). A value v in
	// [0, 2^64) is held as the natural
	// number V = floor(v 2^F) in n + 1 limbs of 64 bits, least significant first, with F = 64 n
	// fraction bits, so the binary point sits on a limb boundary and every product shifts by
	// whole limbs. Nothing inside the series normalizes, merges precisions or rounds: each
	// operation truncates, and the truncations are counted in the guard bits (fixed_series).
	namespace fixed_series {

		using efloat_mpn::limb_t;

		// R[0, n) = floor(|v| 2^shift), for a finite v with |v| 2^shift < 2^(64 n)
		template<unsigned nlimbs>
		inline void to_fixed(const efloat<nlimbs>& v, int64_t shift, limb_t* R, std::size_t n) noexcept {
			for (std::size_t i = 0; i < n; ++i) R[i] = 0;
			if (v.iszero()) return;
			const limb_t*     w = v.limb_data();   // least significant first, leading bit at 63 of the top
			const std::size_t m = v.limb_count();
			// v's limb j has its bit 0 at 2^(scale(v) - 64 (m - j) + 1), at R's bit shift + that
			efloat_mpn::load_window(R, n, w, m, shift + v.scale() - 64 * static_cast<int64_t>(m) + 1);
		}

		// A[0, len) 2^(scale - 64 len + 1) as an efloat, exactly: the top bit of A[len - 1] weighs
		// 2^scale. The exponent is set raw, so a value past the range overflows when it is rounded.
		template<unsigned nlimbs>
		inline efloat<nlimbs> from_fixed(const limb_t* A, std::size_t len, int64_t scale) {
			efloat<nlimbs> v(1);
			v.set_significand(A, len);
			v.setexponent(0);
			v.set_precision(static_cast<unsigned>(64 * len));   // exact: normalizes, does not round
			v.setexponent(v.scale() + scale);
			return v;
		}

		// q[0, n) = floor(a[0, n) / d) for any d > 0: the quotient of a 2^l by d 2^l, d 2^l
		// normalized, one 2-by-1 step per limb with Moller and Granlund's reciprocal
		inline void div_1(limb_t* q, const limb_t* a, std::size_t n, limb_t d) noexcept {
			const unsigned l  = static_cast<unsigned>(std::countl_zero(d));
			const limb_t   dn = d << l;
			const limb_t   v  = efloat_mpn::invert_limb(dn);
			limb_t r = (l != 0u) ? a[n - 1] >> (64u - l) : 0u;   // below dn
			for (std::size_t i = n; i-- > 0;) {
				const limb_t n0 = (l != 0u) ? ((a[i] << l) | ((i > 0) ? a[i - 1] >> (64u - l) : 0u)) : a[i];
				efloat_mpn::udiv_qr_2by1(q[i], r, r, n0, dn, v);
			}
		}

		// out[0, outn) = floor(a[0, len) / 2^shift), the limbs past a's end zero
		inline void shift_right(limb_t* out, std::size_t outn, const limb_t* a, std::size_t len, uint64_t shift) noexcept {
			const std::size_t q = static_cast<std::size_t>(shift / 64u);
			const unsigned    b = static_cast<unsigned>(shift % 64u);
			for (std::size_t i = 0; i < outn; ++i) {
				const limb_t lo = (i + q < len) ? a[i + q] : 0u;
				const limb_t hi = (i + q + 1 < len) ? a[i + q + 1] : 0u;
				out[i] = (b == 0u) ? lo : ((lo >> b) | (hi << (64u - b)));
			}
		}

		// r[0, n) = a + b or a - b (a >= b), n limbs
		inline void add_or_sub(limb_t* r, const limb_t* a, const limb_t* b, std::size_t n, bool subtract) noexcept {
			if (subtract) efloat_mpn::sub_n(r, a, b, n); else efloat_mpn::add_n(r, a, b, n);
		}

		// log2(k!), from a table below 4096 and Stirling's series above, where it is within 2^-40
		inline double log2_factorial(unsigned k) noexcept {
			static const std::vector<double> table = [] {
				std::vector<double> t(4096);
				t[0] = 0.0;
				for (std::size_t i = 1; i < t.size(); ++i) t[i] = t[i - 1] + std::log2(static_cast<double>(i));
				return t;
			}();
			if (k < table.size()) return table[k];
			const double x = static_cast<double>(k);
			return (x * std::log(x) - x + 0.5 * std::log(2.0 * 3.141592653589793 * x) + 1.0 / (12.0 * x)) / 0.6931471805599453;
		}

		// The series sum_k (+-1)^k s^k / (d(1) d(2) ... d(k)): d(t) = t for exp, and
		// d(t) = (2t + 1)(2t + 2) for the versine's v / y^2 = (1/2) sum_k (-1)^k z^k 2 / (2k + 2)!
		// in z = y^2 (#1790). The arctangent's divisors do not chain: its series
		// atan(y) / y = sum_k (-1)^k z^k / (2k + 1), and atanh(y) / y without the signs, have their
		// own engine, arctangent_series (#1792, #1795).
		enum class divisors { exponential, versine, arctangent };

		inline constexpr limb_t divisor(divisors kind, limb_t t) noexcept {
			return (kind == divisors::exponential) ? t : (kind == divisors::versine) ? (2 * t + 1) * (2 * t + 2) : 2 * t + 1;
		}

		// log2 of the divisor of the N-th term: d(1) ... d(N), or 2N + 1 for the arctangent
		inline double log2_divisors(divisors kind, unsigned N) noexcept {
			if (kind == divisors::exponential) return log2_factorial(N);
			if (kind == divisors::versine) return log2_factorial(2 * N + 2) - 1.0;
			return std::log2(2.0 * static_cast<double>(N) + 1.0);
		}

		// the number of terms N for which s^N / (d(1) ... d(N)) < 2^-(F + 4), for s < 2^-a, a >= 1:
		// the tail past it, below twice its first term, is then below 2^-(F + 3). N a + log2 of the
		// divisors grows with N, so the least N past F + 4 is found by bisection.
		inline unsigned series_terms(int64_t a, unsigned F, divisors kind = divisors::exponential) noexcept {
			const double need = static_cast<double>(F) + 4.0;
			auto enough = [a, need, kind](unsigned N) { return static_cast<double>(N) * static_cast<double>(a) + log2_divisors(kind, N) > need; };
			unsigned lo = 1, hi = static_cast<unsigned>(need / static_cast<double>(a)) + 1u;   // enough(hi)
			while (lo < hi) {
				const unsigned mid = lo + (hi - lo) / 2;
				if (enough(mid)) hi = mid; else lo = mid + 1;
			}
			return lo;
		}

		// the block length m of the rectangular splitting: about sqrt(N), and short enough that the
		// divisor of a block, a product of m divisors up to d(N + m), fits in a limb
		inline unsigned block_length(unsigned N, divisors kind = divisors::exponential) noexcept {
			unsigned m = static_cast<unsigned>(std::ceil(std::sqrt(static_cast<double>(N))));
			if (m < 1u) m = 1u;
			while (m > 1u && static_cast<double>(m) * std::log2(static_cast<double>(divisor(kind, N + m))) >= 63.0) --m;
			return m;
		}

		// The halving target T, s = |r| / 2^h below 2^-T. Each halving costs a squaring at the end
		// and saves the series terms. T = 0.25 sqrt(F) was measured best, to within the noise of
		// a flat optimum, from 128 to 8192 bits with the tapered series (#1790): T = 4 at 128
		// bits, 23 at 8192. Without tapering the optimum was 0.4 sqrt(F) (#1788), and a cost model
		// with a scalar step at a tenth of a product chose 34 and 46 there and ran 1.8x and 1.05x
		// slower.
		inline unsigned halving_target(unsigned F) noexcept {
			const unsigned T = static_cast<unsigned>(0.25 * std::sqrt(static_cast<double>(F)) + 0.5);
			return (T < 2u) ? 2u : T;
		}

		// scratch space, kept per thread so that exp allocates once
		inline limb_t* scratch(std::size_t limbs) {
			thread_local std::vector<limb_t> space;
			if (space.size() < limbs) space.resize(limbs);
			return space.data();
		}

		// a b / B^drop truncated, na + nb - drop limbs, returned where they lie: below
		// truncated_min_limbs the exact floor, the top of the full product formed in tmp[0, na + nb)
		// as before #1807; from it a truncated short product within 2 units
		// (efloat_mpn::mul_truncated), formed in tmp
		inline const limb_t* truncated_product(const limb_t* a, std::size_t na, const limb_t* b, std::size_t nb,
		                                       std::size_t drop, limb_t* tmp, limb_t* s) noexcept {
			if (na < efloat_mpn::truncated_min_limbs || nb < efloat_mpn::truncated_min_limbs) {
				if (a == b && na == nb) efloat_mpn::mul_n(tmp, a, a, na, s, true);
				else                    efloat_mpn::mul(tmp, a, na, b, nb, s);
				return tmp + drop;
			}
			efloat_mpn::mul_truncated(tmp, a, na, b, nb, drop, s);
			return tmp;
		}

		// E[0, n + 1) = sum_k (+-1)^k s^k / (d(1) ... d(k)) 2^F, s = R 2^-F, truncated at each step,
		// by Smith's rectangular splitting; for exp it is exp(+-s). With T_k = s^k / (d(1) ... d(k))
		// = A_i P_j / D_ij for k = i m + j, A_i = T_(i m), P_j = s^j and D_ij = d(i m + 1) ...
		// d(i m + j), the series is a Horner scheme in P_m / D_im over blocks
		// B_i = sum_j P_j / D_ij, and each block is formed over the common denominator D_i(m-1)
		// with one-limb multipliers and one division. Only the m powers and the N / m Horner steps
		// are full products. For a negative argument the terms alternate; every partial value is
		// positive, as the series of exp(-s), s < 1, and of the versine are with decreasing terms.
		//
		// Tapering (#1790): block b enters the sum scaled by A_b = T_(b m) < 2^-G_b, G_b = b m a +
		// log2(d(1) ... d(b m)) for s < 2^-a, so it needs only F - G_b fraction bits: it is formed
		// on the top n - q_b limbs, 64 q_b <= G_b - 8, and its truncation, 2^-(F - 64 q_b) scaled
		// by A_b, stays below 2^-(F + 8). The Horner value moves down a block by gaining the limbs
		// the next block keeps. a = 0 turns tapering off.
		inline void series(limb_t* E, const limb_t* R, std::size_t n, bool negative, unsigned N, unsigned m,
		                   divisors kind = divisors::exponential, int64_t a = 0) {
			const std::size_t w  = n + 1;           // a value below 2^64
			const unsigned    nb = (N + m - 1) / m; // blocks
			limb_t* P   = scratch((m + 1) * w + 2 * (2 * w + 2) + 2 * (w + 1) + efloat_mpn::truncated_scratch_limbs(w, w, w - 1));
			limb_t* tmp = P + (m + 1) * w;          // a product, 2 w limbs
			limb_t* acc = tmp + (2 * w + 2);        // the Horner value, w limbs
			limb_t* pos = acc + (2 * w + 2);        // a block's numerator terms of either sign, w + 1 limbs
			limb_t* neg = pos + (w + 1);
			limb_t* s   = neg + (w + 1);
			// P_j = s^j, j = 1 .. m, n limbs each (the w-th is 0): P_j = P_(j/2)^2 or P_(j-1) s
			for (std::size_t i = 0; i < w; ++i) P[w + i] = (i < n) ? R[i] : 0u;
			for (unsigned j = 2; j <= m; ++j) {   // truncated products, within 2 units from truncated_min_limbs (#1807)
				const limb_t* A = (j % 2 == 0) ? P + (j / 2) * w : P + (j - 1) * w;
				const limb_t* B = (j % 2 == 0) ? A : R;
				const limb_t* t = truncated_product(A, n, B, n, n, tmp, s);
				for (std::size_t i = 0; i < n; ++i) P[j * w + i] = t[i];
				P[j * w + n] = 0;
			}
			std::size_t acc_q = 0;                  // the limbs the Horner value has dropped
			for (unsigned b = nb; b-- > 0;) {
				const limb_t base = static_cast<limb_t>(b) * m;
				std::size_t q = 0;
				if (a > 0 && b > 0) {
					const double G = static_cast<double>(base) * static_cast<double>(a) + log2_divisors(kind, static_cast<unsigned>(base));
					if (G >= 72.0) q = std::min(n - 1, static_cast<std::size_t>((G - 8.0) / 64.0));
				}
				const std::size_t k  = n - q;         // fraction limbs of this block
				const std::size_t kw = k + 1;
				// numerator sum_j P_j c_j, c_j = d(base + j + 1) ... d(base + m - 1), over L = c_0
				for (std::size_t i = 0; i <= kw; ++i) { pos[i] = 0; neg[i] = 0; }
				limb_t c = 1;
				for (unsigned j = m; j-- > 0;) {
					limb_t* into = (negative && (j % 2 == 1)) ? neg : pos;
					if (j == 0) {
						const limb_t t = into[k] + c;   // P_0 = 1 = 2^F
						into[k + 1] += (t < c) ? 1u : 0u;
						into[k] = t;
					}
					else {
						const limb_t carry = efloat_mpn::addmul_1(into, P + j * w + q, k, c);
						efloat_mpn::add_1(into + k, 2, carry);
						c *= divisor(kind, base + j);
					}
				}
				efloat_mpn::sub_n(pos, pos, neg, kw + 1);   // the block is positive
				div_1(tmp, pos, kw + 1, c);                  // B_b, below 2
				if (b + 1 == nb) {
					for (std::size_t i = 0; i < kw; ++i) acc[i] = tmp[i];
					acc_q = q;
					continue;
				}
				// the Horner value at this block's width: acc_q - q more limbs at the bottom, zero
				const std::size_t up = acc_q - q;
				if (up > 0) {
					for (std::size_t i = kw; i-- > 0;) acc[i] = (i >= up) ? acc[i - up] : 0u;
				}
				acc_q = q;
				// acc = B_b +- acc P_m / D_bm, D_bm = c d(base + m)
				for (std::size_t i = 0; i < kw; ++i) pos[i] = tmp[i];
				const limb_t* t = truncated_product(acc, kw, P + m * w + q, k, k, tmp, s);   // kw limbs, within 2 units
				div_1(neg, t, kw, c * divisor(kind, base + m));
				add_or_sub(acc, pos, neg, kw, negative && (m % 2 == 1));
			}
			for (std::size_t i = 0; i < w; ++i) E[i] = acc[i];
		}

		// S[0, n + 1) = sum_k (-1)^k z^k / (2k + 1) 2^F, z = Z 2^-F < 2^-a, truncated at each step:
		// atan(y) / y for z = y^2 (#1792), or, not alternating, sum_k z^k / (2k + 1) = atanh(y) / y
		// (#1795), for which every sum below is a plain one. The divisors do not chain from term to
		// term as exp's do, so a block of the rectangular splitting is
		// B_i = sum_j (-1)^j P_j / (2(i m + j) + 1), P_j = z^j, and the Horner step multiplies by
		// (-1)^m P_m alone. A block is summed in groups whose divisors' product L fits in a limb:
		// each term adds or subtracts P_j L / d_j, one multiplication by a limb, and the group
		// divides by L once. The terms alternate and decrease, so every partial sum of a group, of
		// a block and of the Horner scheme has the sign of its first term and is held as a natural
		// number.
		//
		// Tapering, as in series: block b enters scaled by z^(b m) < 2^-(b m a), so it is formed on
		// the top n - q_b limbs, 64 q_b <= b m a - 8, and its truncation stays below 2^-(F + 8).
		inline void arctangent_series(limb_t* S, const limb_t* Z, std::size_t n, unsigned N, unsigned m, int64_t a,
		                              bool alternating = true) {
			const std::size_t w  = n + 1;
			const unsigned    nb = (N + m - 1) / m; // blocks
			limb_t* P   = scratch((m + 1) * w + (2 * w + 2) + 3 * (w + 2) + efloat_mpn::truncated_scratch_limbs(w, w, w - 1));
			limb_t* tmp = P + (m + 1) * w;          // a product or a quotient, 2 w + 2 limbs
			limb_t* acc = tmp + (2 * w + 2);        // the Horner value, w + 2 limbs
			limb_t* blk = acc + (w + 2);            // a block's sum
			limb_t* grp = blk + (w + 2);            // a group's numerator
			limb_t* s   = grp + (w + 2);
			// P_j = z^j, j = 1 .. m, n limbs each (the w-th is 0)
			for (std::size_t i = 0; i < w; ++i) P[w + i] = (i < n) ? Z[i] : 0u;
			for (unsigned j = 2; j <= m; ++j) {   // truncated products, within 2 units from truncated_min_limbs (#1807)
				const limb_t* A = (j % 2 == 0) ? P + (j / 2) * w : P + (j - 1) * w;
				const limb_t* B = (j % 2 == 0) ? A : Z;
				const limb_t* t = truncated_product(A, n, B, n, n, tmp, s);
				for (std::size_t i = 0; i < n; ++i) P[j * w + i] = t[i];
				P[j * w + n] = 0;
			}
			constexpr limb_t limit = limb_t(1) << 63;
			std::size_t acc_q = 0;                  // the limbs the Horner value has dropped
			for (unsigned b = nb; b-- > 0;) {
				const limb_t base = static_cast<limb_t>(b) * m;
				std::size_t q = 0;
				if (a > 0 && b > 0) {
					const double G = static_cast<double>(base) * static_cast<double>(a);
					if (G >= 72.0) q = std::min(n - 1, static_cast<std::size_t>((G - 8.0) / 64.0));
				}
				const std::size_t k  = n - q;         // fraction limbs of this block
				const std::size_t kw = k + 1;
				for (std::size_t i = 0; i <= kw; ++i) blk[i] = 0;
				for (unsigned j = 0; j < m;) {
					// the group j .. e - 1, L = d(j) ... d(e - 1) <= 2^63
					limb_t L = 2 * (base + j) + 1;
					unsigned e = j + 1;
					while (e < m) {
						const limb_t d = 2 * (base + e) + 1;
						if (L > limit / d) break;
						L *= d;
						++e;
					}
					for (std::size_t i = 0; i <= kw; ++i) grp[i] = 0;
					for (unsigned t = j; t < e; ++t) {
						const limb_t c = L / (2 * (base + t) + 1);
						if (t == 0) {
							grp[k] = c;                   // P_0 = 1 = 2^F, the group's first term
						}
						else if (!alternating || (t - j) % 2 == 0) {
							const limb_t carry = efloat_mpn::addmul_1(grp, P + t * w + q, k, c);
							efloat_mpn::add_1(grp + k, 2, carry);
						}
						else {
							const limb_t borrow = efloat_mpn::submul_1(grp, P + t * w + q, k, c);
							efloat_mpn::sub_1(grp + k, 2, borrow);
						}
					}
					div_1(tmp, grp, kw, L);           // the group, below 2
					add_or_sub(blk, blk, tmp, kw, alternating && j % 2 == 1);
					j = e;
				}
				if (b + 1 == nb) {
					for (std::size_t i = 0; i < kw; ++i) acc[i] = blk[i];
					acc_q = q;
					continue;
				}
				// the Horner value at this block's width: acc_q - q more limbs at the bottom, zero
				const std::size_t up = acc_q - q;
				if (up > 0) {
					for (std::size_t i = kw; i-- > 0;) acc[i] = (i >= up) ? acc[i - up] : 0u;
				}
				acc_q = q;
				// acc = B_b +- acc P_m
				const limb_t* t = truncated_product(acc, kw, P + m * w + q, k, k, tmp, s);   // kw limbs, within 2 units
				add_or_sub(acc, blk, t, kw, alternating && m % 2 == 1);
			}
			for (std::size_t i = 0; i < w; ++i) S[i] = acc[i];
		}

		// E[0, n + 1) = E^2, truncated to F fraction bits; from truncated_min_limbs a truncated short
		// product within 2 units of E's last limb (#1807)
		inline void square(limb_t* E, std::size_t n) {
			const std::size_t w = n + 1;
			if (w < efloat_mpn::truncated_min_limbs) {
				limb_t* tmp = scratch(2 * w + efloat_mpn::scratch_limbs(w));
				efloat_mpn::mul_n(tmp, E, E, w, tmp + 2 * w, true);
				for (std::size_t i = 0; i < w; ++i) E[i] = tmp[n + i];
				return;
			}
			limb_t* tmp = scratch((w + 1) + efloat_mpn::truncated_scratch_limbs(w, w, n));
			efloat_mpn::mul_truncated(tmp, E, w, E, w, n, tmp + w + 1);   // w + 1 limbs, the top one 0
			for (std::size_t i = 0; i < w; ++i) E[i] = tmp[i];
		}

	}  // namespace fixed_series

	// exp(x) for a finite x in exp's range, unrounded, at `precision` bits plus guard bits.
	//
	// Range reduction: x = k ln 2 + r with |r| <= ln 2 / 2, at the working precision: the result's
	// precision plus guard bits, with ln 2 computed at that precision (#1712). r is then halved
	// h times, to s = |r| / 2^h below 2^-T, and exp(+-s) is summed in fixed point by rectangular
	// splitting and squared h times (fixed_series, #1788). T = 0.25 sqrt(F) was measured best
	// (halving_target): more halvings cost squarings and save terms.
	//
	// Error: the fixed-point values carry F >= precision + 64 + T + 16 fraction bits. R = s 2^F is
	// truncated (2^-F), each block's sum and quotient truncate (2^-F each, 2^-(F + 8) for a
	// tapered block's dropped limbs, #1790), each power and Horner product is a truncated short
	// product within 2^(1 - F) (efloat_mpn::mul_truncated, #1807): about N + 4 N / m + 2 m units
	// of 2^-F, under 2^10 at 8192 bits, and the tail lies below 2^-(F + 3). Each squaring doubles
	// the relative error and adds 2^(1 - F), so exp(+-s)^(2^h) is within 2^h (2^10 + 2) 2^-F <
	// 2^(h + 11 - F) <= 2^-(precision + 64 + 5) of its value, far inside exp_error_bits; r's own
	// rounding at the working precision adds 2^-(working + 1).
	template<unsigned nlimbs>
	inline efloat<nlimbs> exp_working(const efloat<nlimbs>& x, unsigned precision) {
		using fixed_series::limb_t;
		const unsigned T       = fixed_series::halving_target(precision + 64u + 16u);
		const unsigned working = precision + efloat_detail::exp_log_guard_bits + T;

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

		const std::size_t n = (precision + 64u + T + 16u + 63u) / 64u;   // fraction limbs
		const unsigned    F = static_cast<unsigned>(64 * n);
		const int64_t  h_signed = r.iszero() ? 0 : static_cast<int64_t>(T) + r.scale() + 1;
		const unsigned h        = (h_signed > 0) ? static_cast<unsigned>(h_signed) : 0u;
		std::vector<limb_t> E(n + 1);
		fixed_series::to_fixed(r, static_cast<int64_t>(F) - static_cast<int64_t>(h), E.data(), n);   // R, then exp(+-s)
		std::size_t top = n;
		while (top > 0 && E[top - 1] == 0u) --top;
		if (top == 0) {
			E.assign(n + 1, 0u);   // s below 2^-F: exp(s) is 1 to within the guard bits
			E[n] = 1;
		}
		else {
			const int64_t a = static_cast<int64_t>(F) - (64 * static_cast<int64_t>(top) - std::countl_zero(E[top - 1]));   // s < 2^-a
			const unsigned N = fixed_series::series_terms(a, F);
			std::vector<limb_t> R(E.begin(), E.begin() + static_cast<std::ptrdiff_t>(n));
			fixed_series::series(E.data(), R.data(), n, r.sign() == -1, N, fixed_series::block_length(N), fixed_series::divisors::exponential, a);
			for (unsigned i = 0; i < h; ++i) fixed_series::square(E.data(), n);
		}
		// exp(r) 2^k = E 2^(k - F): the top bit of E[n] weighs 2^(64 n + 63 - F) = 2^63 of it
		efloat<nlimbs> result = fixed_series::from_fixed<nlimbs>(E.data(), n + 1, 63 + k);
		result.set_precision(working);
		return result;
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
