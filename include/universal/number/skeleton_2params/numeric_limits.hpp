#pragma once
// numeric_limits.hpp: definition of numeric_limits for twoparam number system types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace std {

template <unsigned nbits, unsigned es, typename bt> 
class numeric_limits< sw::universal::twoparam<nbits,es,bt> > {
public:
	using TWOPARAM = sw::universal::twoparam<nbits,es,bt>;
	static constexpr bool is_specialized = true;
	static constexpr TWOPARAM  min() { // return minimum value
		TWOPARAM lminpos;
		return minpos<nbits, es, bt>(lminpos);
	} 
	static constexpr TWOPARAM  max() { // return maximum value
		TWOPARAM lmaxpos;
		return maxpos<nbits, es, bt>(lmaxpos);
	} 
	static constexpr TWOPARAM  lowest() { // return most negative value
		TWOPARAM lminneg;
		return minneg<nbits, es, bt>(lminneg);
	} 
	static constexpr TWOPARAM  epsilon() { // return smallest effective increment from 1.0
		TWOPARAM one{ 1.0f }, incr{ 1.0f };
		++incr;
		return incr - one;
	}
	static constexpr TWOPARAM  round_error() { // return largest rounding error
		return TWOPARAM(0.5);
	}
	static constexpr TWOPARAM  denorm_min() {  // return minimum denormalized value
		return TWOPARAM(1.0); 
	}
	static constexpr TWOPARAM  infinity() { // return positive infinity
		return TWOPARAM(INFINITY); 
	}
	static constexpr TWOPARAM  quiet_NaN() { // return non-signaling NaN
		return TWOPARAM(NAN); 
	}
	static constexpr TWOPARAM  signaling_NaN() { // return signaling NaN
		return TWOPARAM(NAN);
	}

	static constexpr int digits       = 3333333;
	static constexpr int digits10     = 1000000;
	static constexpr int max_digits10 = 1000000;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	static constexpr int min_exponent = 0;
	static constexpr int min_exponent10 = 0;
	static constexpr int max_exponent = 0;
	static constexpr int max_exponent10 = 0;
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
