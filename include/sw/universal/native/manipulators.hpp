#pragma once
// manipulators.hpp: definition of manipulation functions for native types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The text half. manipulators_core.hpp is the field-extraction half (#1334); it is
// included here so this header's surface is unchanged for every existing consumer.
#include <iomanip>
#include <string>
#include <sstream>
#include <cstdint>       // uint64_t
#include <limits>        // std::numeric_limits
#include <type_traits>   // std::enable_if / std::is_floating_point
#include <universal/native/manipulators_core.hpp>
#include <universal/native/ieee754_type_tag.hpp>
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// return in triple form (sign, scale, fraction)
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
    inline std::string to_triple(Real number, bool bNibbleMarker = false) noexcept {
		std::stringstream s;

		bool sign{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(number, sign, rawExponent, rawFraction, bits);

		// print sign bit
		s << '(' << (sign ? '-' : '+') << ',';

		// exponent 
		// the exponent value used in the arithmetic is the exponent shifted by a bias 
		// for the IEEE 754 binary32 case, an exponent value of 127 represents the actual zero 
		// (i.e. for 2^(e - 127) to be one, e must be 127). 
		// Exponents range from -126 to +127 because exponents of -127 (all 0s) and 128 (all 1s) are reserved for special numbers.
		if (rawExponent == 0) {
			s << "denorm, ";
		}
		else if (rawExponent == ieee754_parameter<Real>::eallset) {
			s << "super, ";
		}
		else {
			int scale = static_cast<int>(rawExponent) - ieee754_parameter<Real>::bias;
			s << std::setw(4) << scale << ", ";
		}

		// print fraction bits
		uint64_t mask = (uint64_t(1) << (ieee754_parameter<Real>::fbits - 1));
		s << "0b";
		for (int i = (ieee754_parameter<Real>::fbits - 1); i >= 0; --i) {
			s << ((rawFraction & mask) ? '1' : '0');
			if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
			mask >>= 1;
		}

		s << ')';
		return s.str();
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
    inline std::string to_base2_scientific(Real number) noexcept {
		std::stringstream s;

		bool sign{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(number, sign, rawExponent, rawFraction, bits);

		s << (sign == 1 ? "-" : "+") << "1.";
		uint64_t mask = (uint64_t(1) << (ieee754_parameter<Real>::fbits - 1));
		for (int i = (ieee754_parameter<Real>::fbits - 1); i >= 0; --i) {
			s << ((rawFraction & mask) ? '1' : '0');
			mask >>= 1;
		}
		s << "e2^" << std::showpos << (rawExponent - ieee754_parameter<Real>::bias);

		return s.str();
	}

	// generate a hex formatted string for a native IEEE floating point
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
    inline std::string to_hex(Real number) noexcept {
		std::stringstream s;
		s << std::hexfloat << number;
		return s.str();
	}

	template<typename RealType,
		std::enable_if_t< ::std::is_floating_point<RealType>::value, bool> = true
	>	
	std::string pretty_print(const RealType f) noexcept {
		return std::string("TBD");
	}

	template<typename RealType,
		std::enable_if_t< ::std::is_floating_point<RealType>::value, bool> = true
	>	
	std::string info_print(const RealType f, int printPrecision = 17) noexcept {
		return std::string("TBD");
	}


	// generate a string representing the IEEE-754 components: sign, scale, significand
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
	inline std::string components(Real number) noexcept {
		std::stringstream s;

		bool _sign{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(number, _sign, rawExponent, rawFraction, bits);

		s << "sign: " << (_sign ? '-' : '+');

		if (rawExponent == ieee754_parameter<Real>::eallset) {
			// inf or nan
			if (rawFraction == 0) {
				s << ", inf";
			} else {
				s << ", nan";
			}
		}
		else if (rawExponent == 0) {
			// zero or subnormal
			if (rawFraction == 0) {
				s << ", zero";
			} else {
				int scale = 1 - static_cast<int>(ieee754_parameter<Real>::bias);
				Real frac = Real(rawFraction) / Real(uint64_t(1) << ieee754_parameter<Real>::fbits);
				s << ", scale: " << scale
				  << ", significand: " << std::setprecision(std::numeric_limits<Real>::max_digits10) << frac
				  << " (subnormal)";
			}
		}
		else {
			// normal
			int scale = static_cast<int>(rawExponent) - static_cast<int>(ieee754_parameter<Real>::bias);
			Real frac = Real(1.0) + Real(rawFraction) / Real(uint64_t(1) << ieee754_parameter<Real>::fbits);
			s << ", scale: " << scale
			  << ", significand: " << std::setprecision(std::numeric_limits<Real>::max_digits10) << frac;
		}

		return s.str();
	}

	// generate a color coded binary string for a native single/double/long double IEEE floating point
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
    inline std::string color_print(Real number, bool nibbleMarker = false) noexcept {
		std::stringstream s;

		bool sign{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits;
		extractFields(number, sign, rawExponent, rawFraction, bits);

		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color blue(ColorCode::FG_BLUE);
		Color magenta(ColorCode::FG_MAGENTA);
		Color cyan(ColorCode::FG_CYAN);
		Color white(ColorCode::FG_WHITE);
		Color def(ColorCode::FG_DEFAULT);

		// print sign bit
		s << red << (sign ? '1' : '0'); // << '.';

		// print exponent bits
		{
			uint64_t mask = (1 << (ieee754_parameter<Real>::ebits - 1));
			for (int i = (ieee754_parameter<Real>::ebits - 1); i >= 0; --i) {
				s << cyan << ((rawExponent & mask) ? '1' : '0');
				if (nibbleMarker && i > 0 && i % 4 == 0) s << cyan << '\'';
				mask >>= 1;
			}
		}

		//	s << '.';

			// print fraction bits
		uint64_t mask = (uint64_t(1) << (ieee754_parameter<Real>::fbits - 1));
		for (int i = (ieee754_parameter<Real>::fbits - 1); i >= 0; --i) {
			s << magenta << ((rawFraction & mask) ? '1' : '0');
			if (nibbleMarker && i > 0 && i % 4 == 0) s << magenta << '\'';
			mask >>= 1;
		}

		s << def;
		return s.str();
	}

}} // namespace sw::universal
