#pragma once
// numeric_limits.hpp: definition of numeric_limits for hfloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace std {

template<unsigned ndigits, unsigned es, typename bt>
class numeric_limits< sw::universal::hfloat<ndigits, es, bt> > {
public:
	using Hfloat = sw::universal::hfloat<ndigits, es, bt>;

	static constexpr bool is_specialized = true;

	static constexpr Hfloat min() {
		return Hfloat(sw::universal::SpecificValue::minpos);
	}
	static constexpr Hfloat max() {
		return Hfloat(sw::universal::SpecificValue::maxpos);
	}
	static constexpr Hfloat lowest() {
		return Hfloat(sw::universal::SpecificValue::maxneg);
	}
	static constexpr Hfloat epsilon() {
		Hfloat one(1), incr(1);
		return ++incr - one;
	}
	static constexpr Hfloat round_error() {
		return Hfloat(1); // truncation rounding: max error is 1 ULP
	}
	static constexpr Hfloat denorm_min() {
		return Hfloat(sw::universal::SpecificValue::minpos);
	}
	static constexpr Hfloat infinity() {
		return Hfloat(sw::universal::SpecificValue::maxpos); // no infinity, saturate
	}
	static constexpr Hfloat quiet_NaN() {
		return Hfloat(sw::universal::SpecificValue::zero); // no NaN
	}
	static constexpr Hfloat signaling_NaN() {
		return Hfloat(sw::universal::SpecificValue::zero); // no NaN
	}

	// digits in base-16 (hex digits of fraction)
	static constexpr int  digits       = static_cast<int>(ndigits * 4); // binary digits
	static constexpr int  digits10     = static_cast<int>(ndigits * 4 * 0.301); // approx log10(2) * binary digits
	static constexpr int  max_digits10 = digits10 + 1;
	static constexpr bool is_signed    = true;
	static constexpr bool is_integer   = false;
	static constexpr bool is_exact     = false;
	static constexpr int  radix        = 16;

	static constexpr int  min_exponent    = Hfloat::emin;
	static constexpr int  min_exponent10  = static_cast<int>(Hfloat::emin * 1.204); // log10(16)
	static constexpr int  max_exponent    = Hfloat::emax;
	static constexpr int  max_exponent10  = static_cast<int>(Hfloat::emax * 1.204);
	static constexpr bool has_infinity       = false;
	static constexpr bool has_quiet_NaN      = false;
	static constexpr bool has_signaling_NaN  = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss    = false;

	static constexpr bool is_iec559   = false;
	static constexpr bool is_bounded  = true;
	static constexpr bool is_modulo   = false;
	static constexpr bool traps       = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_toward_zero; // truncation
};

} // namespace std
