#pragma once
// numeric_limits.hpp: definition of numeric_limits for faithful types
//
// Copyright (C) 2023-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace std {

template <typename FloatingPointType> 
class numeric_limits< sw::universal::faithful<FloatingPointType> > {
public:
	using FAITHFUL = sw::universal::faithful<FloatingPointType>;
	static constexpr bool is_specialized = true;
	static constexpr FAITHFUL  min() { // return minimum value
		FAITHFUL lminpos;
		return sw::universal::minpos<FloatingPointType>(lminpos);
	} 
	static constexpr FAITHFUL  max() { // return maximum value
		FAITHFUL lmaxpos;
		return sw::universal::maxpos<FloatingPointType>(lmaxpos);
	} 
	static constexpr FAITHFUL  lowest() { // return most negative value
		FAITHFUL lminneg;
		return sw::universal::minneg<FloatingPointType>(lminneg);
	} 
	static constexpr FAITHFUL  epsilon() { // return smallest effective increment from 1.0
		FAITHFUL one{ 1.0f }, incr{ 1.0f };
		++incr;
		return incr - one;
	}
	static constexpr FAITHFUL  round_error() { // return largest rounding error
		return FAITHFUL(0.5);
	}
	static constexpr FAITHFUL  denorm_min() {  // return minimum denormalized value
		return FAITHFUL(1.0); 
	}
	static constexpr FAITHFUL  infinity() { // return positive infinity
		return FAITHFUL(INFINITY); 
	}
	static constexpr FAITHFUL  quiet_NaN() { // return non-signaling NaN
		return FAITHFUL(NAN); 
	}
	static constexpr FAITHFUL  signaling_NaN() { // return signaling NaN
		return FAITHFUL(NAN);
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
