#pragma once
// numeric_limits.hpp: definition of numeric_limits for logarithmic types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/decimal_digits.hpp>   // exact digit/exponent counts (#1597, #1601, #1602, #1603, #1604)

namespace std {

template <unsigned nbits, unsigned rbits, typename bt, auto... xtra>
class numeric_limits< sw::universal::lns<nbits, rbits, bt, xtra...> > {
public:
	using LNS = sw::universal::lns<nbits, rbits, bt, xtra...>;
	static constexpr bool is_specialized = true;
	static constexpr LNS  min() { // return minimum value
		return LNS(sw::universal::SpecificValue::minpos);
	} 
	static constexpr LNS  max() { // return maximum value
		return LNS(sw::universal::SpecificValue::maxpos);
	} 
	static constexpr LNS  lowest() { // return most negative value
		return LNS(sw::universal::SpecificValue::maxneg);
	} 
	static constexpr LNS  epsilon() { // return smallest effective increment from 1.0
		LNS one{ 1.0f }, incr{ 1.0f };
		++incr;
		return incr - one;
	}
	static constexpr LNS  round_error() { // return largest rounding error
		return LNS(0.5);
	}
	static constexpr LNS  denorm_min() {  // return minimum denormalized value
		return LNS(sw::universal::SpecificValue::minpos);
	}
	static constexpr LNS  infinity() { // return positive infinity
		return LNS(INFINITY); 
	}
	static constexpr LNS  quiet_NaN() { // return non-signaling NaN
		return LNS(NAN); 
	}
	static constexpr LNS  signaling_NaN() { // return signaling NaN
		return LNS(NAN);
	}

	// An lns stores a fixed-point exponent, so precision is uniform in log space and
	// there is no IEEE significand to count. Adjacent values differ by a factor
	// 2^(2^-rbits), which is an equivalent significand width of rbits + 0.5 at every
	// configuration, so rbits + 1 is the honest round-up. The previous form derived
	// digits from the EXPONENT RANGE: lns<32,8> reported 4194312 significand bits in a
	// 32-bit type (#1602).
	static constexpr int digits       = static_cast<int>(rbits) + 1;
	static constexpr int digits10     = sw::universal::decimal_digits10(digits);
	static constexpr int max_digits10 = sw::universal::decimal_max_digits10(digits);
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	static constexpr int min_exponent = sw::universal::saturate_exponent_to_int(LNS::min_exponent);
	static constexpr int min_exponent10 = sw::universal::decimal_min_exponent10(min_exponent);
	static constexpr int max_exponent = sw::universal::saturate_exponent_to_int(LNS::max_exponent);
	static constexpr int max_exponent10 = sw::universal::decimal_max_exponent10(max_exponent);
	static constexpr bool has_infinity = false;
	static constexpr bool has_quiet_NaN = false;
	static constexpr bool has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss = false;

	static constexpr bool is_iec559 = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo = false;
	static constexpr bool traps = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_toward_zero;
};

}
