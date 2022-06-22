#pragma once
// extract_fp_components.hpp: nonconstexpr implementation of IEEE-754 float and double floating-point component field extraction
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>

namespace sw { namespace universal {

// native IEEE-754 component extraction for float
inline void extract_fp_components(float fp, bool& _sign, int& _exponent, float& _fr, unsigned int& _fraction) {
	static_assert(sizeof(float) == 4, "This function only works when float is 32 bit.");
	_sign = fp < 0.0 ? true : false;
	_fr = frexpf(fp, &_exponent);
	_fraction = static_cast<std::uint32_t>(0x007F'FFFFul) & reinterpret_cast<uint32_t&>(_fr);
}

// native IEEE-754 component extraction for double
inline void extract_fp_components(double fp, bool& _sign, int& _exponent, double& _fr, unsigned long long& _fraction) {
	static_assert(sizeof(double) == 8, "This function only works when double is 64 bit.");
	_sign = fp < 0.0 ? true : false;
	_fr = std::frexp(fp, &_exponent);
	_fraction = static_cast<uint64_t>(0x000F'FFFF'FFFF'FFFFull) & reinterpret_cast<uint64_t&>(_fr);
}

}} // namespace sw::universal


