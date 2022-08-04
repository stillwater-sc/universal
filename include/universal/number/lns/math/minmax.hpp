#pragma once
// minmax.hpp: min/max functions for logarithmic floating-point
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<size_t nbits, size_t rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...>
min(lns<nbits, rbits, bt, xtra...> x, lns<nbits, rbits, bt, xtra...> y) {
	return lns<nbits, rbits, bt, xtra...>(std::min(double(x), double(y)));
}

template<size_t nbits, size_t rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...>
max(lns<nbits, rbits, bt, xtra...> x, lns<nbits, rbits, bt, xtra...> y) {
	return lns<nbits, rbits, bt, xtra...>(std::max(double(x), double(y)));
}

}} // namespace sw::universal
