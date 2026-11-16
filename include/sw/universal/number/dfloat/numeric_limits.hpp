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

	// the smallest positive normal, 10^(emin + ndigits - 1); denorm_min() is the smallest
	// subnormal, minpos (#2105: min() was minpos too)
	static constexpr Dfloat min() {
		Dfloat v;
		return v.assign_decimal(false, 1u, Dfloat::emin + static_cast<int>(ndigits) - 1);
	}
	static constexpr Dfloat max() {
		return Dfloat(sw::universal::SpecificValue::maxpos);
	}
	static constexpr Dfloat lowest() {
		return Dfloat(sw::universal::SpecificValue::maxneg);
	}
	// the gap from 1 to the next value, 10^(1 - ndigits). It was ++1 - 1, and ++ adds 1.0, so
	// epsilon() was 1 (#2105).
	static constexpr Dfloat epsilon() {
		Dfloat v;
		return v.assign_decimal(false, 1u, 1 - static_cast<int>(ndigits));
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

	// radix 10: ndigits decimal digits survive a round trip and distinguish every value, so
	// digits10 == max_digits10 == digits, as ISO/IEC TR 24733 gives for decimal32/64/128.
	static constexpr int  digits       = static_cast<int>(ndigits);
	static constexpr int  digits10     = static_cast<int>(ndigits);
	static constexpr int  max_digits10 = static_cast<int>(ndigits);
	static constexpr bool is_signed    = true;
	static constexpr bool is_integer   = false;
	static constexpr bool is_exact     = false; // it rounds: 1/3 is not exact (#2105: was true)
	static constexpr int  radix        = 10;

	// in the C++ convention, radix^(min_exponent - 1) is the smallest normal and
	// radix^(max_exponent - 1) is finite. emin and emax are the exponents of the integer
	// significand's last digit, so the smallest normal is 10^(emin + ndigits - 1) and the
	// largest finite value is below 10^(emax + ndigits): decimal32 gives -94 / -95 / 97 / 96,
	// IEEE 754-2008 Table 3.6's emin = -95 and emax = 96 (#2105: these were emin and emax).
	static constexpr int  min_exponent    = Dfloat::emin + static_cast<int>(ndigits);
	static constexpr int  min_exponent10  = Dfloat::emin + static_cast<int>(ndigits) - 1;
	static constexpr int  max_exponent    = Dfloat::emax + static_cast<int>(ndigits);
	static constexpr int  max_exponent10  = Dfloat::emax + static_cast<int>(ndigits) - 1;
	static constexpr bool has_infinity       = true;
	static constexpr bool has_quiet_NaN      = true;
	static constexpr bool has_signaling_NaN  = true;
	static constexpr float_denorm_style has_denorm = denorm_present;   // gradual underflow below min()
	static constexpr bool has_denorm_loss    = false;

	static constexpr bool is_iec559   = false;
	static constexpr bool is_bounded  = true;
	static constexpr bool is_modulo   = false;
	static constexpr bool traps       = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_to_nearest;
};

} // namespace std
