#pragma once
// numeric_limits.hpp: definition of numeric_limits for microfloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/microfloat/microfloat_fwd.hpp>
namespace std {

// general specialization for microfloat
template<unsigned nbits, unsigned es, bool hasInf, bool hasNaN, bool isSaturating>
class numeric_limits< sw::universal::microfloat<nbits, es, hasInf, hasNaN, isSaturating> > {
public:
	using Microfloat = sw::universal::microfloat<nbits, es, hasInf, hasNaN, isSaturating>;
	static constexpr unsigned fbits = nbits - 1u - es;
	static constexpr int bias = (1 << (es - 1)) - 1;

	static constexpr bool is_specialized = true;
	static constexpr Microfloat min() {
		Microfloat mf;
		// smallest positive normal: biased exponent = 1, fraction = 0
		mf.setbits(1u << fbits);
		return mf;
	}
	static constexpr Microfloat max() {
		return Microfloat(sw::universal::SpecificValue::maxpos);
	}
	static constexpr Microfloat lowest() {
		return Microfloat(sw::universal::SpecificValue::maxneg);
	}
	static Microfloat epsilon() {
		Microfloat one(1.0f), oneplus(1.0f);
		++oneplus;
		return oneplus - one;
	}
	static Microfloat round_error() {
		return Microfloat(0.5f);
	}
	static constexpr Microfloat denorm_min() {
		return Microfloat(sw::universal::SpecificValue::minpos);
	}
	static constexpr Microfloat infinity() {
		return Microfloat(sw::universal::SpecificValue::infpos);
	}
	static constexpr Microfloat quiet_NaN() {
		return Microfloat(sw::universal::SpecificValue::qnan);
	}
	static constexpr Microfloat signaling_NaN() {
		return Microfloat(sw::universal::SpecificValue::snan);
	}

	static constexpr int digits       = static_cast<int>(fbits) + 1;
	static constexpr int digits10     = static_cast<int>(digits / 3.3f);
	static constexpr int max_digits10 = digits10;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	static constexpr int min_exponent   = 1 - bias;
	static constexpr int min_exponent10 = static_cast<int>(min_exponent / 3.3f);
	static constexpr int max_exponent   = static_cast<int>((1u << es) - 1u) - bias;
	static constexpr int max_exponent10 = static_cast<int>(max_exponent / 3.3f);
	static constexpr bool has_infinity  = hasInf;
	static constexpr bool has_quiet_NaN = hasNaN;
	static constexpr bool has_signaling_NaN = hasNaN;
	static constexpr float_denorm_style has_denorm = denorm_present;
	static constexpr bool has_denorm_loss = false;

	static constexpr bool is_iec559  = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo  = false;
	static constexpr bool traps      = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_to_nearest;
};

}
