#pragma once
// sqrt.hpp: sqrt functions for rationals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>
#include <universal/number/rational/numeric_limits.hpp>

#ifndef RATIONAL_NATIVE_SQRT
#define RATIONAL_NATIVE_SQRT 0
#endif

namespace sw { namespace universal {

#if RATIONAL_NATIVE_SQRT
	// native sqrt for rational
	inline rational sqrt(const rational& f) {
		if (f < 0) throw rational_negative_sqrt_arg();
		using Rational = rational;
		constexpr Rational eps = std::numeric_limits<Rational>::epsilon();
		Rational y(f);
		Rational x(f);
		x >>= 1; // divide by 2
		Rational diff = (x * x - y);
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
	inline rational sqrt(const rational& f) {
#if RATIONAL_THROW_ARITHMETIC_EXCEPTION
		if (f.isneg()) {
			throw rational_negative_sqrt_arg();
		}
#else
		std::cerr << "rational_negative_sqrt_arg\n";
#endif
		return rational(std::sqrt((double)f));
	}
#endif

	// reciprocal sqrt
//	inline rational rsqrt(const rational& f) {
//		rational rsqrt = sqrt(f);
//		return rsqrt.reciprocate();
//	}

	///////////////////////////////////////////////////////////////////
	// specialized sqrt configurations

}} // namespace sw::universal
