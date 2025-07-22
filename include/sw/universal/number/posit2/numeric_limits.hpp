#pragma once
// numeric_limits.hpp: definition of numeric_limits for generalized posit types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace std {

	template <unsigned nbits, unsigned es, typename bt>
	class numeric_limits< sw::universal::posit<nbits, es, bt> > {
	public:
		using Posit = sw::universal::posit<nbits, es, bt>;
		static constexpr bool is_specialized = true;
		static constexpr Posit min() { // return minimum value
			return Posit(sw::universal::SpecificValue::minpos);
		} 
		static constexpr Posit max() { // return maximum value
			return Posit(sw::universal::SpecificValue::maxpos);
		} 
		static constexpr Posit lowest() { // return most negative value
			return Posit(sw::universal::SpecificValue::maxneg);
		} 
		static constexpr Posit epsilon() { // return smallest effective increment from 1.0
			Posit one{ 1 }, incr{ 1 };
			return ++incr - one;
		}
		static constexpr Posit round_error() { // return largest rounding error
			return Posit(0.5);
		}
		static constexpr Posit denorm_min() {  // return minimum denormalized value
			return Posit(sw::universal::SpecificValue::minpos);
		}
		static constexpr Posit infinity() { // return positive infinity
			return Posit(sw::universal::SpecificValue::maxpos);
		}
		static constexpr Posit quiet_NaN() { // return non-signaling NaN
			return Posit(NAR);
		}
		static constexpr Posit signaling_NaN() { // return signaling NaN
			return Posit(NAR);
		}

		static constexpr int digits = ((es + 2) > nbits) ? 0 : (int(nbits) - 3 - int(es) + 1);
		static constexpr int digits10 = int((digits) / 3.3);
		static constexpr int max_digits10 = int((digits) / 3.3) + 1;
		static constexpr bool is_signed = true;
		static constexpr bool is_integer = false;
		static constexpr bool is_exact = false;
		static constexpr int radix = 2;

		static constexpr int min_exponent = static_cast<int>(2 - int(nbits)) * (1 << es);
		static constexpr int min_exponent10 = static_cast<int>(min_exponent / 3.3f);
		static constexpr int max_exponent = (nbits - 2) * (1 << es);
		static constexpr int max_exponent10 = static_cast<int>(max_exponent / 3.3f);
		static constexpr bool has_infinity = true;
		static constexpr bool has_quiet_NaN = true;
		static constexpr bool has_signaling_NaN = true;
		static constexpr float_denorm_style has_denorm = denorm_absent;
		static constexpr bool has_denorm_loss = false;

		static constexpr bool is_iec559 = false;
		static constexpr bool is_bounded = false;
		static constexpr bool is_modulo = false;
		static constexpr bool traps = false;
		static constexpr bool tinyness_before = false;
		static constexpr float_round_style round_style = round_to_nearest;
	};

}
