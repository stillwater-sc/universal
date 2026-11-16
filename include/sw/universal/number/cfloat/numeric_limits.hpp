#pragma once
// numeric_limits.hpp: definition of numeric_limits for classic cfloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/utility/decimal_digits.hpp>   // exact digits10 / max_digits10 (#1597)
namespace std {

template <unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating> 
class numeric_limits< sw::universal::cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> > {
public:
	using Cfloat = sw::universal::cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	static constexpr bool is_specialized = true;
	static constexpr Cfloat min() { // return minimum value
		Cfloat normal{};
		normal.clear();
		normal.setexponent(Cfloat::MIN_EXP_NORMAL);
		return normal;
	} 
	static constexpr Cfloat max() { // return maximum value
		return Cfloat(sw::universal::SpecificValue::maxpos);
	} 
	static constexpr Cfloat lowest() { // return most negative value
		return Cfloat(sw::universal::SpecificValue::maxneg);
	} 
	static CONSTEXPRESSION Cfloat epsilon() { // return smallest effective increment from 1.0
		// 2^-fbits, the step above 1.0.  A layout without subnormals cannot hold it when it is below
		// the smallest normal (cfloat<12,4> without subnormals: 2^-7 against 2^-6), and the
		// subtraction flushed it to 0, an epsilon nothing can divide by.  Such a layout reports its
		// smallest positive value, min() (#2089).
		if constexpr (hasSubnormals || -static_cast<int>(Cfloat::fbits) >= Cfloat::MIN_EXP_NORMAL) {
			Cfloat one{ 1.0f }, incr{ 1.0f };
			return ++incr - one;
		}
		else {
			return min();
		}
	}
	static CONSTEXPRESSION Cfloat round_error() { // return largest rounding error
		return Cfloat(0.5f);
	}
	static constexpr Cfloat denorm_min() {  // return minimum denormalized value
		return Cfloat(sw::universal::SpecificValue::minpos);
	}
	static constexpr Cfloat infinity() { // return positive infinity, maxpos when there is none
		return Cfloat(sw::universal::SpecificValue::infpos);
	}
	static constexpr Cfloat quiet_NaN() { // return non-signaling NaN
		return Cfloat(sw::universal::SpecificValue::qnan);
	}
	static constexpr Cfloat signaling_NaN() { // return signaling NaN
		return Cfloat(sw::universal::SpecificValue::snan);
	}

	static constexpr int digits                    = nbits - 1 - es + 1;
	static constexpr int digits10                  = sw::universal::decimal_digits10(digits);
	static constexpr int max_digits10              = sw::universal::decimal_max_digits10(digits);
	static constexpr bool is_signed                = true;
	static constexpr bool is_integer               = false;
	static constexpr bool is_exact                 = false;
	static constexpr int radix                     = 2;

	// C++ specification: min_exponent is one more than the smallest negative power 
	// of the radix that is a valid normalized number
	static constexpr int min_exponent              = Cfloat::MIN_EXP_NORMAL + 1;
	static constexpr int min_exponent10            = sw::universal::decimal_min_exponent10(min_exponent);
	// C++ specification: max_exponent is one more than the largest integer power 
    // of the radix that is a valid finite floating-point number
	// maxpos is in the binade 2^MAX_EXP with max-exponent values and 2^(MAX_EXP - 1) without them:
	// MAX_EXP counts the top exponent binade either way, so it was one short for a hasMaxExpValues
	// layout (cfloat<8,4,uint8_t,true,true,false> reported 8, its max() scale is 8, #2089)
	static constexpr int max_exponent              = Cfloat(sw::universal::SpecificValue::maxpos).scale() + 1;
	static constexpr int max_exponent10            = sw::universal::decimal_max_exponent10(max_exponent);
	static constexpr bool has_infinity             = !(isSaturating && hasMaxExpValues); // that Inf pattern is maxpos
	static constexpr bool has_quiet_NaN            = true;
	static constexpr bool has_signaling_NaN        = true;
	static constexpr float_denorm_style has_denorm = (hasSubnormals ? denorm_present : denorm_absent);
	static constexpr bool has_denorm_loss          = false;

	// the IEEE-754 layouts: subnormals, inf and NaN in the all-ones exponent field (#1780), quiet and
	// signalling NaNs typed by the quiet bit (#1896), and round to nearest even.  Like
	// std::numeric_limits<float> on most hosts, this does not promise status flags or the other
	// rounding modes, which cfloat does not have (#2089).
	static constexpr bool is_iec559                = hasSubnormals && !hasMaxExpValues && !isSaturating;
	static constexpr bool is_bounded               = true;    // a finite set of values (#2089)
	static constexpr bool is_modulo                = false;
	static constexpr bool traps                    = false;
	static constexpr bool tinyness_before          = false;
	// cfloat rounds to nearest, ties to even (see cfloat_impl.hpp::round(), the classic
	// guard/round/sticky round-to-even logic), not toward zero (#1254).
	static constexpr float_round_style round_style = round_to_nearest;
};

}
