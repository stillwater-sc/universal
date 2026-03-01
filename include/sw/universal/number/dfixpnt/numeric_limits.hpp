#pragma once
// numeric_limits.hpp: definition of numeric_limits for decimal fixed-point dfixpnt types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace std {

template<unsigned ndigits, unsigned radix, sw::universal::DecimalEncoding encoding, bool arithmetic, typename bt>
class numeric_limits< sw::universal::dfixpnt<ndigits, radix, encoding, arithmetic, bt> > {
public:
	using Dfixpnt = sw::universal::dfixpnt<ndigits, radix, encoding, arithmetic, bt>;

	static constexpr bool is_specialized = true;

	static constexpr Dfixpnt min() {
		return Dfixpnt(sw::universal::SpecificValue::minpos);
	}
	static constexpr Dfixpnt max() {
		return Dfixpnt(sw::universal::SpecificValue::maxpos);
	}
	static constexpr Dfixpnt lowest() {
		return Dfixpnt(sw::universal::SpecificValue::maxneg);
	}
	static constexpr Dfixpnt epsilon() {
		Dfixpnt eps;
		eps.setdigit(0, 1);
		return eps;
	}
	static constexpr Dfixpnt round_error() {
		return epsilon();
	}
	static constexpr Dfixpnt denorm_min() {
		return epsilon();
	}
	static constexpr Dfixpnt infinity() {
		return max();
	}
	static constexpr Dfixpnt quiet_NaN() {
		return Dfixpnt(0);
	}
	static constexpr Dfixpnt signaling_NaN() {
		return Dfixpnt(0);
	}

	static constexpr int  digits       = static_cast<int>(ndigits);
	static constexpr int  digits10     = static_cast<int>(ndigits);
	static constexpr int  max_digits10 = static_cast<int>(ndigits);
	static constexpr bool is_signed    = true;
	static constexpr bool is_integer   = (radix == 0);
	static constexpr bool is_exact     = true;
	static constexpr int  radix_value  = 10;  // using radix_value to avoid shadowing template param

	static constexpr int  min_exponent   = -static_cast<int>(radix);
	static constexpr int  min_exponent10 = -static_cast<int>(radix);
	static constexpr int  max_exponent   = static_cast<int>(ndigits - radix - 1);
	static constexpr int  max_exponent10 = static_cast<int>(ndigits - radix - 1);
	static constexpr bool has_infinity       = false;
	static constexpr bool has_quiet_NaN      = false;
	static constexpr bool has_signaling_NaN  = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss    = false;

	static constexpr bool is_iec559  = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo  = arithmetic;
	static constexpr bool traps      = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_toward_zero;
};

} // namespace std
