#pragma once
// numeric_limits.hpp: definition of numeric_limits for rational arithmetic types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/decimal_digits.hpp>   // exact digit/exponent counts (#1597, #1601, #1602, #1603, #1604)

namespace std {

using namespace sw::universal;

// numeric_limits for binary rational: rational<nbits, base2, bt>
template <unsigned nbits, typename bt>
class numeric_limits< sw::universal::rational<nbits, sw::universal::base2, bt> >
{
	using RationalType = rational<nbits, sw::universal::base2, bt>;
public:
	static constexpr bool is_specialized = true;
	static constexpr RationalType min() { return RationalType().minpos(); }
	static constexpr RationalType max() { return RationalType().maxpos(); }
	static constexpr RationalType lowest() { return RationalType().maxneg(); }
	static constexpr RationalType epsilon() {
		RationalType r(0, 0);
		r.setnbit(0);
		r.setdbit(nbits - 2);
		return r;
	}
	static constexpr RationalType round_error() {
		return long(0.5);
	}
	static constexpr RationalType denorm_min() {
		return RationalType().minpos();
	}
	static constexpr RationalType infinity() {
		return INT64_MAX;
	}
	static constexpr RationalType quiet_NaN() {
		return RationalType(0, 0);
	}
	static constexpr RationalType signaling_NaN() {
		return RationalType(0, 0);
	}

	static constexpr int digits       = nbits;
	// is_exact and is_integer are both true for a rational: it is an exact ratio of
	// integers, so it takes the integer pair and max_digits10 is 0, as the native
	// integers report (#1601). The base8/base10/base16 specializations below express
	// digits in base-N digits rather than bits and are left alone.
	static constexpr int digits10     = sw::universal::decimal_digits10_integer(digits);
	static constexpr int max_digits10 = 0;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = true;
	static constexpr bool is_exact    = true;
	static constexpr int radix        = 2;

	static constexpr int min_exponent   = 0;
	static constexpr int min_exponent10 = 0;
	static constexpr int max_exponent   = 0;
	static constexpr int max_exponent10 = 0;
	static constexpr bool has_infinity  = false;
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

// numeric_limits for octal-based rationals
template<unsigned ndigits, typename bt>
class numeric_limits<sw::universal::rational<ndigits, sw::universal::base8, bt>> {
	using RationalType = rational<ndigits, sw::universal::base8, bt>;

  public:
	static constexpr bool         is_specialized = true;
	static constexpr RationalType min() { return RationalType().minpos(); }
	static constexpr RationalType max() { return RationalType().maxpos(); }
	static constexpr RationalType lowest() { return RationalType().maxneg(); }
	static constexpr RationalType epsilon() {
		blockdigit<ndigits, 8> n(SpecificValue::maxpos);
		RationalType r(1, n);
		return r;
	}
	static constexpr RationalType round_error() { return long(0.5); }
	static constexpr RationalType denorm_min() { return RationalType().minpos(); }
	static constexpr RationalType infinity() { return INT64_MAX; }
	static constexpr RationalType quiet_NaN() { return RationalType(0, 0); }
	static constexpr RationalType signaling_NaN() { return RationalType(0, 0); }

	static constexpr int  digits       = ndigits;
	static constexpr int  digits10     = ndigits;
	static constexpr int  max_digits10 = ndigits;
	static constexpr bool is_signed    = true;
	static constexpr bool is_integer   = true;
	static constexpr bool is_exact     = true;
	static constexpr int  radix        = 8;

	static constexpr int                min_exponent      = 0;
	static constexpr int                min_exponent10    = 0;
	static constexpr int                max_exponent      = 0;
	static constexpr int                max_exponent10    = 0;
	static constexpr bool               has_infinity      = false;
	static constexpr bool               has_quiet_NaN     = false;
	static constexpr bool               has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm        = denorm_absent;
	static constexpr bool               has_denorm_loss   = false;

	static constexpr bool              is_iec559       = false;
	static constexpr bool              is_bounded      = false;
	static constexpr bool              is_modulo       = false;
	static constexpr bool              traps           = false;
	static constexpr bool              tinyness_before = false;
	static constexpr float_round_style round_style     = round_toward_zero;
};

// numeric_limits for digit-based rationals
template <unsigned ndigits, typename bt>
class numeric_limits< sw::universal::rational<ndigits, sw::universal::base10, bt> >
{
	using RationalType = rational<ndigits, sw::universal::base10, bt>;
public:
	static constexpr bool is_specialized = true;
	static constexpr RationalType min() { return RationalType().minpos(); }
	static constexpr RationalType max() { return RationalType().maxpos(); }
	static constexpr RationalType lowest() { return RationalType().maxneg(); }
	static constexpr RationalType epsilon() {
		blockdigit<ndigits, 10> one(1), maxpos(SpecificValue::maxpos);
		RationalType           r(one, maxpos);
		return r;
	}
	static constexpr RationalType round_error() { return long(0.5); }
	static constexpr RationalType denorm_min() { return RationalType().minpos(); }
	static constexpr RationalType infinity() { return INT64_MAX; }
	static constexpr RationalType quiet_NaN() { return RationalType(0, 0); }
	static constexpr RationalType signaling_NaN() { return RationalType(0, 0); }

	static constexpr int digits       = ndigits;
	static constexpr int digits10     = ndigits;
	static constexpr int max_digits10 = ndigits;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = true;
	static constexpr bool is_exact    = true;
	static constexpr int radix        = 10;

	static constexpr int                min_exponent      = 0;
	static constexpr int                min_exponent10    = 0;
	static constexpr int                max_exponent      = 0;
	static constexpr int                max_exponent10    = 0;
	static constexpr bool               has_infinity      = false;
	static constexpr bool               has_quiet_NaN     = false;
	static constexpr bool               has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm        = denorm_absent;
	static constexpr bool               has_denorm_loss   = false;

	static constexpr bool              is_iec559       = false;
	static constexpr bool              is_bounded      = false;
	static constexpr bool              is_modulo       = false;
	static constexpr bool              traps           = false;
	static constexpr bool              tinyness_before = false;
	static constexpr float_round_style round_style     = round_toward_zero;
};

// numeric_limits for hexadecimal-based rationals
template<unsigned ndigits, typename bt>
class numeric_limits<sw::universal::rational<ndigits, sw::universal::base16, bt>> {
	using RationalType = rational<ndigits, sw::universal::base16, bt>;

  public:
	static constexpr bool         is_specialized = true;
	static constexpr RationalType min() { return RationalType().minpos(); }
	static constexpr RationalType max() { return RationalType().maxpos(); }
	static constexpr RationalType lowest() { return RationalType().maxneg(); }
	static constexpr RationalType epsilon() {
		blockdigit<ndigits, 16> n(SpecificValue::maxpos);
		RationalType           r(1, n);
		return r;
	}
	static constexpr RationalType round_error() { return long(0.5); }
	static constexpr RationalType denorm_min() { return RationalType().minpos(); }
	static constexpr RationalType infinity() { return INT64_MAX; }
	static constexpr RationalType quiet_NaN() { return RationalType(0, 0); }
	static constexpr RationalType signaling_NaN() { return RationalType(0, 0); }

	static constexpr int  digits       = ndigits;
	static constexpr int  digits10     = ndigits;
	static constexpr int  max_digits10 = ndigits;
	static constexpr bool is_signed    = true;
	static constexpr bool is_integer   = true;
	static constexpr bool is_exact     = true;
	static constexpr int  radix        = 16;

	static constexpr int                min_exponent      = 0;
	static constexpr int                min_exponent10    = 0;
	static constexpr int                max_exponent      = 0;
	static constexpr int                max_exponent10    = 0;
	static constexpr bool               has_infinity      = false;
	static constexpr bool               has_quiet_NaN     = false;
	static constexpr bool               has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm        = denorm_absent;
	static constexpr bool               has_denorm_loss   = false;

	static constexpr bool              is_iec559       = false;
	static constexpr bool              is_bounded      = false;
	static constexpr bool              is_modulo       = false;
	static constexpr bool              traps           = false;
	static constexpr bool              tinyness_before = false;
	static constexpr float_round_style round_style     = round_toward_zero;
};

}
