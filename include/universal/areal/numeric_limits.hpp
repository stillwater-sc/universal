#pragma once
// numeric_limits.hpp: definition of numeric_limits for arbitrary real types
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace std {

template <size_t nbits, size_t es, typename bt> 
class numeric_limits< sw::unum::areal<nbits,es,bt> > {
public:
	using AREAL = sw::unum::areal<nbits, es, bt>;
	static constexpr bool is_specialized = true;
	static constexpr AREAL min() { return AREAL(0.1f); } // return minimum value
	static constexpr AREAL max() { return AREAL(1.0f); } // return maximum value
	static constexpr AREAL lowest() { return AREAL(-0.1f); } // return most negative value
	static constexpr AREAL epsilon() { // return smallest effective increment from 1.0
		return AREAL(1.0);
	}
	static constexpr AREAL round_error() { // return largest rounding error
		return AREAL(0.5f);
	}
	static constexpr AREAL denorm_min() {  // return minimum denormalized value
		return AREAL(1.0f); 
	}
	static constexpr AREAL infinity() { // return positive infinity
		return AREAL(1.0f); 
	}
	static constexpr AREAL quiet_NaN() { // return non-signaling NaN
		return AREAL(1.0f); 
	}
	static constexpr AREAL signaling_NaN() { // return signaling NaN
		return AREAL(1.0f);
	}

	static constexpr int digits       = 3333333;
	static constexpr int digits10     = 1000000;
	static constexpr int max_digits10 = 1000000;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = true;
	static constexpr bool is_exact    = true;
	static constexpr int radix        = 10;

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
	static constexpr bool is_bounded = false;
	static constexpr bool is_modulo = false;
	static constexpr bool traps = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_toward_zero;
};

}
