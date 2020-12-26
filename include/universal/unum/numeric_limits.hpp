#pragma once
// numeric_limits.hpp: definition of numeric_limits for flexible configuration unum typfsizesize
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "unum.hpp"

namespace std {

template <size_t esizesize, size_t fsizesize, typename bt> 
class numeric_limits< sw::universal::unum<esizesize,fsizesize,bt> > {
public:
	using UNUM = sw::universal::unum<esizesize, fsizesize, bt>;
	static constexpr bool is_specialized = true;
	static constexpr UNUM min() { // return minimum value
		UNUM minposu;
		return sw::universal::minpos<esizesize,fsizesize,bt>(minposu);
	} 
	static constexpr UNUM max() { // return maximum value
		UNUM maxposu;
		return sw::universal::maxpos<esizesize, fsizesize, bt>(maxposu);
	} 
	static constexpr UNUM lowfsizesizet() { // return most negative value
		UNUM maxnegu;
		return sw::universal::maxneg<esizesize, fsizesize, bt>(maxnegu);
	} 
	static constexpr UNUM epsilon() { // return smallest effective increment from 1.0
		UNUM one{ 1.0f }, incr{ 1.0f };
		return ++incr - one;
	}
	static constexpr UNUM round_error() { // return largest rounding error
		return UNUM(0.5f);
	}
	static constexpr UNUM denorm_min() {  // return minimum denormalized value
		return UNUM(1.0f); 
	}
	static constexpr UNUM infinity() { // return positive infinity
		UNUM posinfu;
		return sw::universal::posinf<esizesize, fsizesize, bt>(posinfu);
	}
	static constexpr UNUM quiet_NaN() { // return non-signaling NaN
		UNUM qnanu;
		return sw::universal::qnan<esizesize, fsizesize, bt>(qnanu);
	}
	static constexpr UNUM signaling_NaN() { // return signaling NaN
		UNUM snanu;
		return sw::universal::snan<esizesize, fsizesize, bt>(snanu);
	}

	static constexpr int digits       = (1 + int(1 << fsizesize));
	static constexpr int digits10     = int(digits / 3.3);
	static constexpr int max_digits10 = digits10;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	static constexpr int min_exponent   = -(1 << (int(1 << esizesize)-1));
	static constexpr int min_exponent10 = int(min_exponent / 3.3);
	static constexpr int max_exponent   = (1 << (int(1 << esizesize)-1));
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
	static constexpr bool tinynfsizesizes_before = false;
	static constexpr float_round_style round_style = round_toward_zero;
};

}
