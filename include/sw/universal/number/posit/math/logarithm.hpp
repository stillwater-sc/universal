#pragma once
// logarithm.hpp: logarithm functions for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Natural logarithm of x
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> log(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::log(double(x)));
}

// Binary logarithm of x
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> log2(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::log2(double(x)));
}

// Decimal logarithm of x
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> log10(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::log10(double(x)));
}
		
// Natural logarithm of 1+x
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> log1p(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::log1p(double(x)));
}

}} // namespace sw::universal
