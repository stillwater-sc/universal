#pragma once
// extract_fp_components.hpp: nonconstexpr implementation of IEEE-754 float/double/long double floating-point component field extractions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <cstring>
#include <universal/internal/uint128/uint128.hpp>

/*
 * The frexpf/frexp/frexpl functions have become constexpr in C++23. Universal is using the <bit> library
 * for more control. These functions are just here to support old compilers.
 */
namespace sw { namespace universal {

// native IEEE-754 component extraction for float
inline void extract_fp_components(float fp, bool& _sign, int& _exponent, float& _fr, unsigned int& _fraction) {
	_sign = fp < 0.0;
	_fr = ::std::frexp(fp, &_exponent);
	_fraction = static_cast<std::uint32_t>(0x007F'FFFFul) & reinterpret_cast<uint32_t&>(_fr);
}

// native IEEE-754 component extraction for double
inline void extract_fp_components(double fp, bool& _sign, int& _exponent, double& _fr, unsigned long long& _fraction) {
	_sign = fp < 0.0;
	_fr = ::std::frexp(fp, &_exponent);
	_fraction = static_cast<uint64_t>(0x000F'FFFF'FFFF'FFFFull) & reinterpret_cast<uint64_t&>(_fr);
}

// native IEEE-754 component extraction for long double
// this implementation supports an 80bit extended precision representation
inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, std::uint64_t& _fraction) {
	assert(sizeof(long double) == 8 || (sizeof(long double) == 16 && std::numeric_limits<long double>::digits <= 64));
	if constexpr (sizeof(long double) == 8) { // check if (long double) is aliased to be just a double
		_sign = fp < 0.0;
		_fr = (long double)(::std::frexp(double(fp), &_exponent));
		_fraction = 0x000F'FFFF'FFFF'FFFFull & reinterpret_cast<uint64_t&>(_fr);
	}
	else if constexpr (sizeof(long double) == 16 && std::numeric_limits<long double>::digits <= 64) {
		_sign = fp < 0.0;
		_fr = ::std::frexp(fp, &_exponent);
		_fraction = 0x7FFF'FFFF'FFFF'FFFFull & reinterpret_cast<uint64_t&>(_fr); // 80bit extended format only has 63bits of fraction
	}
}

// native IEEE-754 component extraction for long double
// this implementation supports a proper 128bit quad precision representation
inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, internal::uint128& _fraction) {
	assert(sizeof(long double) == 16 && std::numeric_limits<long double>::digits > 64);
	_sign = fp < 0.0;
	_fr = ::std::frexp(fp, &_exponent);

	std::memcpy(&_fraction, &_fr, sizeof(internal::uint128));

	// we need to remove the upper bits that are not part of the mantissa. (all bits - mantissa bits - 1). -1 because the first bit is not stored
	// we only need to do this on the upper part of the uint128, as we asserted that the mantissa has more than 64 bits.
	constexpr int nrUpperBits = 8 * sizeof(long double) - (std::numeric_limits<long double>::digits - 1);
	constexpr int shift = (nrUpperBits < 64 ? nrUpperBits : 0);
	if constexpr (shift < 64) {
		_fraction.upper <<= shift;
		_fraction.upper >>= shift;
	}
	else {
		_fraction.upper = 0;
	}
}

}} // namespace sw::universal


