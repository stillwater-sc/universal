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

}}}  // namespace sw::universal::decimal_to_binary
