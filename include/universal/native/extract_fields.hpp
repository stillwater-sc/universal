#pragma once
// extract_fields.hpp: configure constexpr/nonconst manipulation functions for IEEE-754 native types using C++20 <bit>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>

namespace sw { namespace universal {

#if BIT_CAST_IS_CONSTEXPR
#include <bit>    // C++20 bit_cast

	template<typename Real>
	inline BIT_CAST_CONSTEXPR void extractFields(Real value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		if (value == 0) {
			s = false;
			rawExponentBits = 0ull;
			rawFractionBits = 0ull;
		}
		if (value < 0) s = true;
	}

	// specialization to extract fields from a float
	template<>
	inline BIT_CAST_CONSTEXPR void extractFields(float value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		uint32_t bc = std::bit_cast<uint32_t, float>(value);
		s = (ieee754_parameter<float>::smask & bc);
		rawExponentBits = (ieee754_parameter<float>::emask & bc) >> ieee754_parameter<float>::fbits;
		rawFractionBits = (ieee754_parameter<float>::fmask & bc);
		bits = bc;
	}

	// specialization to extract fields from a double
	template<>
	inline BIT_CAST_CONSTEXPR void extractFields(double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		uint64_t bc = std::bit_cast<uint64_t, double>(value);
		s = (ieee754_parameter<double>::smask & bc);
		rawExponentBits = (ieee754_parameter<double>::emask & bc) >> ieee754_parameter<double>::fbits;
		rawFractionBits = (ieee754_parameter<double>::fmask & bc);
		bits = bc;
	}

#if LONG_DOUBLE_SUPPORT

// Clang bit_cast<> can't deal with long double

#if defined(LONG_DOUBLE_DOWNCAST)
	template<>
	inline BIT_CAST_CONSTEXPR void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		double d = static_cast<double>(value);
		uint64_t bc = std::bit_cast<uint64_t, double>(d);
		s = (ieee754_parameter<double>::smask & bc);
		rawExponentBits = (ieee754_parameter<double>::emask & bc) >> ieee754_parameter<double>::fbits;
		rawFractionBits = (ieee754_parameter<double>::fmask & bc);
		bits = bc;
	}
#else // !DOWNCAST
/*
	ETLO 8/1/2024: not able to make std::bit_cast<> work for long double
	// specialization to extract fields from a long double
	template<>
	inline BIT_CAST_CONSTEXPR void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		struct blob {
			std::uint64_t hi;
			std::uint64_t fraction;
		} raw;
		raw = std::bit_cast<blob, long double>(value);
		s = (ieee754_parameter<long double>::smask & raw.hi);
		rawExponentBits = (ieee754_parameter<long double>::emask & raw.hi);
		rawFractionBits = (ieee754_parameter<long double>::fmask & raw.fraction);
	}
	*/
	// falling back to non-constexpr
	// specialization to extract fields from a long double
	inline void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		long_double_decoder decoder;
		decoder.ld = value;
		s = decoder.parts.sign ? true : false;
		rawExponentBits = decoder.parts.exponent;
		rawFractionBits = decoder.parts.fraction;
		bits = decoder.bits[0];  // communicate the lower order bits which represent the fraction bits
	}

#endif // LONG_DOUBLE_DOWNCAST
#endif // LONG_DOUBLE_SUPPORT

#else // !BIT_CAST_IS_CONSTEXPR

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
// Clang bit_cast<> can't deal with long double
#define LONG_DOUBLE_DOWNCAST
#ifdef LONG_DOUBLE_DOWNCAST
	inline void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		extractFields(double(value), s, rawExponentBits, rawFractionBits, bits);
	}
#else
	// specialization to extract fields from a long double
	inline void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits, uint64_t& bits) noexcept {
		long_double_decoder decoder;
		decoder.ld = value;
		s = decoder.parts.sign ? true : false;
		rawExponentBits = decoder.parts.exponent;
		rawFractionBits = decoder.parts.fraction;
		bits = decoder.bits[0];  // communicate the lower order bits which represent the fraction bits
	}
#endif // LONG_DOUBLE_DOWNCAST
#endif // LONG_DOUBLE_SUPPORT
#endif // BIT_CAST_IS_CONSTEXPR

template<typename Real>
	inline BIT_CAST_CONSTEXPR bool checkNaN(Real value, int& nan_type) {
		nan_type = NAN_TYPE_NEITHER;
		return false;
	}

	template<>
	inline BIT_CAST_CONSTEXPR bool checkNaN(float value, int& nan_type) {
		bool bIsNaN{ false };
		bool s{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(value, s, rawExponent, rawFraction, bits);
		if (rawExponent == ieee754_parameter<float>::eallset) { // nan and inf need to be remapped
			if (rawFraction == (ieee754_parameter<float>::fmask & ieee754_parameter<float>::snanmask) ||
				rawFraction == (ieee754_parameter<float>::fmask & (ieee754_parameter<float>::qnanmask | ieee754_parameter<float>::snanmask))) {
				// 1.11111111.00000000.......00000001 signalling nan
				// 0.11111111.00000000000000000000001 signalling nan
				// MSVC
				// 1.11111111.10000000.......00000001 signalling nan
				// 0.11111111.10000000.......00000001 signalling nan
				nan_type = NAN_TYPE_SIGNALLING;
				bIsNaN = true;
			}
			else if (rawFraction == (ieee754_parameter<float>::fmask & ieee754_parameter<float>::qnanmask)) {
				// 1.11111111.10000000.......00000000 quiet nan
				// 0.11111111.10000000.......00000000 quiet nan
				nan_type = NAN_TYPE_QUIET;
				bIsNaN = true;
			}
			else {
				nan_type = NAN_TYPE_NEITHER;
				bIsNaN = false;
			}
		}
		return bIsNaN;
	}

	template<>
	inline BIT_CAST_CONSTEXPR bool checkNaN(double value, int& nan_type) {
		bool bIsNaN{ false };
		bool s{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(value, s, rawExponent, rawFraction, bits);
		if (rawExponent == ieee754_parameter<double>::eallset) { // nan and inf need to be remapped
			if (rawFraction == (ieee754_parameter<double>::fmask & ieee754_parameter<double>::snanmask) ||
				rawFraction == (ieee754_parameter<double>::fmask & (ieee754_parameter<double>::qnanmask | ieee754_parameter<double>::snanmask))) {
				// 1.11111111.00000000.......00000001 signalling nan
				// 0.11111111.00000000000000000000001 signalling nan
				// MSVC
				// 1.11111111.10000000.......00000001 signalling nan
				// 0.11111111.10000000.......00000001 signalling nan
				nan_type = NAN_TYPE_SIGNALLING;
				bIsNaN = true;
			}
			else if (rawFraction == (ieee754_parameter<double>::fmask & ieee754_parameter<double>::qnanmask)) {
				// 1.11111111.10000000.......00000000 quiet nan
				// 0.11111111.10000000.......00000000 quiet nan
				nan_type = NAN_TYPE_QUIET;
				bIsNaN = true;
			}
			else {
				nan_type = NAN_TYPE_NEITHER;
				bIsNaN = false;
			}
		}
		return bIsNaN;
	}

	template<typename Real>
	inline BIT_CAST_CONSTEXPR bool checkInf(Real value, int& inf_type) {
		inf_type = INF_TYPE_NEITHER;
		return false;
	}

	template<>
	inline BIT_CAST_CONSTEXPR bool checkInf(float value, int& inf_type) {
		bool bIsInf{ false };
		bool s{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(value, s, rawExponent, rawFraction, bits);
		if (rawExponent == ieee754_parameter<float>::eallset) { // nan and inf need to be remapped
			if (rawFraction == 0ull) {
				// 1.11111111.0000000.......000000000 -inf
				// 0.11111111.0000000.......000000000 +inf
				inf_type = (s ? INF_TYPE_NEGATIVE : INF_TYPE_POSITIVE);
				bIsInf = true;
			}
			else {
				inf_type = INF_TYPE_NEITHER;
				bIsInf = false;
			}
		}
		return bIsInf;
	}

	template<>
	inline BIT_CAST_CONSTEXPR bool checkInf(double value, int& inf_type) {
		bool bIsInf{ false };
		bool s{ false };
		uint64_t rawExponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		extractFields(value, s, rawExponent, rawFraction, bits);
		if (rawExponent == ieee754_parameter<double>::eallset) { // nan and inf need to be remapped
			if (rawFraction == 0ull) {
				// 1.11111111.0000000.......000000000 -inf
				// 0.11111111.0000000.......000000000 +inf
				inf_type = (s ? INF_TYPE_NEGATIVE : INF_TYPE_POSITIVE);
				bIsInf = true;
			}
			else {
				inf_type = INF_TYPE_NEITHER;
				bIsInf = false;
			}
		}
		return bIsInf;
	}
	
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
