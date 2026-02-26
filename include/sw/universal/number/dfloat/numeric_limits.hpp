#pragma once
// numeric_limits.hpp: definition of numeric_limits for decimal floating-point dfloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace std {

template<unsigned ndigits, unsigned es, sw::universal::DecimalEncoding Encoding, typename bt>
class numeric_limits< sw::universal::dfloat<ndigits, es, Encoding, bt> > {
public:
	using Dfloat = sw::universal::dfloat<ndigits, es, Encoding, bt>;

	static constexpr bool is_specialized = true;

	static constexpr Dfloat min() {
		return Dfloat(sw::universal::SpecificValue::minpos);
	}
	static constexpr Dfloat max() {
		return Dfloat(sw::universal::SpecificValue::maxpos);
	}
	static constexpr Dfloat lowest() {
		return Dfloat(sw::universal::SpecificValue::maxneg);
	}
	static constexpr Dfloat epsilon() {
		Dfloat one(1), incr(1);
		return ++incr - one;
	}
	static constexpr Dfloat round_error() {
		return Dfloat(0.5);
	}
	static constexpr Dfloat denorm_min() {
		return Dfloat(sw::universal::SpecificValue::minpos);
	}
	static constexpr Dfloat infinity() {
		return Dfloat(sw::universal::SpecificValue::infpos);
	}
	static constexpr Dfloat quiet_NaN() {
		return Dfloat(sw::universal::SpecificValue::qnan);
	}
	static constexpr Dfloat signaling_NaN() {
		return Dfloat(sw::universal::SpecificValue::snan);
	}

	static constexpr int  digits       = static_cast<int>(ndigits);
	static constexpr int  digits10     = static_cast<int>(ndigits);
	static constexpr int  max_digits10 = static_cast<int>(ndigits);
	static constexpr bool is_signed    = true;
	static constexpr bool is_integer   = false;
	static constexpr bool is_exact     = true; // decimal floating-point is exact for decimal fractions
	static constexpr int  radix        = 10;

	static constexpr int  min_exponent    = Dfloat::emin;
	static constexpr int  min_exponent10  = Dfloat::emin;
	static constexpr int  max_exponent    = Dfloat::emax;
	static constexpr int  max_exponent10  = Dfloat::emax;
	static constexpr bool has_infinity       = true;
	static constexpr bool has_quiet_NaN      = true;
	static constexpr bool has_signaling_NaN  = true;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss    = false;

	static constexpr bool is_iec559   = false;
	static constexpr bool is_bounded  = true;
	static constexpr bool is_modulo   = false;
	static constexpr bool traps       = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_to_nearest;
};

} // namespace std
