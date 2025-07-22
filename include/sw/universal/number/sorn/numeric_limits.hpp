#pragma once
// numeric_limits.hpp: definition of numeric_limits for SORN types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace std {

template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
class numeric_limits< sw::universal::sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> > {
public:
	using SORN = sw::universal::sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>;
	static constexpr bool is_specialized = true;
	static constexpr SORN  min() { // return minimum value
		SORN lminpos(0);
		return lminpos;
	} 
	static constexpr SORN  max() { // return maximum value
		SORN lmaxpos(0);
		return lmaxpos;
	} 
	static constexpr SORN  lowest() { // return most negative value
		SORN lmaxneg(0);
		return lmaxneg;
	} 
	static constexpr SORN  epsilon() { // return smallest effective increment from 1.0
		SORN one{ 1.0f }, incr{ 1.0f };
		++incr;
		return incr - one;
	}
	static constexpr SORN  round_error() { // return largest rounding error
		return SORN(0.5);
	}
	static constexpr SORN  denorm_min() {  // return minimum denormalized value
		return SORN(1.0); 
	}
	static constexpr SORN  infinity() { // return positive infinity
		return SORN(INFINITY); 
	}
	static constexpr SORN  quiet_NaN() { // return non-signaling NaN
		return SORN(NAN); 
	}
	static constexpr SORN  signaling_NaN() { // return signaling NaN
		return SORN(NAN);
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
