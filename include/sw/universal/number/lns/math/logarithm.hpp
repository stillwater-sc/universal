#pragma once
// logarithm.hpp: logarithm functions for logarithmic floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Natural logarithm of x
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> log(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::log(double(x)));
}

// Binary logarithm of x
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> log2(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::log2(double(x)));
}

// Decimal logarithm of x
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> log10(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::log10(double(x)));
}
		
// Natural logarithm of 1+x
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> log1p(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::log1p(double(x)));
}

}} // namespace sw::universal
