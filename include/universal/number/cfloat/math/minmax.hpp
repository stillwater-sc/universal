#pragma once
// minmax.hpp: min/max functions for classic floating-point cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormal, bool hasSupernormal, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormal, hasSupernormal, isSaturating> 
min(cfloat<nbits, es, bt, hasSubnormal, hasSupernormal, isSaturating> x, cfloat<nbits, es, bt, hasSubnormal, hasSupernormal, isSaturating> y) {
	return cfloat<nbits, es, bt, hasSubnormal, hasSupernormal, isSaturating>(std::min(double(x), double(y)));
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormal, bool hasSupernormal, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormal, hasSupernormal, isSaturating> 
max(cfloat<nbits, es, bt, hasSubnormal, hasSupernormal, isSaturating> x, cfloat<nbits, es, bt, hasSubnormal, hasSupernormal, isSaturating> y) {
	return cfloat<nbits, es, bt, hasSubnormal, hasSupernormal, isSaturating>(std::max(double(x), double(y)));
}

}} // namespace sw::universal
