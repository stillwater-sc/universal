#pragma once
// manipulators.hpp: definition of manipulation functions for native types
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <universal/utility/find_msb.hpp>
#include <universal/native/ieee754_parameter.hpp>
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	template<typename IntegralType, 
		std::enable_if_t< ::std::is_integral<IntegralType>::value, bool> = true
	>
	std::string type_tag(IntegralType = {}) {
		// can't use a simple typeid(Real).name() because gcc and clang obfuscate the native types
		constexpr unsigned nbits = sizeof(IntegralType) * 8;
		std::string type_string;
		if constexpr (nbits == 8) {
			type_string = ::std::is_signed<IntegralType>::value ? std::string("int8_t") : std::string("uint8_t");
		}
		else if constexpr (nbits == 16) {
			type_string = ::std::is_signed<IntegralType>::value ? std::string("int16_t") : std::string("uint16_t");
		}
		else if constexpr (nbits == 32) {
			type_string = ::std::is_signed<IntegralType>::value ? std::string("int32_t") : std::string("uint32_t");
		}
		else if constexpr (nbits == 64) {
			type_string = ::std::is_signed<IntegralType>::value ? std::string("int64_t") : std::string("uint64_t");
		}
		else {
			type_string = std::string("unknown");
		}
		return type_string;
	}

	template<typename RealType,
		std::enable_if_t< ::std::is_floating_point<RealType>::value, bool> = true
	>
	std::string type_tag(RealType = {}) {
		// can't use a simple typeid(Real).name() because gcc and clang obfuscate the native types
		constexpr unsigned nbits = sizeof(RealType) * 8;
		std::string type_string;
		if constexpr (nbits == 32) {
			type_string = std::string("float");
		}
		else if constexpr (nbits == 64) {
			type_string = std::string("double");
		}
		else if constexpr (nbits == 128) {
			type_string = std::string("long double");
		}
		else {
			type_string = std::string("unknown");
		}
		return type_string;
	}

	template<typename RealType,
		std::enable_if_t< ::std::is_floating_point<RealType>::value, bool> = true
	>
	std::string type_field(RealType = {}) {
		std::string fields{};
		if constexpr (sizeof(RealType) == 4) {
			fields = "fields(s:1|e:8|f:23)";
		}
		else if constexpr (sizeof(RealType) == 8) {
			fields = "fields(s:1|e:11|f:52)";
		}
		if constexpr (sizeof(RealType) == 16) {
			fields = "fields(s:1|e:15|f:112)";
		}
		return fields;
	}

	// internal function to extract exponent
	template<typename Uint, typename Real>
	int _extractExponent(Real v) {
		static_assert(sizeof(Real) == sizeof(Uint), "mismatched sizes");
		Uint raw{ BitCast<Uint>(v) };
		raw &= static_cast<Uint>(~ieee754_parameter<Real>::smask);
		Uint frac{ raw };
		raw >>= ieee754_parameter<Real>::fbits;
		// de-bias
		int e = static_cast<int>(raw) - static_cast<int>(ieee754_parameter<Real>::bias);
		if (raw == 0) { // a subnormal encoding
			int msb = static_cast<int>(find_msb(frac));
			e -= (static_cast<int>(ieee754_parameter<Real>::fbits) - msb);
		}
		return e;
	}

	template<typename Real,
		typename = typename ::std::enable_if< ::std::is_floating_point<Real>::value, Real >::type
	>
	int scale(Real v) {
		int _e{ 0 };
		if constexpr (sizeof(Real) == 2) { // half precision floating-point
			_e = _extractExponent<std::uint16_t>(v);
		}
		if constexpr (sizeof(Real) == 4) { // single precision floating-point
			_e = _extractExponent<std::uint32_t>(v);
		}
		else if constexpr (sizeof(Real) == 8) { // double precision floating-point
			_e = _extractExponent<std::uint64_t>(v);
		}
		else if constexpr (sizeof(Real) == 16) { // long double precision floating-point
			//long double frac = frexpl(v, &_e);
			frexpl(v, &_e);
			_e -= 1;
		}
		return _e;
	}

	// internal function to extract significant
	template<typename Uint, typename Real>
	Uint _extractSignificant(Real v) {
		static_assert(sizeof(Real) == sizeof(Uint), "mismatched sizes");
		Uint raw{ BitCast<Uint>(v) };
		raw &= ieee754_parameter<Real>::fmask;
		raw |= ieee754_parameter<Real>::hmask; // add the hidden bit
		return raw;
	}

	template<typename Real,
		typename = typename ::std::enable_if< ::std::is_floating_point<Real>::value, Real>::type
	>
	unsigned long long significant(Real v) {
		std::uint64_t _f{ 0 };
		if constexpr (sizeof(Real) == 2) { // half precision floating-point
			_f = _extractSignificant<std::uint16_t>(v);
		}
		if constexpr (sizeof(Real) == 4) { // single precision floating-point
			_f = _extractSignificant<std::uint32_t>(v);
		}
		else if constexpr (sizeof(Real) == 8) { // double precision floating-point
			_f = _extractSignificant<std::uint64_t>(v);
		}
		else if constexpr (sizeof(Real) == 16) { // long double precision floating-point
			_f = 0;
		}
		return _f;
	}

	// print representations of an IEEE-754 floating-point
	template<typename Real>
	void valueRepresentations(Real value) {
		using namespace sw::universal;
		std::cout << "IEEE-754 type : " << type_tag<Real>() << '\n';
		std::cout << "hex    : " << to_hex(value) << '\n';
		std::cout << "binary : " << to_binary(value) << '\n';
		std::cout << "triple : " << to_triple(value) << '\n';
		std::cout << "base2  : " << to_base2_scientific(value) << '\n';
		std::cout << "base10 : " << value << '\n';
		std::cout << "color  : " << color_print(value) << '\n';
	}

	// generate a binary string for a native IEEE floating point
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
	inline std::string to_binary(Real number, bool bNibbleMarker = false) {
		std::stringstream s;

		bool sign{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(number, sign, rawExponent, rawFraction, bits);

		s << "0b";
		// print sign bit
		s << (sign ? '1' : '0') << '.';

		// print exponent bits
		{
			uint32_t mask = (uint32_t(1) << (ieee754_parameter<Real>::ebits - 1));
			for (int i = ieee754_parameter<Real>::ebits - 1; i >= 0; --i) {
				s << ((rawExponent & mask) ? '1' : '0');
				if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
				mask >>= 1;
			}
		}

		s << '.';

		// print fraction bits
		uint64_t mask = (uint64_t(1) << (ieee754_parameter<Real>::fbits - 1));
		for (int i = ieee754_parameter<Real>::fbits - 1; i >= 0; --i) {
			s << ((rawFraction & mask) ? '1' : '0');
			if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
			mask >>= 1;
		}

			return s.str();
	}

	// return in triple form (sign, scale, fraction)
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
	inline std::string to_triple(Real number, bool bNibbleMarker = false) {
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
	inline std::string to_base2_scientific(Real number) {
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

#ifdef DEPRECATED
	// DEPRECATED: we have standardized on raw bit hex, not field hex format
	// generate a hex formatted string for a native IEEE floating point
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
	inline std::string to_hex(Real number) {
		std::stringstream s;
		bool sign{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(number, sign, rawExponent, rawFraction, bits);
		s << (sign ? '1' : '0') << '.' << std::hex << int(rawExponent) << '.' << rawFraction;
		return s.str();
	}
#endif // DEPRECATED


	template<typename RealType,
		std::enable_if_t< ::std::is_floating_point<RealType>::value, bool> = true
	>	
	std::string pretty_print(const RealType f) {
		return std::string("TBD");
	}

	template<typename RealType,
		std::enable_if_t< ::std::is_floating_point<RealType>::value, bool> = true
	>	
	std::string info_print(const RealType f, int printPrecision = 17) {
		return std::string("TBD");
	}


	// generate a color coded binary string for a native single/double/long double IEEE floating point
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
	>
	inline std::string color_print(Real number) {
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
				//			if (i > 0 && i % 4 == 0) s << cyan << '\'';
				mask >>= 1;
			}
		}

		//	s << '.';

			// print fraction bits
		uint64_t mask = (uint64_t(1) << (ieee754_parameter<Real>::fbits - 1));
		for (int i = (ieee754_parameter<Real>::fbits - 1); i >= 0; --i) {
			s << magenta << ((rawFraction & mask) ? '1' : '0');
			//		if (i > 0 && i % 4 == 0) s << magenta << '\'';
			mask >>= 1;
		}

		s << def;
		return s.str();
	}

}} // namespace sw::universal
