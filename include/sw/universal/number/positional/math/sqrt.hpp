#pragma once
// sqrt.hpp: sqrt function for positional integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>

namespace sw { namespace universal {

// integer sqrt via double
template<unsigned ndigits, unsigned radix>
inline positional<ndigits, radix> sqrt(const positional<ndigits, radix>& a) {
#if POSITIONAL_THROW_ARITHMETIC_EXCEPTION
	if (a.isneg()) throw positional_negative_sqrt_arg();
#else
	if (a.isneg()) std::cerr << "positional argument to sqrt is negative: " << a << std::endl;
#endif
	if (a.iszero()) return a;
	return positional<ndigits, radix>(::std::sqrt((double)a));
}

}} // namespace sw::universal
