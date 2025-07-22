#pragma once
// numeric_limits.hpp: definition of numeric_limits for double base number system types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace std {

template <unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
class numeric_limits< sw::universal::dbns<nbits, fbbits, bt, xtra...> > {
public:
	using DBNS = sw::universal::dbns<nbits, fbbits, bt, xtra...>;
	static constexpr bool is_specialized = true;
	static constexpr DBNS  min() { // return minimum value
		return DBNS(sw::universal::SpecificValue::minpos);
	} 
	static constexpr DBNS  max() { // return maximum value
		return DBNS(sw::universal::SpecificValue::maxpos);
	} 
	static constexpr DBNS  lowest() { // return most negative value
		return DBNS(sw::universal::SpecificValue::maxneg);
	} 
	static constexpr DBNS  epsilon() { // return smallest effective increment from 1.0
		DBNS one{ 1.0f }, incr{ 1.0f };
		++incr;
		return incr - one;
	}
	static constexpr DBNS  round_error() { // return largest rounding error
		return DBNS(0.5);
	}
	static constexpr DBNS  denorm_min() {  // return minimum denormalized value
		return DBNS(sw::universal::SpecificValue::minpos);
	}
	static constexpr DBNS  infinity() { // return positive infinity
		return DBNS(INFINITY); 
	}
	static constexpr DBNS  quiet_NaN() { // return non-signaling NaN
		return DBNS(NAN); 
	}
	static constexpr DBNS  signaling_NaN() { // return signaling NaN
		return DBNS(NAN);
	}

	static constexpr int digits       = -DBNS::min_exponent + fbbits;
	static constexpr int digits10     = static_cast<int>(digits / 3.3f);
	static constexpr int max_digits10 = digits10;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	static constexpr int min_exponent = DBNS::min_exponent;
	static constexpr int min_exponent10 = static_cast<int>(min_exponent / 3.3f);
	static constexpr int max_exponent = DBNS::max_exponent;
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
