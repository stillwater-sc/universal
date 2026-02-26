#pragma once
// minmax.hpp: min/max functions for classic floating-point cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> 
min(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::min(double(x), double(y)));
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> 
max(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::max(double(x), double(y)));
}

}} // namespace sw::universal
