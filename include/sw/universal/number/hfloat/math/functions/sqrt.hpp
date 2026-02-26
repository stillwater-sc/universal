#pragma once
// sqrt.hpp: sqrt functions for IBM System/360 hexadecimal floating-point hfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// square root of x
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> sqrt(const hfloat<ndigits, es, bt>& a) {
#if HFLOAT_THROW_ARITHMETIC_EXCEPTION
	if (a.isneg()) throw hfloat_negative_sqrt_arg();
#else
	if (a.isneg()) std::cerr << "hfloat argument to sqrt is negative: " << a << std::endl;
#endif
	if (a.iszero()) return a;
	return hfloat<ndigits, es, bt>(std::sqrt(double(a)));
}

}} // namespace sw::universal
