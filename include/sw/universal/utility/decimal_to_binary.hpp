#pragma once
// decimal_to_binary.hpp: high-precision decimal-string to binary-mantissa converter
//                       (Phase B2a of issue #835)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// -----------------------------------------------------------------------------
// What this does
// -----------------------------------------------------------------------------
//
// Takes a decimal floating-point literal (via `string_parse::scan_decimal_float`)
// and produces an integer mantissa + binary scale + round/sticky bits with
// enough precision that any Universal number system can encode the result with
// correct round-to-nearest-even rounding.
//
// Specifically, given input "[+-]?int.frac[eE][+-]?exp" the converter
// produces a result satisfying
//
//     value = (-1)^negative * mantissa * 2^(binary_scale - target_mantissa_bits + 1)
//
// where `mantissa` is an integer whose MSB sits at bit position
// (target_mantissa_bits - 1) when the value is non-zero, plus a guard_bit and
// sticky_bit summarising the discarded tail. The caller -- posit, cfloat,
// fixpnt, dd, qd, ... -- applies its own encoding-specific rounding using
// these primitives.
//
// Algorithm
// ---------
// Build the input as an exact rational p/q (or just an integer p when the
// decimal exponent is non-negative) using multi-limb integer arithmetic
// (`einteger`). Then convert to the form (mantissa * 2^k) with rounding
// information by shifting and dividing in the bigint domain.
//
// For decimal exponent E >= 0:
//     V = M * 10^E = M * 2^E * 5^E
//     mantissa_unrounded = M * 5^E
//     binary_scale_of_LSB = E
//
// For decimal exponent E < 0:
//     V = M / 10^|E| = M / (2^|E| * 5^|E|)
//     We pick a shift K large enough that the precision-preserving quotient
//     (M << K) / 5^|E| has target_mantissa_bits + 2 valid bits. Then
//     mantissa_unrounded = (M << K) / 5^|E|,
//     binary_scale_of_LSB = -(K + |E|),
//     and the remainder of that division feeds the sticky bit.
//
// Normalize: shift the mantissa so its MSB sits at position
// (target_mantissa_bits - 1). Bits shifted off below feed the guard and
// sticky outputs.
//
// Limitations of this header
// --------------------------
// - Inputs producing more than INT64_MAX/INT64_MIN binary_scale saturate.
// - Empty mantissa (e.g. ".") is rejected upstream by scan_decimal_float.
// - "inf" / "nan" string literals are NOT handled here. Callers route them
//   separately if they want stream-extraction parity with native floats.

#include <cmath>     // std::ldexp
#include <cstdint>
#include <string_view>
#include <universal/utility/string_parse.hpp>
#include <universal/number/integer/integer.hpp>

namespace sw { namespace universal { namespace decimal_to_binary {

// Internal big-integer width used for the decimal-to-binary computation.
//
// 2048 bits comfortably covers all IEEE double-range decimal exponents
// (the bigint can reach ~ headroom + 3 * |E| bits during the shift+divide,
// which for |E| <= ~500 stays well under the budget). For higher-precision
// targets (e.g., posit<256,*>) with extreme exponents, the public template
// overload below lets callers raise the budget.
//
// einteger<> (the elastic integer) is intentionally NOT used here: it
// currently produces incorrect results for certain large-operand division
// patterns this code path exercises (a separate issue, not in scope for
// this header). The fixed-width `integer<N>` path is bit-exact under both
// gcc and clang.
constexpr unsigned default_big_bits = 2048;

template<unsigned BigBits = default_big_bits>
using big_integer = sw::universal::integer<BigBits, std::uint32_t,
                                            sw::universal::IntegerNumberType::IntegerNumber>;

// Result of the conversion. See header doc for the contract.
//
// `mantissa` is stored as a wide fixed-size `integer<>` because that's the
// type the internal arithmetic uses; callers that only need a few-dozen
// significant bits can read them out via `mantissa.test(i)` or extract a
// uint64_t.
template<unsigned BigBits = default_big_bits>
struct basic_result {
	bool                       valid;        // input parsed and produced a finite value
	bool                       negative;     // sign bit
	bool                       is_zero;      // value is exactly zero (other fields then meaningless)
	std::int64_t               binary_scale; // 2^binary_scale = the MSB weight of `mantissa`
	big_integer<BigBits>       mantissa;     // normalized: MSB at position (target_mantissa_bits - 1)
	bool                       guard_bit;    // bit just below the LSB of mantissa
	bool                       sticky_bit;   // OR of every bit below the guard
};

// Default alias for the common case.
using result = basic_result<default_big_bits>;

namespace detail {

// Build the bigint M = int_digits || frac_digits (concatenated as a single
// integer value).
// Accumulate the decimal digits into a big integer, nine at a time.
//
// The obvious loop does one full-width bigint multiply per digit. At the default
// 2048 bits that is a 64x64 limb multiply for every character, and it dominated
// the parse of anything long: 147 usec for a 62-digit literal (universal#1319).
//
// Nine digits fit in a uint32 (10^9 < 2^32), so a chunk can be accumulated in
// machine arithmetic and folded in with a single big multiply, cutting the number
// of full-width multiplies by nine.
template<unsigned BigBits>
inline big_integer<BigBits>
collect_digits(std::string_view int_part, std::string_view frac_part) {
	using Big = big_integer<BigBits>;
	constexpr std::size_t CHUNK_DIGITS = 9;
	static const int power_of_ten[CHUNK_DIGITS + 1] = {
		1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
	};

	Big M(0);
	auto absorb = [&M](std::string_view part) {
		std::size_t i = 0;
		while (i < part.size()) {
			std::size_t n = part.size() - i;
			if (n > CHUNK_DIGITS) n = CHUNK_DIGITS;
			int chunk = 0;
			for (std::size_t k = 0; k < n; ++k) chunk = chunk * 10 + static_cast<int>(part[i + k] - '0');
			M *= Big(power_of_ten[n]);
			M += Big(chunk);
			i += n;
		}
	};
	absorb(int_part);
	absorb(frac_part);
	return M;
}

// Multiply x in place by 5^e.
//
// Was a loop of e multiplications by the single-limb constant 5. Each step is
// cheap, but there are e of them: parsing 1e-300 spent 300 full-width multiplies
// here, and a 62-digit literal spent 144 usec of its 382 (universal#1319).
//
// Binary exponentiation needs O(log e) multiplications instead. The squarings are
// wider than a multiply by 5, but there are ~8 of them for e = 300 rather than 300.
// No new overflow exposure: the running base never exceeds 5^e, which the caller
// must already have room for.
template<unsigned BigBits>
inline void multiply_by_5_to_the_e(big_integer<BigBits>& x, std::int64_t e) {
	if (e <= 0) return;
	using Big = big_integer<BigBits>;
	Big base(5);
	while (e > 0) {
		if (e & 1) x *= base;
		e >>= 1;
		if (e > 0) base *= base;
	}
}

// Position of the most significant set bit, or -1 when x is zero.
//
// Walks blocks rather than bits. convert() and distill() both need this, and at
// the 2048-bit default working width the naive bit loop is 2048 iterations per
// call, several calls per parse (universal#1319).
template<unsigned BigBits>
inline int msb_position(const big_integer<BigBits>& x) noexcept {
	using Big = big_integer<BigBits>;
	static_assert(Big::bitsInBlock == 32u, "block scan assumes 32-bit limbs");
	for (int b = static_cast<int>(Big::nrBlocks) - 1; b >= 0; --b) {
		std::uint32_t blk = x.block(static_cast<unsigned>(b));
		if (blk != 0u) {
			int bit = 31;
			while (((blk >> bit) & 1u) == 0u) --bit;
			return b * 32 + bit;
		}
	}
	return -1;
}

// True when any bit strictly below position `limit` is set (the sticky test).
template<unsigned BigBits>
inline bool any_bit_below(const big_integer<BigBits>& x, int limit) noexcept {
	if (limit <= 0) return false;
	const int fullBlocks = limit / 32;
	for (int b = 0; b < fullBlocks; ++b) {
		if (x.block(static_cast<unsigned>(b)) != 0u) return true;
	}
	const int rest = limit % 32;
	if (rest > 0) {
		const std::uint32_t mask = (std::uint32_t{ 1 } << rest) - 1u;
		if ((x.block(static_cast<unsigned>(fullBlocks)) & mask) != 0u) return true;
	}
	return false;
}

// Number of bigint bits the conversion below actually needs.
//
// The widest intermediate is the numerator just before the division:
//   E >= 0:  M * 5^E                     ~ 3.33*digits + 2.33*E bits,
//            normalized up to target bits when the product is smaller
//   E <  0:  M << (target + 4 + 2.33*|E|) ~ 3.33*digits + target + 4 + 2.33*|E|
// Both estimates round up, and a 32-bit slack covers the rounding.
//
// This is an estimate of a *storage* requirement, so it must never come out
// too small: a shift that runs off the end of the fixed-width integer loses
// bits silently. test_decimal_to_binary_oracle checks the dispatched result
// against the undispatched 2048-bit path bit-for-bit.
inline std::uint64_t required_working_bits(const sw::universal::string_parse::decimal_float_scan& d,
                                           unsigned target_mantissa_bits) noexcept {
	const std::uint64_t digits = static_cast<std::uint64_t>(d.int_part.size())
	                           + static_cast<std::uint64_t>(d.frac_part.size());
	// log2(10) ~= 3.3219, log2(5) ~= 2.3219, in 1/1024ths and rounded up.
	const std::uint64_t mbits = (digits * 3402u) / 1024u + 2u;
	const std::int64_t  E = static_cast<std::int64_t>(d.exp10)
	                      - static_cast<std::int64_t>(d.frac_part.size());
	const std::uint64_t target = static_cast<std::uint64_t>(target_mantissa_bits);

	std::uint64_t need{ 0 };
	if (E >= 0) {
		need = mbits + (static_cast<std::uint64_t>(E) * 2378u) / 1024u + 2u;
		if (need < target + 2u) need = target + 2u;
	}
	else {
		const std::uint64_t neg_E = static_cast<std::uint64_t>(-E);
		need = mbits + target + 4u + (neg_E * 2378u + 1023u) / 1024u;
	}
	return need + 32u;
}

// Copy a conversion produced at one working width into the caller's width.
// The normalized mantissa occupies target_mantissa_bits + 2 bits at most, so
// this is always a widening (or identity) copy of a small value.
template<unsigned Out, unsigned Work>
inline basic_result<Out> rewidth(const basic_result<Work>& in) {
	basic_result<Out> out;
	out.valid        = in.valid;
	out.negative     = in.negative;
	out.is_zero      = in.is_zero;
	out.binary_scale = in.binary_scale;
	out.mantissa     = big_integer<Out>(in.mantissa);
	out.guard_bit    = in.guard_bit;
	out.sticky_bit   = in.sticky_bit;
	return out;
}

}  // namespace detail

// The conversion, carried out at a caller-chosen bigint width.
//
// `convert()` below is the entry point: it picks the narrowest width that can
// hold the intermediates and calls this. Use convert_at_width directly only to
// pin the width (the oracle test does, to check the dispatch against a fixed
// 2048-bit reference).
template<unsigned BigBits>
inline basic_result<BigBits>
convert_at_width(const sw::universal::string_parse::decimal_float_scan& d,
                 unsigned target_mantissa_bits) {
	using Big = big_integer<BigBits>;
	basic_result<BigBits> out;
	out.valid       = false;
	out.negative    = d.negative;
	out.is_zero     = false;
	out.binary_scale = 0;
	out.mantissa    = Big(0);
	out.guard_bit   = false;
	out.sticky_bit  = false;

	if (!d.valid) return out;

	// target_mantissa_bits == 0 is meaningless (no bits to populate). Values
	// above the internal bigint width can't be reached by normalization
	// without overflowing the storage. Either case is rejected up front so
	// the rest of the routine can assume target_mantissa_bits in [1, BigBits].
	if (target_mantissa_bits == 0u || target_mantissa_bits > BigBits) return out;

	// Build M as an integer; compute the effective decimal exponent E.
	Big M = detail::collect_digits<BigBits>(d.int_part, d.frac_part);
	{
		Big zero(0);
		if (M == zero) {
			// Zero value.
			out.is_zero = true;
			out.valid   = true;
			return out;
		}
	}
	// Effective base-10 exponent after concatenating fractional digits:
	//   value = M * 10^(d.exp10 - len(d.frac_part))
	std::int64_t E = static_cast<std::int64_t>(d.exp10)
	               - static_cast<std::int64_t>(d.frac_part.size());

	// Headroom: ask for target_mantissa_bits + 2 useful bits so the round
	// (just below the cut) and sticky (everything further below) bits are
	// reliable.
	const unsigned headroom = target_mantissa_bits + 2u;

	// Build value*2^scale as an integer in `num` (numerator).
	Big num = M;
	std::int64_t lsb_scale = 0;  // 2^lsb_scale = unit of LSB of `num`

	if (E >= 0) {
		// value = M * 10^E = M * 2^E * 5^E
		// num = M * 5^E,  lsb_scale = E.
		detail::multiply_by_5_to_the_e<BigBits>(num, E);
		lsb_scale = E;
	} else {
		// value = M / 10^|E| = M / (2^|E| * 5^|E|).
		//
		// We want num/denom = value * 2^K for some K large enough that the
		// quotient carries at least `headroom` precision bits AFTER dividing
		// by 5^|E| (which consumes ~|E| * log2(5) ~= |E| * 2.322 bits).
		//     value * 2^K = M * 2^K / (2^|E| * 5^|E|) = M * 2^(K - |E|) / 5^|E|
		// so the quotient has about bits(M) + shift - |E|*log2(5) bits, and
		// shift = headroom + ceil(|E| * log2(5)) + 2 covers `headroom` of them
		// for any M (bits(M) >= 1). The extra bits an integral shift of 3*|E|
		// would add are not free: they widen the numerator by ~0.68 bits per
		// decimal digit of exponent, and the division that follows is quadratic
		// in the limb count. At |E| = 317 that is the difference between a
		// 1024-bit working width and a 2048-bit one (universal#1319).
		//
		// 2378/1024 = 2.32227 > log2(5) = 2.321928, so the ceiling never
		// undershoots. required_working_bits() uses the same constant.
		std::int64_t neg_E = -E;
		const std::int64_t log2_of_5_to_the_E = (neg_E * 2378 + 1023) / 1024;
		const std::int64_t shift_amount = static_cast<std::int64_t>(headroom)
		                                + log2_of_5_to_the_E + 2;
		const std::int64_t K            = shift_amount + neg_E;
		num <<= static_cast<int>(shift_amount);
		Big denom(1);
		detail::multiply_by_5_to_the_e<BigBits>(denom, neg_E);
		// One long division, not two. operator/= and operator%= each run the full
		// algorithm and discard the half they were not asked for; idiv() returns
		// both. The remainder is only needed as a sticky bit, but computing it
		// separately doubled the cost of every parse with a negative exponent -
		// 23 of the 30 usec floor (universal#1319).
		auto division = sw::universal::idiv(num, denom);
		num = division.quot;
		const bool divide_residual = !division.rem.iszero();
		lsb_scale = -K;
		out.sticky_bit = divide_residual;
	}

	// Normalize: shift num so its MSB sits at position
	// (target_mantissa_bits - 1). Bits dropped off the bottom go into
	// guard and sticky.
	// Find MSB by linear scan from the top (integer<> doesn't expose a
	// dedicated findMsb in the same form as einteger; this is simple and
	// sufficient).
	int msb = detail::msb_position<BigBits>(num);
	int top = static_cast<int>(target_mantissa_bits) - 1;

	if (msb > top) {
		// Need to shift right by (msb - top). The bit at position
		// (msb - top - 1) is the guard; the OR of bits below is sticky.
		int rshift = msb - top;
		int guard_pos = rshift - 1;
		bool sticky = out.sticky_bit || detail::any_bit_below<BigBits>(num, guard_pos);
		bool guard = (guard_pos >= 0) ? num.at(static_cast<unsigned>(guard_pos)) : false;
		num >>= rshift;
		out.guard_bit  = guard;
		out.sticky_bit = sticky;
		lsb_scale += rshift;
	} else if (msb < top) {
		int lshift = top - msb;
		num <<= lshift;
		lsb_scale -= lshift;
	}

	out.mantissa     = num;
	out.binary_scale = lsb_scale + static_cast<std::int64_t>(target_mantissa_bits - 1);
	out.valid        = true;
	return out;
}

// Convert the components of a decimal-float scan into a normalized
// (sign, mantissa, binary_scale, guard, sticky) result with at least
// target_mantissa_bits significant bits of mantissa.
//
// `d` must be a successful `scan_decimal_float` result (d.valid == true).
// target_mantissa_bits is the count of explicit mantissa bits the caller
// wants (e.g., 24 for IEEE float, 53 for IEEE double, fbits+1 for a cfloat
// or fbits+regime+1 for a posit).
//
// Everything runs at the narrowest of a handful of bigint widths that can hold
// the intermediates. The width that matters is the numerator's, and it is set
// by the decimal exponent: "1.5" needs ~170 bits for a dd, 1e-300 needs ~1050.
// Running every parse at the 2048-bit worst case cost an order of magnitude on
// the common short inputs, because the bigint multiply and divide are quadratic
// in the limb count (universal#1319).
//
// The result is independent of the width chosen -- the working value is only
// ever wider than it needs to be -- so this is purely a speed dispatch. Widths
// beyond the largest instantiation (very large |exponent|, or a caller asking
// for an unusually wide mantissa) fall back to the caller's own BigBits, which
// is what the routine used to do unconditionally.
template<unsigned BigBits = default_big_bits>
inline basic_result<BigBits>
convert(const sw::universal::string_parse::decimal_float_scan& d,
        unsigned target_mantissa_bits) {
	if (!d.valid || target_mantissa_bits == 0u || target_mantissa_bits > BigBits) {
		return convert_at_width<BigBits>(d, target_mantissa_bits);
	}
	const std::uint64_t need = detail::required_working_bits(d, target_mantissa_bits);
	if constexpr (BigBits > 256u) {
		if (need <= 256u)  return detail::rewidth<BigBits>(convert_at_width<256u>(d, target_mantissa_bits));
	}
	if constexpr (BigBits > 512u) {
		if (need <= 512u)  return detail::rewidth<BigBits>(convert_at_width<512u>(d, target_mantissa_bits));
	}
	if constexpr (BigBits > 1024u) {
		if (need <= 1024u) return detail::rewidth<BigBits>(convert_at_width<1024u>(d, target_mantissa_bits));
	}
	if constexpr (BigBits > 2048u) {
		if (need <= 2048u) return detail::rewidth<BigBits>(convert_at_width<2048u>(d, target_mantissa_bits));
	}
	return convert_at_width<BigBits>(d, target_mantissa_bits);
}

// Convenience overload: parse the string in one call.
template<unsigned BigBits = default_big_bits>
inline basic_result<BigBits>
convert(std::string_view s, unsigned target_mantissa_bits) {
	auto scan = sw::universal::string_parse::scan_decimal_float(s);
	if (!scan.valid) {
		basic_result<BigBits> out{};
		out.valid = false;
		return out;
	}
	return convert<BigBits>(scan, target_mantissa_bits);
}

// ---------------------------------------------------------------------------
// distill: split a d2b conversion into N IEEE-754 doubles.
// ---------------------------------------------------------------------------
//
// Given a d2b conversion result (sign + bigint mantissa + binary_scale +
// guard/sticky), produces N doubles `out[0..N-1]` forming a canonical
// non-overlapping expansion whose sum equals the parsed value (rounded to
// the cumulative precision of the cascade, using round-to-nearest-even at
// each step).
//
// This is the bit-exact conversion path used by dd / qd / floatcascade
// parse() routines. It replaces the legacy `r *= pown(10, e)` step, whose
// pown computation accumulates ULP-level error for large |e|.
//
// Preconditions:
//   - d.valid == true (caller must check)
//   - convert() was called with target_mantissa_bits >= 53 * N + 10
//     so the bigint mantissa has enough explicit bits to feed N rounds
//     plus headroom for round/sticky. distill itself doesn't enforce this;
//     callers select target_mantissa_bits when calling convert().
//
// Postconditions:
//   - When d.is_zero: out[0..N-1] are all 0.0.
//   - Otherwise: out[0] is the round-to-nearest-double of the full value;
//     out[1] is the round-to-nearest-double of the residual; ...
//     out[N-1] is the round-to-nearest-double of the deepest residual.
//     Each out[i+1] is either 0 or satisfies |out[i+1]| <= ulp(out[i]) / 2.
//   - Components after the value has been exactly expressed are 0.0.
//
// Internal representation: we work with a SIGNED bigint residual. After
// each round-up at extraction, the residual can flip sign briefly; the
// next iteration's component then takes the opposite sign. This is the
// standard cascade canonicalization invariant.
template<unsigned BigBits, unsigned N>
inline void distill(const basic_result<BigBits>& d, double (&out)[N]) {
	for (unsigned i = 0; i < N; ++i) out[i] = 0.0;
	if (d.is_zero) return;
	using Big = big_integer<BigBits>;

	// Find MSB position of the magnitude mantissa to anchor the binary scale.
	// The d2b normalization places the mantissa MSB at the bit position that
	// has weight 2^binary_scale.
	int top_msb = detail::msb_position<BigBits>(d.mantissa);
	if (top_msb < 0) return;  // is_zero should have caught this

	// mantissa.bit[k] has weight 2^(binary_scale - top_msb + k).
	const std::int64_t lsb_weight = d.binary_scale - static_cast<std::int64_t>(top_msb);

	// Initialize residual as the signed bigint magnitude.
	Big rem = d.mantissa;
	if (d.negative) rem = -rem;

	const Big zero(0);

	for (unsigned i = 0; i < N; ++i) {
		// Magnitude + sign of the current residual.
		bool neg = (rem < zero);
		Big abs_rem = neg ? -rem : rem;
		int msb = detail::msb_position<BigBits>(abs_rem);
		if (msb < 0) return;  // residual is exactly zero -- remaining out[] stay 0.0

		// Extract top up-to-53 bits of abs_rem at positions [extract_lo, msb].
		int extract_lo = (msb >= 52) ? (msb - 52) : 0;
		const int chunk_bits = msb - extract_lo + 1;  // in [1, 53]
		std::uint64_t chunk = 0;
		for (int k = 0; k < chunk_bits; ++k) {
			if (abs_rem.at(static_cast<unsigned>(extract_lo + k))) {
				chunk |= (std::uint64_t{1} << k);
			}
		}

		// Round-to-nearest-even: round_bit is the bit just below the cut;
		// sticky is the OR of all bits further below (in the bigint).
		bool round_bit = (extract_lo > 0)
			? abs_rem.at(static_cast<unsigned>(extract_lo - 1))
			: false;
		bool sticky = detail::any_bit_below<BigBits>(abs_rem, extract_lo - 1);
		// On the first iteration the d2b residual guard/sticky bits live
		// logically BELOW position 0 in the bigint, so they only contribute
		// to sticky. Subsequent iterations track the exact integer residual
		// and the d2b guard/sticky are already discarded -- they're
		// captured by whether the previous round-up flipped rem's sign.
		if (i == 0) {
			sticky = sticky || d.guard_bit || d.sticky_bit;
		}
		const bool lsb_set = (chunk & 1u) != 0;
		const bool round_up = round_bit && (sticky || lsb_set);
		if (round_up) {
			++chunk;
			if (chunk == (std::uint64_t{1} << 53)) {
				// Mantissa overflowed: renormalize to 1.0 * 2^(exp+1).
				chunk = std::uint64_t{1} << 52;
				++msb;
				++extract_lo;
			}
		}

		// Construct the component as chunk * 2^(lsb_weight + extract_lo).
		// Clamp the std::ldexp exponent to the int range. For all realistic
		// inputs (|decimal exponent| <= ~308 for IEEE-double-range values)
		// the sum stays well within int, but defending against the extreme
		// case keeps the cast safe under any caller's target_mantissa_bits.
		const std::int64_t exp_check = lsb_weight + static_cast<std::int64_t>(extract_lo);
		double comp;
		{
			constexpr std::int64_t INT_MAX_V = static_cast<std::int64_t>((std::numeric_limits<int>::max)());
			constexpr std::int64_t INT_MIN_V = static_cast<std::int64_t>((std::numeric_limits<int>::min)());
			if (exp_check > INT_MAX_V) {
				comp = std::numeric_limits<double>::infinity();
			} else if (exp_check < INT_MIN_V) {
				comp = 0.0;
			} else {
				comp = std::ldexp(static_cast<double>(chunk), static_cast<int>(exp_check));
			}
		}
		if (neg) comp = -comp;
		out[i] = comp;

		// Subtract the rounded value from rem exactly. Skip the subtraction
		// if any bit position would exceed the bigint's storage; this only
		// triggers when chunk overflow during round-up pushed the MSB past
		// BigBits-1, which means subsequent components were going to be
		// dominated by an already-saturating value anyway.
		Big sub_val(0);
		bool out_of_range = false;
		for (int k = 0; k < 64; ++k) {
			if (chunk & (std::uint64_t{1} << k)) {
				const std::int64_t bit_idx = static_cast<std::int64_t>(extract_lo) + k;
				if (bit_idx < 0 || bit_idx >= static_cast<std::int64_t>(BigBits)) {
					out_of_range = true;
					break;
				}
				sub_val.setbit(static_cast<unsigned>(bit_idx), true);
			}
		}
		if (out_of_range) return;  // further components would be zero anyway
		if (neg) sub_val = -sub_val;
		rem -= sub_val;
	}
}

}}}  // namespace sw::universal::decimal_to_binary
