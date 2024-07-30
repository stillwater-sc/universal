#pragma once
// sqrt.hpp: sqrt functions for doubledouble (dd) floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>

#ifndef DOUBLEDOUBLE_NATIVE_SQRT
#define DOUBLEDOUBLE_NATIVE_SQRT 0
#endif

namespace sw { namespace universal {

#if DOUBLEDOUBLE_NATIVE_SQRT
	// sqrt for doubledouble
	inline dd sqrt(dd a) {
#if DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) throw dd_negative_sqrt_arg();
#else
		if (a.isneg()) std::cerr << "doubledouble argument to sqrt is negative: " << a << std::endl;
#endif
		if (a.iszero()) return a;
		return dd(std::sqrt((a.high()));  // TBD
	}
#else
	
	// sqrt shim for doubledouble
	inline dd sqrt(dd a) {
#if DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) throw dd_negative_sqrt_arg();
#else
		if (a.isneg()) std::cerr << "doubledouble argument to sqrt is negative: " << a << std::endl;
#endif
		if (a.iszero()) return a;
		return dd(std::sqrt(a.high()));
	}
#endif

	// reciprocal sqrt
	inline dd rsqrt(dd a) {
		dd v = sw::universal::sqrt(a);
		return reciprocal(v);
	}

}} // namespace sw::universal
