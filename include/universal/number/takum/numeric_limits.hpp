#pragma once
// numeric_limits.hpp: definition of numeric_limits for takum number system types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace std {

template <unsigned nbits, unsigned es, typename bt> 
class numeric_limits< sw::universal::takum<nbits,es,bt> > {
public:
	using TAKUM = sw::universal::takum<nbits,es,bt>;
	static constexpr bool is_specialized = true;
	static constexpr TAKUM  min() { // return minimum value
		TAKUM lminpos;
		return minpos<nbits, es, bt>(lminpos);
	} 
	static constexpr TAKUM  max() { // return maximum value
		TAKUM lmaxpos;
		return maxpos<nbits, es, bt>(lmaxpos);
	} 
	static constexpr TAKUM  lowest() { // return most negative value
		TAKUM lminneg;
		return minneg<nbits, es, bt>(lminneg);
	} 
	static constexpr TAKUM  epsilon() { // return smallest effective increment from 1.0
		TAKUM one{ 1.0f }, incr{ 1.0f };
		++incr;
		return incr - one;
	}
	static constexpr TAKUM  round_error() { // return largest rounding error
		return TAKUM(0.5);
	}
	static constexpr TAKUM  denorm_min() {  // return minimum denormalized value
		return TAKUM(1.0); 
	}
	static constexpr TAKUM  infinity() { // return positive infinity
		return TAKUM(INFINITY); 
	}
	static constexpr TAKUM  quiet_NaN() { // return non-signaling NaN
		return TAKUM(NAN); 
	}
	static constexpr TAKUM  signaling_NaN() { // return signaling NaN
		return TAKUM(NAN);
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
