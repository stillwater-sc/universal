#pragma once
// fractional.hpp: fractional functions for classic floating-point cfloats
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// fmod retuns x - n*y where n = x/y with the fractional part truncated
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> fmod(cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> x, cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> y) {
	if (x < y) return x;
	using Real = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Real n(x/y);
	n.truncate();
	return x - n * y;
}

// shim to stdlib
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> remainder(cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> x, cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> y) {
	return cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating>(std::remainder(double(x), double(y)));
}

// TODO: validate the rounding of these conversion, versus a method that manipulates the fraction bits directly

// frac returns the fraction of a cfloat value that is > 1
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> frac(cfloat<nbits,es, bt, hasSubnormals, hasSupernormals, isSaturating> x) {
	using Real = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	long long intValue = (long long)(x);
	return (x-Real(intValue));
}


}} // namespace sw::universal
