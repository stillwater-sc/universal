#pragma once
// extract_fields.hpp: configure constexpr/nonconst manipulation functions for IEEE-754 native types using C++20 <bit>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

#if BIT_CAST_SUPPORT
#include <bit>    // C++20 bit_cast

	template<typename Real>
	inline constexpr void extractFields(Real value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		if (value == 0) {
			s = false;
			rawExponentBits = 0ull;
			rawFractionBits = 0ull;
		}
		if (value < 0) s = true;
	}

	// specialization to extract fields from a float
	template<>
	inline constexpr void extractFields(float value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		uint32_t bc = std::bit_cast<uint32_t, float>(value);
		s = (ieee754_parameter<float>::smask & bc);
		rawExponentBits = (ieee754_parameter<float>::emask & bc) >> ieee754_parameter<float>::fbits;
		rawFractionBits = (ieee754_parameter<float>::fmask & bc);
		bits = bc;
	}

	// specialization to extract fields from a double
	template<>
	inline constexpr void extractFields(double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		uint64_t bc = std::bit_cast<uint64_t, double>(value);
		s = (ieee754_parameter<double>::smask & bc);
		rawExponentBits = (ieee754_parameter<double>::emask & bc) >> ieee754_parameter<double>::fbits;
		rawFractionBits = (ieee754_parameter<double>::fmask & bc);
		bits = bc;
	}

#if LONG_DOUBLE_SUPPORT

//#pragma message("LONG_DOUBLE_SUPPORT is configured in extract_fields")

	// specialization to extract fields from a long double
	template<>
	inline constexpr void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		uint64_t bc = std::bit_cast<uint64_t, long double>(value);
		s = (ieee754_parameter<long double>::smask & bc);
		rawExponentBits = (ieee754_parameter<long double>::emask & bc) >> ieee754_parameter<long double>::fbits;
		rawFractionBits = (ieee754_parameter<long double>::fmask & bc);
	}
#else

//#pragma message("LONG_DOUBLE_SUPPORT is not configured in extract_fields")

#define LONG_DOUBLE_DOWNCAST
#ifdef LONG_DOUBLE_DOWNCAST
	template<>
	inline constexpr void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		double d = static_cast<double>(value);
		uint64_t bc = std::bit_cast<uint64_t, double>(d);
		s = (ieee754_parameter<double>::smask & bc);
		rawExponentBits = (ieee754_parameter<double>::emask & bc) >> ieee754_parameter<double>::fbits;
		rawFractionBits = (ieee754_parameter<double>::fmask & bc);
		bits = bc;
	}
#endif // LONG_DOUBLE_DOWNCAST
#endif // LONG_DOUBLE_SUPPORT

#else // BIT_CAST_SUPPORT
 
////////////////////////////////////////////////////////////////////////
// nonconst extractFields for single precision floating-point

	inline void extractFields(float value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		float_decoder decoder;
		decoder.f = value;
		s = decoder.parts.sign ? true : false;
		rawExponentBits = static_cast<uint64_t>(decoder.parts.exponent);
		rawFractionBits = static_cast<uint64_t>(decoder.parts.fraction);
		bits = uint64_t(decoder.bits);
	}

////////////////////////////////////////////////////////////////////////
// nonconst extractFields for double precision floating-point

	inline void extractFields(double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		double_decoder decoder;
		decoder.d = value;
		s = decoder.parts.sign ? true : false;
		rawExponentBits = decoder.parts.exponent;
		rawFractionBits = decoder.parts.fraction;
		bits = uint64_t(decoder.bits);
	}

#if LONG_DOUBLE_SUPPORT

//#pragma message("LONG_DOUBLE_SUPPORT is configured in extract_fields")

	// specialization to extract fields from a long double
	inline void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		long_double_decoder decoder;
		decoder.ld = value;
		s = decoder.parts.sign ? true : false;
		rawExponentBits = decoder.parts.exponent;
		rawFractionBits = decoder.parts.fraction;
		bits = decoder.bits[0];  // communicate the lower order bits which represent the fraction bits
	}
#else

//#pragma message("LONG_DOUBLE_SUPPORT is not configured in extract_fields")

#define LONG_DOUBLE_DOWNCAST
#ifdef LONG_DOUBLE_DOWNCAST
	inline void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		extractFields(double(value), s, rawExponentBits, rawFractionBits, bits);
	}
#endif // LONG_DOUBLE_DOWNCAST
#endif // LONG_DOUBLE_SUPPORT


#endif   // BIT_CAST_SUPPORT

	inline void setFields(float& value, bool s, uint64_t rawExponentBits, uint64_t rawFractionBits) noexcept {
		float_decoder decoder;
		decoder.parts.sign = s;
		decoder.parts.exponent = rawExponentBits & 0xFF;
		decoder.parts.fraction = rawFractionBits & 0x7FFFFF;
		value = decoder.f;
	}

	inline void setFields(double& value, bool s, uint64_t rawExponentBits, uint64_t rawFractionBits) noexcept {
		double_decoder decoder;
		decoder.parts.sign = s;
		decoder.parts.exponent = rawExponentBits & 0x7FF;
		decoder.parts.fraction = rawFractionBits & 0xF'FFFF'FFFF'FFFF;
		value = decoder.d;
	}

}} // namespace sw::universal
