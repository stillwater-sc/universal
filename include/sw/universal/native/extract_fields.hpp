#pragma once
// extract_fields.hpp: configure constexpr/nonconst manipulation functions for IEEE-754 native types using C++20 <bit>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <bit>      // std::endian
#include <cfloat>   // LDBL_MANT_DIG
#include <cmath>    // std::frexp, std::ldexp
#include <cstdint>
#include <cstring>  // std::memcpy
#include <universal/native/long_double_significand.hpp>  // the full significand, for types wider than 62 bits (#1517)
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>

namespace sw { namespace universal {

#if LONG_DOUBLE_SUPPORT && (LDBL_MANT_DIG != 64) && (LDBL_MANT_DIG != 53)
	// the quiet bit of a long double NaN, read from the format's own leading fraction bit
	inline bool longDoubleNaNIsQuiet(long double value) noexcept {
#	if LDBL_MANT_DIG == 113  // IEEE binary128: the fraction's MSB, bit 111
		std::uint64_t word[2]{};
		std::memcpy(word, &value, sizeof(word));
		const std::uint64_t upper = (std::endian::native == std::endian::little) ? word[1] : word[0];
		return ((upper >> 47) & 1u) != 0;
#	elif LDBL_MANT_DIG == 106  // IBM double-double: the leading double carries the NaN
		std::uint64_t leading{};
		std::memcpy(&leading, &value, sizeof(leading));
		return ((leading >> 51) & 1u) != 0;
#	else
		(void)value;
		return true;
#	endif
	}

	// x87-shaped fields of a long double that is not x87: IEEE binary128 (aarch64 Linux) or IBM
	// double-double (POWER). Callers read the fields through ieee754_parameter<long double>, which
	// describes the x87 layout -- sign, a 15-bit exponent biased by 16383, 63 fraction bits below an
	// explicit integer bit -- and carry the fraction in a uint64_t. The x87 bit decoder read these
	// formats as if they were x87 and got the value wrong: 0.75 as 0.5, 3 as 2, on POWER 0 (#1515).
	// So the fields are computed from the value instead: frexp gives the exponent, and the
	// significand is cut to 64 bits with any bits below folded into the last one (round to odd).
	// A caller that rounds that to 62 significand bits or fewer rounds the long double correctly,
	// and a caller that truncates truncates it correctly; a wider one gets its leading 64 bits.
	// NaN and inf take x87's canonical encodings, so they classify as they do on x86.
	inline void extractLongDoubleFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& lowerBits, uint64_t& upperBits) noexcept {
		constexpr int           bias        = 16383;
		constexpr std::uint64_t integerBit  = 0x8000'0000'0000'0000ull;
		constexpr std::uint64_t fmask       = 0x7FFF'FFFF'FFFF'FFFFull;
		s = std::signbit(value);
		if (value != value) {  // nan
			rawExponentBits = 0x7FFF;
			lowerBits       = longDoubleNaNIsQuiet(value) ? 0x4000'0000'0000'0000ull : 0x2000'0000'0000'0000ull;
			upperBits       = 0;   // the shaped significand is 64 bits: nothing above it
			return;
		}
		if (std::isinf(value)) {
			rawExponentBits = 0x7FFF;
			lowerBits       = 0;
			upperBits       = 0;   // the shaped significand is 64 bits: nothing above it
			return;
		}
		if (value == 0.0l) {
			rawExponentBits = 0;
			lowerBits       = 0;
			upperBits       = 0;   // the shaped significand is 64 bits: nothing above it
			return;
		}
		int               e   = 0;
		const long double sig = std::ldexp(std::frexp(std::fabs(value), &e), 64);  // [2^63, 2^64)
		std::uint64_t     top = static_cast<std::uint64_t>(sig);                 // the leading 64 bits
		if (sig != static_cast<long double>(top)) top |= 1u;                     // round to odd
		const int biased = e - 1 + bias;  // |value| = 1.f * 2^(e - 1)
		if (biased >= 1) {
			rawExponentBits = static_cast<std::uint64_t>(biased);
			lowerBits       = top & fmask;
			upperBits       = 0;   // the shaped significand is 64 bits: nothing above it
		}
		else {  // below x87's normal range, which only binary128 reaches: an x87 denormal
			const int     shift = 1 - biased;
			std::uint64_t f     = (shift < 64) ? (top >> shift) : 0u;
			if (shift >= 64 || (top & ((std::uint64_t(1) << shift) - 1u)) != 0) f |= 1u;  // round to odd
			rawExponentBits = 0;
			lowerBits       = f;
			upperBits       = 0;   // the shaped significand is 64 bits: nothing above it
		}
	}
#endif

#if BIT_CAST_IS_CONSTEXPR
// sw::bit_cast is provided by <universal/utility/bit_cast.hpp>

// THE CONTRACT (#1536): extractFields is where a native floating-point value is taken apart, and
// the fields it hands back are
//
//   s                the sign
//   rawExponentBits  the biased exponent, as the format encodes it
//   lowerBits        the fraction, low word
//   upperBits        the fraction bits above 64, for a format that has them
//
// upperBits is what the old `bits` parameter became. That one carried the whole encoding for a
// float and a double but only the low word for a long double, no caller ever read it, and a
// uint64_t fraction is what made ieee_components unable to describe a binary128 at all -- the
// reason that function is gone and this one is the only way in.
//
// upperBits is zero in every configuration today: float, double and x87 all have fewer than 64
// fraction bits, and the formats that have more -- IEEE binary128, IBM double-double -- come
// through extractLongDoubleFields, which hands back x87-SHAPED fields so that every consumer can
// read them through ieee754_parameter<long double> (#1515). Giving those formats their own
// two-word fields means giving the consumers a second shape to read, which is a separate change.

	// specialization to extract fields from a float
	inline BIT_CAST_CONSTEXPR void extractFields(float value, bool& s, uint64_t& rawExponentBits, uint64_t& lowerBits, uint64_t& upperBits) noexcept {
		uint32_t bc = sw::bit_cast<uint32_t>(value);
		s = (ieee754_parameter<float>::smask & bc);
		rawExponentBits = (ieee754_parameter<float>::emask & bc) >> ieee754_parameter<float>::fbits;
		lowerBits = (ieee754_parameter<float>::fmask & bc);
		upperBits = 0;   // no fraction bits above the low word in this format
	}

	// specialization to extract fields from a double
	inline BIT_CAST_CONSTEXPR void extractFields(double value, bool& s, uint64_t& rawExponentBits, uint64_t& lowerBits, uint64_t& upperBits) noexcept {
		uint64_t bc = sw::bit_cast<uint64_t>(value);
		s = (ieee754_parameter<double>::smask & bc);
		rawExponentBits = (ieee754_parameter<double>::emask & bc) >> ieee754_parameter<double>::fbits;
		lowerBits = (ieee754_parameter<double>::fmask & bc);
		upperBits = 0;   // no fraction bits above the low word in this format
	}

#if LONG_DOUBLE_SUPPORT

// Clang bit_cast<> can't deal with long double

#if defined(LONG_DOUBLE_DOWNCAST)

	inline BIT_CAST_CONSTEXPR void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& lowerBits, uint64_t& upperBits) noexcept {
		double d = static_cast<double>(value);
		uint64_t bc = sw::bit_cast<uint64_t>(d);
		s = (ieee754_parameter<double>::smask & bc);
		rawExponentBits = (ieee754_parameter<double>::emask & bc) >> ieee754_parameter<double>::fbits;
		lowerBits = (ieee754_parameter<double>::fmask & bc);
		upperBits = 0;   // no fraction bits above the low word in this format
	}
#else // !DOWNCAST
/*
	ETLO 8/1/2024: not able to make std::bit_cast<> work for long double
	// specialization to extract fields from a long double

	inline BIT_CAST_CONSTEXPR void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& lowerBits, uint64_t& upperBits) noexcept {
		struct blob {
			std::uint64_t hi;
			std::uint64_t fraction;
		} raw;
		raw = std::bit_cast<blob, long double>(value);
		s = (ieee754_parameter<long double>::smask & raw.hi);
		rawExponentBits = (ieee754_parameter<long double>::emask & raw.hi);
		lowerBits = (ieee754_parameter<long double>::fmask & raw.fraction);
	}
	*/
	// falling back to non-constexpr
	// specialization to extract fields from a long double
#if (LDBL_MANT_DIG == 64) || (LDBL_MANT_DIG == 53)  // x87, or long double is double: read the bits
	inline void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& lowerBits, uint64_t& upperBits) noexcept {
		long_double_decoder decoder;
		decoder.ld = value;
		s = decoder.parts.sign ? true : false;
		rawExponentBits = decoder.parts.exponent;
		lowerBits = decoder.parts.fraction;
		upperBits = 0;   // no fraction bits above the low word in this format
	}
#else  // binary128, double-double: x87-shaped fields computed from the value (#1515)
	inline void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& lowerBits, uint64_t& upperBits) noexcept {
		extractLongDoubleFields(value, s, rawExponentBits, lowerBits, upperBits);
	}
#endif

#endif // LONG_DOUBLE_DOWNCAST
#endif // LONG_DOUBLE_SUPPORT

#else // !BIT_CAST_IS_CONSTEXPR

////////////////////////////////////////////////////////////////////////
// nonconst extractFields for single precision floating-point

	// the fields come back in a uint64_t here as they do on the constexpr path above. They used to
	// be uint32_t on this one, so the same call could not serve every Real: generic code passing
	// uint64_t got an exact match for a double and, for a float, an overload it could not bind to,
	// leaving the double and long double candidates to tie. MSVC, which takes this path, reported
	// the ambiguity (#1536).
	inline void extractFields(float value, bool& s, uint64_t& rawExponentBits, uint64_t& lowerBits, uint64_t& upperBits) noexcept {
		float_decoder decoder;
		decoder.f = value;
		s = decoder.parts.sign ? true : false;
		rawExponentBits = decoder.parts.exponent;
		lowerBits = decoder.parts.fraction;
		upperBits = 0;   // no fraction bits above the low word in this format
	}

////////////////////////////////////////////////////////////////////////
// nonconst extractFields for double precision floating-point

	inline void extractFields(double value, bool& s, uint64_t& rawExponentBits, uint64_t& lowerBits, uint64_t& upperBits) noexcept {
		double_decoder decoder;
		decoder.d = value;
		s = decoder.parts.sign ? true : false;
		rawExponentBits = decoder.parts.exponent;
		lowerBits = decoder.parts.fraction;
		upperBits = 0;   // no fraction bits above the low word in this format
	}

#if LONG_DOUBLE_SUPPORT
// Clang bit_cast<> can't deal with long double
#define LONG_DOUBLE_DOWNCAST
#if (LDBL_MANT_DIG != 64) && (LDBL_MANT_DIG != 53)  // binary128, double-double: x87-shaped fields (#1515)
	inline void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& lowerBits, uint64_t& upperBits) noexcept {
		extractLongDoubleFields(value, s, rawExponentBits, lowerBits, upperBits);
	}
#elif defined(LONG_DOUBLE_DOWNCAST)
	inline void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& lowerBits, uint64_t& upperBits) noexcept {
		extractFields(double(value), s, rawExponentBits, lowerBits, upperBits);
	}
#else
	// specialization to extract fields from a long double
	inline void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& lowerBits, uint64_t& upperBits) noexcept {
		long_double_decoder decoder;
		decoder.ld = value;
		s = decoder.parts.sign ? true : false;
		rawExponentBits = decoder.parts.exponent;
		lowerBits = decoder.parts.fraction;
		upperBits = 0;   // no fraction bits above the low word in this format
	}
#endif // LONG_DOUBLE_DOWNCAST
#endif // LONG_DOUBLE_SUPPORT
#endif // BIT_CAST_IS_CONSTEXPR

#if !LONG_DOUBLE_SUPPORT
	// A host whose long double this library cannot take apart still HAS the type, and it is a
	// distinct one for overload resolution, so a call has to have somewhere to go: with only the
	// float and double overloads in scope, extractFields(aLongDouble, ...) is ambiguous rather than
	// absent. ieee_components carried a forwarding overload for exactly this, and retiring it left
	// the hole (#1536); the same reasoning as #1535.
	//
	// LONG_DOUBLE_SUPPORT is 0 only where long double is double -- MSVC, and a GNU build with
	// -mlong-double-64 -- so forwarding to the double overload reads the very same bits.
	inline void extractFields(long double value, bool& s, uint64_t& rawExponentBits, uint64_t& lowerBits, uint64_t& upperBits) noexcept {
		extractFields(static_cast<double>(value), s, rawExponentBits, lowerBits, upperBits);
	}
#endif

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

}} // namespace sw::universal
