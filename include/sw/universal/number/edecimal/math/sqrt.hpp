#pragma once
// sqrt.hpp: sqrt functions for adaptive precision decimal integers
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>       // std::sqrt on the double fallback path
#include <cstdio>      // fprintf(stderr,...) for the diagnostic (#1334: no <iostream> in a math header)
#include <universal/native/ieee754_core.hpp>
#include <universal/number/edecimal/core.hpp>            // the edecimal type itself; this header was relying on a prior include
#include <universal/number/edecimal/numeric_limits.hpp>

#ifndef EDECIMAL_NATIVE_SQRT
#define EDECIMAL_NATIVE_SQRT 0
#endif

namespace sw { namespace universal {

#if EDECIMAL_NATIVE_SQRT
	// native sqrt for decimal
	inline decimal sqrt(const decimal& f) {
		if (f < 0) throw decimal_negative_sqrt_arg();
		using Decimal = decimal;
		constexpr Decimal eps = std::numeric_limits<Rational>::epsilon();
		Decimal y(f);
		Decimal x(f);
		x >>= 1; // divide by 2
		Decimal diff = (x * x - y);
		int iterations = 0;
		while (sw::universal::abs(diff) > eps) {
			x = (x + y);
			x >>= 1;
			y = f / x;
			diff = x - y;
//			std::cout << " x: " << x << " y: " << y << " diff " << diff << '\n';
			if (++iterations > rbits) break;
		}
		if (iterations > rbits) std::cerr << "sqrt(" << double(f) << ") failed to converge\n";
		return x;
	}
#else
	inline edecimal sqrt(const edecimal& f) {
		// The negative-argument report has to be INSIDE the sign test. It was not: the
		// #else branch ran unconditionally, so every call -- sqrt(144) included -- wrote
		// "decimal_negative_sqrt_arg" to stderr while returning the right answer.
		if (f.isneg()) {
#if EDECIMAL_THROW_ARITHMETIC_EXCEPTION
			throw edecimal_negative_sqrt_arg();
#else
			std::fprintf(stderr, "edecimal_negative_sqrt_arg\n");
			return edecimal(0);
#endif
		}
		return edecimal(std::sqrt((double)f));
	}
#endif

	// reciprocal sqrt
//	inline edecimal rsqrt(const edecimal& f) {
//		edecimal rsqrt = sqrt(f);
//		return rsqrt.reciprocate();
//	}

	///////////////////////////////////////////////////////////////////
	// specialized sqrt configurations

}} // namespace sw::universal
