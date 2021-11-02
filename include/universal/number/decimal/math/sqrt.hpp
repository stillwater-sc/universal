#pragma once
// sqrt.hpp: sqrt functions for decimals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>
#include <universal/number/decimal/numeric_limits.hpp>

#ifndef DECIMAL_NATIVE_SQRT
#define DECIMAL_NATIVE_SQRT 0
#endif

namespace sw::universal {

#if DECIMAL_NATIVE_SQRT
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
	inline decimal sqrt(const decimal& f) {
#if DECIMAL_THROW_ARITHMETIC_EXCEPTION
		if (f.isneg()) {
			throw decimal_negative_sqrt_arg();
		}
#else
		std::cerr << "decimal_negative_sqrt_arg\n";
#endif
		return decimal(std::sqrt((double)f));
	}
#endif

	// reciprocal sqrt
//	inline decimal rsqrt(const decimal& f) {
//		decimal rsqrt = sqrt(f);
//		return rsqrt.reciprocate();
//	}

	///////////////////////////////////////////////////////////////////
	// specialized sqrt configurations

} // namespace sw::universal
