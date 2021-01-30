#pragma once
// numeric_limits.hpp: definition of numeric_limits for logarithmic types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace std {

template <size_t nbits, typename bt> 
class numeric_limits< sw::universal::lns<nbits,bt> > {
public:
	using LNS = sw::universal::lns<nbits, bt>;
	static constexpr bool is_specialized = true;
	static constexpr LNS  min() { // return minimum value
		LNS lminpos;
		return minpos<nbits, bt>(lminpos);
	} 
	static constexpr LNS  max() { // return maximum value
		LNS lmaxpos;
		return maxpos<nbits, bt>(lmaxpos);
	} 
	static constexpr LNS  lowest() { // return most negative value
		LNS lminneg;
		return minneg<nbits, bt>(lminneg);
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
		return LNS(1.0); 
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
