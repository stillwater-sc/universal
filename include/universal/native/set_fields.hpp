#pragma once
// set_fields.hpp: configure constexpr/nonconst manipulation functions for IEEE-754 native types using C++20 <bit>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>

namespace sw { namespace universal {

#if BIT_CAST_IS_CONSTEXPR
#include <bit>    // C++20 bit_cast

	inline BIT_CAST_CONSTEXPR void setbit(float& v, unsigned index, bool b = true) {
		uint32_t raw = std::bit_cast<uint32_t, float>(v);
		uint32_t mask = (1u << index); // do we want to bound check?
		if (b) raw |= mask; else raw &= ~mask;
		v = std::bit_cast<float, uint32_t>(raw);
	}

	inline BIT_CAST_CONSTEXPR void setbit(double& v, unsigned index, bool b = true) {
		uint64_t raw = std::bit_cast<uint64_t, double>(v);
		uint64_t mask = (1ull << index);
		if (b) raw |= mask; else raw &= ~mask;
		v = std::bit_cast<double, uint64_t>(raw);
	}

#if LONG_DOUBLE_SUPPORT

// Clang bit_cast<> can't deal with long double

#if defined(LONG_DOUBLE_DOWNCAST)
	// specialization to set fields on a long double
	inline void setFields(long double& value, bool s, uint64_t rawExponentBits, uint64_t rawFractionBits) noexcept {
		double dv = double(value);
		setFields(dv, s, rawExponentBits, rawFractionBits);
		value = (long double)(dv);
	}

#else // !DOWNCAST

/*
		struct blob {
			std::uint64_t hi;
			std::uint64_t fraction;
		} raw;
		raw = std::bit_cast<blob, long double>(value);
		s = (ieee754_parameter<long double>::smask & raw.hi);
		rawExponentBits = (ieee754_parameter<long double>::emask & raw.hi);
		rawFractionBits = (ieee754_parameter<long double>::fmask & raw.fraction);
*/
	/*
	ETLO 8/1/2024: not able to make std::bit_cast<> work for long double
	// specialization to set fields on a long double
	inline BIT_CAST_CONSTEXPR setbit(long double& v, unsigned index, bool b) {
		struct blob {
			std::uint64_t hi;
			std::uint64_t fraction;
		} bits;
		bits = std::bit_cast<blob, long double>(value);
		uint64_t raw{};
		if (index < 64) raw = bits.fraction; else raw = bits.hi;
		uint64_t mask = (1ull << index);
		if (b) raw |= mask; else raw &= ~mask;
		if (index < 64) bits.fraction = raw; else bits.hi = raw;
		v = std::bit_cast<blob, long double>(bits);
	}
	*/
	// falling back to non-constexpr
	// specialization to set fields on a long double
	inline void setFields(long double& value, bool s, uint64_t rawExponentBits, uint64_t rawFractionBits) noexcept {
		long_double_decoder decoder;
		decoder.parts.sign = s;
		decoder.parts.exponent = rawExponentBits & 0x7FFF;
		decoder.parts.fraction = rawFractionBits & 0xFFFF'FFFF'FFFF'FFFF;
		value = decoder.ld;
	}

#endif // LONG_DOUBLE_DOWNCAST
#endif // LONG_DOUBLE_SUPPORT

#else // !BIT_CAST_IS_CONSTEXPR

	inline void setbit(float& v, unsigned index, bool b = true) {
		float_decoder decoder;
		decoder.f = v;
		if (index == 31) {
			decoder.parts.sign = b;
		}
		else if (index < 23) { // 22...0 are fraction bits
			uint32_t raw = decoder.parts.fraction;
			uint32_t mask = (1ull << index);
			if (b) raw |= mask; else raw &= ~mask;
			decoder.parts.fraction = raw;
		}
		else if (index < 32) {
			uint32_t raw = decoder.parts.exponent;
			uint32_t mask = (1ull << (index - 23));
			if (b) raw |= mask; else raw &= ~mask;
			decoder.parts.exponent = raw;
		}
		v = decoder.f;
	}

	inline void setbit(double& v, unsigned index, bool b = true) {
		double_decoder decoder;
		decoder.d = v;
		if (index == 63) {
			decoder.parts.sign = b;
		}
		else if (index < 52) { // 51...0 are fraction bits
			uint64_t raw = decoder.parts.fraction;
			uint64_t mask = (1ull << index);
			if (b) raw |= mask; else raw &= ~mask;
			decoder.parts.fraction = raw;
		}
		else if (index < 64) {
			uint64_t raw = decoder.parts.exponent;
			uint64_t mask = (1ull << (index - 52));
			if (b) raw |= mask; else raw &= ~mask;
			decoder.parts.exponent = raw;
		}
		v = decoder.d;
	}

////////////////////////////////////////////////////////////////////////
// nonconst setFields on single precision floating-point

	inline void setFields(float& value, bool s, uint64_t rawExponentBits, uint64_t rawFractionBits) noexcept {
		float_decoder decoder;
		decoder.parts.sign = s;
		decoder.parts.exponent = rawExponentBits & 0xFF;
		decoder.parts.fraction = rawFractionBits & 0x7FFFFF;
		value = decoder.f;
	}

////////////////////////////////////////////////////////////////////////
// nonconst setFields on double precision floating-point

	inline void setFields(double& value, bool s, uint64_t rawExponentBits, uint64_t rawFractionBits) noexcept {
		double_decoder decoder;
		decoder.parts.sign = s;
		decoder.parts.exponent = rawExponentBits & 0x7FF;
		decoder.parts.fraction = rawFractionBits & 0xF'FFFF'FFFF'FFFF;
		value = decoder.d;
	}

#if LONG_DOUBLE_SUPPORT
// Clang bit_cast<> doesn't appear to deal with long double
#define LONG_DOUBLE_DOWNCAST
#ifdef LONG_DOUBLE_DOWNCAST
	inline void setFields(long double& value, bool s, uint64_t rawExponentBits, uint64_t rawFractionBits) noexcept {
		double dv = double(value);
		setFields(dv, s, rawExponentBits, rawFractionBits);
		value = (long double)(dv);
	}
#else
	// specialization to extract fields from a long double
	inline void setFields(long double& value, bool s, uint64_t rawExponentBits, uint64_t rawFractionBits) noexcept {
		long_double_decoder decoder;
		decoder.parts.sign = s;
		decoder.parts.exponent = rawExponentBits & 0x7FFF;
		decoder.parts.fraction = rawFractionBits & 0xFFFF'FFFF'FFFF'FFFF;
		value = decoder.ld;
	}
#endif // LONG_DOUBLE_DOWNCAST
#endif // LONG_DOUBLE_SUPPORT
#endif // BIT_CAST_IS_CONSTEXPR

}} // namespace sw::universal
