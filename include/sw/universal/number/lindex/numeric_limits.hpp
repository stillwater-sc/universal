#pragma once
// numeric_limits.hpp: definition of numeric_limits for lindex types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Every member comes from the extreme encodings, none from a value-domain formula; see the
// numeric_limits table of docs/level-index-design.md.
#include <universal/utility/decimal_digits.hpp>

namespace std {

template <unsigned nbits, unsigned lbits, typename bt>
class numeric_limits< sw::universal::lindex<nbits, lbits, bt> > {
public:
	using LINDEX = sw::universal::lindex<nbits, lbits, bt>;
	static constexpr bool is_specialized = true;
	// level 0 is evenly spaced, so the smallest positive value is also the smallest normal
	static constexpr LINDEX min()    { return LINDEX(sw::universal::SpecificValue::minpos); }   // 2^-fbits
	static constexpr LINDEX max()    { return LINDEX(sw::universal::SpecificValue::maxpos); }   // 0111...1
	static constexpr LINDEX lowest() { return LINDEX(sw::universal::SpecificValue::maxneg); }   // -max()
	// (phi(1 + u) - 1) / u = 1 + O(u) for u = 2^-fbits, so the gap above 1 is 2^-fbits to
	// first order, and 2^-fbits is the encoding of minpos
	static constexpr LINDEX epsilon()       { return min(); }
	static LINDEX round_error()             { return LINDEX(0.5); }
	static constexpr LINDEX denorm_min()    { return min(); }   // no subnormal range
	// there is no infinity, and converting one gives NaR
	static constexpr LINDEX infinity()      { return LINDEX(sw::universal::SpecificValue::nar); }
	static constexpr LINDEX quiet_NaN()     { return LINDEX(sw::universal::SpecificValue::nar); }
	static constexpr LINDEX signaling_NaN() { return LINDEX(sw::universal::SpecificValue::nar); }

	// epsilon == 2^(1 - digits)
	static constexpr int digits       = static_cast<int>(LINDEX::fbits) + 1;
	static constexpr int digits10     = sw::universal::decimal_digits10(digits);
	static constexpr int max_digits10 = sw::universal::decimal_max_digits10(digits);
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	// the radix of the fixed-point grid of l + f; the value map has no base
	static constexpr int radix        = 2;

	// min() = 2^(min_exponent - 1) = 2^-fbits
	static constexpr int min_exponent   = 1 - static_cast<int>(LINDEX::fbits);
	static constexpr int min_exponent10 = sw::universal::decimal_min_exponent10(min_exponent);
	// the true value is about log2(phi(2^lbits)), far past any int: saturate
	static constexpr int max_exponent   = std::numeric_limits<int>::max();
	static constexpr int max_exponent10 = std::numeric_limits<int>::max();
	static constexpr bool has_infinity      = false;
	static constexpr bool has_quiet_NaN     = true;   // NaR
	static constexpr bool has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss   = false;

	static constexpr bool is_iec559  = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo  = false;
	static constexpr bool traps      = false;
	static constexpr bool tinyness_before = false;
	// to nearest, ties to even, in the psi domain
	static constexpr float_round_style round_style = round_to_nearest;
};

}  // namespace std
