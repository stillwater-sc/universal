#pragma once
// numeric_limits.hpp: definition of numeric_limits for classic cfloat types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/cfloat/cfloat.hpp>
namespace std {

template <size_t nbits, size_t es, typename bt> 
class numeric_limits< sw::universal::cfloat<nbits,es,bt> > {
public:
	using cfloat = sw::universal::cfloat<nbits, es, bt>;
	static constexpr bool is_specialized = true;
	static constexpr cfloat min() { // return minimum value
		return cfloat(sw::universal::SpecificValue::minpos);
	} 
	static constexpr cfloat max() { // return maximum value
		return cfloat(sw::universal::SpecificValue::maxpos);
	} 
	static constexpr cfloat lowest() { // return most negative value
		return cfloat(sw::universal::SpecificValue::maxneg);
	} 
	static constexpr cfloat epsilon() { // return smallest effective increment from 1.0
		cfloat one{ 1.0f }, incr{ 1.0f };
		return ++incr - one;
	}
	static constexpr cfloat round_error() { // return largest rounding error
		return cfloat(0.5f);
	}
	static constexpr cfloat denorm_min() {  // return minimum denormalized value
		return cfloat(1.0f); 
	}
	static constexpr cfloat infinity() { // return positive infinity
		return cfloat(INFINITY); 
	}
	static constexpr cfloat quiet_NaN() { // return non-signaling NaN
		return cfloat(NAN); 
	}
	static constexpr cfloat signaling_NaN() { // return signaling NaN
		return cfloat(NAN);
	}

	static constexpr int digits       = nbits - 1 - es + 1;
	static constexpr int digits10     = int(digits / 3.3);
	static constexpr int max_digits10 = digits10;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	static constexpr int min_exponent   = -int(1 << (es - 1));
	static constexpr int min_exponent10 = int(min_exponent / 3.3);
	static constexpr int max_exponent   = int(1 << (es - 1));
	static constexpr int max_exponent10 = int(max_exponent / 3.3);
	static constexpr bool has_infinity  = true;
	static constexpr bool has_quiet_NaN = true;
	static constexpr bool has_signaling_NaN = true;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss = false;

	static constexpr bool is_iec559 = false;
	static constexpr bool is_bounded = false;
	static constexpr bool is_modulo = false;
	static constexpr bool traps = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_toward_zero;
};

}
