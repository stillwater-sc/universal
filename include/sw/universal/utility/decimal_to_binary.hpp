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
template<unsigned BigBits>
inline big_integer<BigBits>
collect_digits(std::string_view int_part, std::string_view frac_part) {
	using Big = big_integer<BigBits>;
	Big M(0);
	const Big ten(10);
	for (char c : int_part) {
		M *= ten;
		M += Big(static_cast<int>(c - '0'));
	}
	for (char c : frac_part) {
		M *= ten;
		M += Big(static_cast<int>(c - '0'));
	}
	return M;
}

// Multiply x in place by 5^e via repeated *= 5. O(e) bigint multiplications
// by the single-limb constant 5; each step is fast.
template<unsigned BigBits>
inline void multiply_by_5_to_the_e(big_integer<BigBits>& x, std::int64_t e) {
	if (e <= 0) return;
	using Big = big_integer<BigBits>;
	const Big five(5);
	for (std::int64_t i = 0; i < e; ++i) x *= five;
}

}  // namespace detail

// Convert the components of a decimal-float scan into a normalized
// (sign, mantissa, binary_scale, guard, sticky) result with at least
// target_mantissa_bits significant bits of mantissa.
//
// `d` must be a successful `scan_decimal_float` result (d.valid == true).
// target_mantissa_bits is the count of explicit mantissa bits the caller
// wants (e.g., 24 for IEEE float, 53 for IEEE double, fbits+1 for a cfloat
// or fbits+regime+1 for a posit).
template<unsigned BigBits = default_big_bits>
inline basic_result<BigBits>
convert(const sw::universal::string_parse::decimal_float_scan& d,
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
		// Choosing K = headroom + |E| * 4 overshoots log2(5) ~= 2.32 with
		// generous margin; cost is O(|E|) extra bigint bits, negligible vs
		// the O(|E|) work of computing 5^|E|.
		std::int64_t neg_E = -E;
		const std::int64_t shift_amount = static_cast<std::int64_t>(headroom)
		                                + 3 * neg_E;
		const std::int64_t K            = static_cast<std::int64_t>(headroom)
		                                + 4 * neg_E;
		num <<= static_cast<int>(shift_amount);
		Big denom(1);
		detail::multiply_by_5_to_the_e<BigBits>(denom, neg_E);
		Big rem = num;
		rem %= denom;
		num /= denom;
		Big zero(0);
		const bool divide_residual = (rem != zero);
		lsb_scale = -K;
		out.sticky_bit = divide_residual;
	}

	// Normalize: shift num so its MSB sits at position
	// (target_mantissa_bits - 1). Bits dropped off the bottom go into
	// guard and sticky.
	// Find MSB by linear scan from the top (integer<> doesn't expose a
	// dedicated findMsb in the same form as einteger; this is simple and
	// sufficient).
	int msb = -1;
	for (int i = static_cast<int>(BigBits) - 1; i >= 0; --i) {
		if (num.at(static_cast<unsigned>(i))) { msb = i; break; }
	}
	int top = static_cast<int>(target_mantissa_bits) - 1;

	if (msb > top) {
		// Need to shift right by (msb - top). The bit at position
		// (msb - top - 1) is the guard; the OR of bits below is sticky.
		int rshift = msb - top;
		int guard_pos = rshift - 1;
		bool sticky = out.sticky_bit;
		for (int i = 0; i < guard_pos; ++i) {
			if (num.at(static_cast<unsigned>(i))) { sticky = true; break; }
		}
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
	int top_msb = -1;
	for (int i = static_cast<int>(BigBits) - 1; i >= 0; --i) {
		if (d.mantissa.at(static_cast<unsigned>(i))) { top_msb = i; break; }
	}
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
		int msb = -1;
		for (int k = static_cast<int>(BigBits) - 1; k >= 0; --k) {
			if (abs_rem.at(static_cast<unsigned>(k))) { msb = k; break; }
		}
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
		bool sticky = false;
		for (int k = 0; k < extract_lo - 1; ++k) {
			if (abs_rem.at(static_cast<unsigned>(k))) { sticky = true; break; }
		}
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
