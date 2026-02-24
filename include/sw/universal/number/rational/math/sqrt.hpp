#pragma once
// sqrt.hpp: sqrt functions for rational
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>

namespace sw { namespace universal {

	// sqrt for arbitrary rational
	template<unsigned nbits, typename Base, typename bt>
	inline rational<nbits, Base, bt> sqrt(const rational<nbits, Base, bt>& a) {
#if RATIONAL_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) throw rational_negative_sqrt_arg();
#else
		if (a.isneg()) std::cerr << "rational argument to sqrt is negative: " << a << std::endl;
#endif
		if (a.iszero()) return a;
		return rational<nbits, Base, bt>(std::sqrt((double)a));
	}

}} // namespace sw::universal
