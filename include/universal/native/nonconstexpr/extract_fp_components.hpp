#pragma once
// extract_fp_components.hpp: nonconstexpr implementation of IEEE-754 float/double/long double floating-point component field extractions
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>

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
// this implementation assumes an 80bit extended precision representation
// TODO: support a full quad precision long double
inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, std::uint64_t& _fraction) {
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

}} // namespace sw::universal


