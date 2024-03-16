#pragma once
// numeric_limits.hpp: definition of numeric_limits for logarithmic types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

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

	static constexpr int digits       = -LNS::min_exponent + rbits;
	static constexpr int digits10     = static_cast<int>(digits / 3.3f);
	static constexpr int max_digits10 = digits10;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	static constexpr int min_exponent = LNS::min_exponent;
	static constexpr int min_exponent10 = static_cast<int>(min_exponent / 3.3f);
	static constexpr int max_exponent = LNS::max_exponent;
	static constexpr int max_exponent10 = static_cast<int>(max_exponent / 3.3f);
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
