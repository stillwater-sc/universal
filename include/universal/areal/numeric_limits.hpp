#pragma once
// numeric_limits.hpp: definition of numeric_limits for arbitrary real types
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace std {

template <size_t nbits, size_t es, typename bt> 
class numeric_limits< sw::universal::real<nbits,es,bt> > {
public:
	using AREAL = sw::universal::real<nbits, es, bt>;
	static constexpr bool is_specialized = true;
	static constexpr AREAL min() { // return minimum value
		AREAL aminpos;
		return minpos<nbits,es,bt>(aminpos);
	} 
	static constexpr AREAL max() { // return maximum value
		AREAL amaxpos;
		return maxpos<nbits, es, bt>(amaxpos);
	} 
	static constexpr AREAL lowest() { // return most negative value
		AREAL amaxneg;
		return maxneg<nbits, es, bt>(amaxneg);
	} 
	static constexpr AREAL epsilon() { // return smallest effective increment from 1.0
		AREAL one{ 1.0f }, incr{ 1.0f };
		return ++incr - one;
	}
	static constexpr AREAL round_error() { // return largest rounding error
		return AREAL(0.5f);
	}
	static constexpr AREAL denorm_min() {  // return minimum denormalized value
		return AREAL(1.0f); 
	}
	static constexpr AREAL infinity() { // return positive infinity
		return AREAL(INFINITY); 
	}
	static constexpr AREAL quiet_NaN() { // return non-signaling NaN
		return AREAL(NAN); 
	}
	static constexpr AREAL signaling_NaN() { // return signaling NaN
		return AREAL(NAN);
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
