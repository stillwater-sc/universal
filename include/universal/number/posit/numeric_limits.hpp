#pragma once
// numeric_limits.hpp: definition of numeric_limits for posit types
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace std {

#if 0
#define _STCONS(ty, name, val)	static constexpr ty name = (ty)(val)

	struct _Num_base
	{	// base for all types, with common defaults
		_STCONS(float_denorm_style, has_denorm, denorm_absent);
		_STCONS(bool, has_denorm_loss, false);
		_STCONS(bool, has_infinity, false);
		_STCONS(bool, has_quiet_NaN, false);
		_STCONS(bool, has_signaling_NaN, false);
		_STCONS(bool, is_bounded, false);
		_STCONS(bool, is_exact, false);
		_STCONS(bool, is_iec559, false);
		_STCONS(bool, is_integer, false);
		_STCONS(bool, is_modulo, false);
		_STCONS(bool, is_signed, false);
		_STCONS(bool, is_specialized, false);
		_STCONS(bool, tinyness_before, false);
		_STCONS(bool, traps, false);
		_STCONS(float_round_style, round_style, round_toward_zero);
		_STCONS(int, digits, 0);
		_STCONS(int, digits10, 0);

		_STCONS(int, max_digits10, 0);

		_STCONS(int, max_exponent, 0);
		_STCONS(int, max_exponent10, 0);
		_STCONS(int, min_exponent, 0);
		_STCONS(int, min_exponent10, 0);
		_STCONS(int, radix, 0);
	};

	// STRUCT _Num_float_base
	struct _Num_posit_base
		: public _Num_base
	{	// base for floating-point types
		_STCONS(float_denorm_style, has_denorm, denorm_present);
		_STCONS(bool, has_denorm_loss, true);
		_STCONS(bool, has_infinity, true);
		_STCONS(bool, has_quiet_NaN, true);
		_STCONS(bool, has_signaling_NaN, true);
		_STCONS(bool, is_bounded, true);
		_STCONS(bool, is_exact, false);
		_STCONS(bool, is_iec559, true);
		_STCONS(bool, is_integer, false);
		_STCONS(bool, is_modulo, false);
		_STCONS(bool, is_signed, true);
		_STCONS(bool, is_specialized, true);
		_STCONS(bool, tinyness_before, true);
		_STCONS(bool, traps, false);
		_STCONS(float_round_style, round_style, round_to_nearest);
		_STCONS(int, radix, FLT_RADIX);
	};


	// CLASS numeric_limits<float>
	template<> class numeric_limits<float>
		: public _Num_float_base
	{	// limits for type float
	public:
		typedef float _Ty;

		static constexpr _Ty(min)() _THROW0()
		{	// return minimum value
			return (_FLT_MIN);
		}

		static constexpr _Ty(max)() _THROW0()
		{	// return maximum value
			return (_FLT_MAX);
		}

		static constexpr _Ty lowest() _THROW0()
		{	// return most negative value
			return (-(max)());
		}

		static constexpr _Ty epsilon() _THROW0()
		{	// return smallest effective increment from 1.0
			return (_FLT_EPSILON);
		}

		static constexpr _Ty round_error() _THROW0()
		{	// return largest rounding error
			return (0.5F);
		}

		static constexpr _Ty denorm_min() _THROW0()
		{	// return minimum denormalized value
			return (_FLT_TRUE_MIN);
		}

		static constexpr _Ty infinity() _THROW0()
		{	// return positive infinity
			return (__builtin_huge_valf());
		}

		static constexpr _Ty quiet_NaN() _THROW0()
		{	// return non-signaling NaN
			return (__builtin_nanf("0"));
		}

		static constexpr _Ty signaling_NaN() _THROW0()
		{	// return signaling NaN
			return (__builtin_nansf("1"));
		}

		_STCONS(int, digits, FLT_MANT_DIG);
		_STCONS(int, digits10, FLT_DIG);

		_STCONS(int, max_digits10, 2 + FLT_MANT_DIG * 301L / 1000);

		_STCONS(int, max_exponent, (int)FLT_MAX_EXP);
		_STCONS(int, max_exponent10, (int)FLT_MAX_10_EXP);
		_STCONS(int, min_exponent, (int)FLT_MIN_EXP);
		_STCONS(int, min_exponent10, (int)FLT_MIN_10_EXP);
	};
#endif

	template <size_t nbits, size_t es> 
	class numeric_limits< sw::universal::posit<nbits, es> > {
	public:
		using Posit = sw::universal::posit<nbits, es>;
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
		static constexpr int min_exponent10 = int((min_exponent) / 3.3);
		static constexpr int max_exponent = (nbits - 2) * (1 << es);
		static constexpr int max_exponent10 = int((max_exponent) / 3.3);
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
