#pragma once
// numeric_limits.hpp: definition of numeric_limits for hfloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>
#include <universal/utility/decimal_digits.hpp>   // exact min_exponent10 / max_exponent10 (#2133)

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
	// the gap from 1 = 0.1 * 16^1 to the next value: 16^(1 - ndigits). It was ++1 - 1, and ++
	// adds 1.0, so epsilon() was 1.0 (#2105).
	static constexpr Hfloat epsilon() {
		Hfloat e;
		e.assign_from_mantissa64(false, 4 - 4 * static_cast<int>(ndigits) + 1, uint64_t(1) << 63);
		return e;
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

	// digits counts radix digits, so for radix 16 it is the number of hex digits (#2105: it was
	// ndigits * 4, the bit count). digits10 is floor((ndigits - 1) * log10(16)), the decimal
	// digits that survive a round trip at the worst wobble; max_digits10 is the decimal digits
	// that distinguish every value, from the 4 * ndigits bits a full leading digit carries.
	static constexpr int  digits       = static_cast<int>(ndigits);
	static constexpr int  digits10     = static_cast<int>((ndigits - 1) * 1.2041199826559248);
	static constexpr int  max_digits10 = static_cast<int>(ndigits * 4 * 0.30102999566398120) + 2;
	static constexpr bool is_signed    = true;
	static constexpr bool is_integer   = false;
	static constexpr bool is_exact     = false;
	static constexpr int  radix        = 16;

	static constexpr int  min_exponent    = Hfloat::emin;
	// the decimal exponents through the exact helpers (#2133): min() = 16^(emin - 1) = 2^(4 emin - 4),
	// a binary min_exponent of 4 emin - 3, and max() < 16^emax = 2^(4 emax). int(emin * 1.204) truncated
	// toward zero: hfp32 reported -77 for ceil(log10(min())) = -78, hfloat<2,3> -4 for -6. min() is an exact
	// power of two, so the helper's result is exact.
	static constexpr int  min_exponent10  = sw::universal::decimal_min_exponent10(4LL * Hfloat::emin - 3);
	static constexpr int  max_exponent    = Hfloat::emax;
	// max() = (1 - 16^-ndigits) 16^emax, just below 2^(4 emax): floor(log10(2^(4 emax))) is one too high
	// when 10^k lands between them, as for hfloat<1,17> (78912; log10(max()) is 78911.975). k drops by one
	// when d = 4 emax - k log2(10), the margin of 10^k below 2^(4 emax) in bits, is under
	// t = -log2(1 - 16^-ndigits). Checked against exact integer arithmetic over es 1..17 and ndigits
	// 1..29 and 56 (#2133 review).
	static constexpr int  max_exponent10  = []() {
		// emax == 0 (es == 1): max() = 1 - 16^-ndigits < 1, so -1. The margin test below also gives -1
		// there, until 16^-ndigits underflows a double (ndigits >= 269) and t reads 0
		if constexpr (Hfloat::emax == 0) return -1;
		int k = sw::universal::decimal_max_exponent10(4LL * Hfloat::emax);
		double x = 1.0;
		for (unsigned i = 0; i < Hfloat::ndigits; ++i) x /= 16.0;
		const double t = (x / 0.69314718055994530942) * (1.0 + x / 2.0 + x * x / 3.0);   // -log2(1 - x)
		const double d = 4.0 * static_cast<double>(Hfloat::emax) - static_cast<double>(k) * 3.32192809488736234787;
		return (d < t) ? k - 1 : k;
	}();
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
