#pragma once
// sqrt.hpp: sqrt function for decimal floating-point dfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

#if DFLOAT_NATIVE_SQRT
// native sqrt for dfloat
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
inline dfloat<ndigits, es, Encoding, bt> sqrt(const dfloat<ndigits, es, Encoding, bt>& a) {
#if DFLOAT_THROW_ARITHMETIC_EXCEPTION
	if (a.isneg()) throw dfloat_negative_sqrt_arg();
#else
	if (a.isneg()) std::cerr << "dfloat argument to sqrt is negative: " << a << std::endl;
#endif
	if (a.iszero()) return a;
	return dfloat<ndigits, es, Encoding, bt>(std::sqrt(double(a)));  // TBD
}
#else
// sqrt delegating through double
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
inline dfloat<ndigits, es, Encoding, bt> sqrt(const dfloat<ndigits, es, Encoding, bt>& a) {
#if DFLOAT_THROW_ARITHMETIC_EXCEPTION
	if (a.isneg()) throw dfloat_negative_sqrt_arg();
#else
	if (a.isneg()) std::cerr << "dfloat argument to sqrt is negative: " << a << std::endl;
#endif
	if (a.iszero()) return a;
	return dfloat<ndigits, es, Encoding, bt>(std::sqrt(double(a)));
}
#endif

// reciprocal sqrt
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
inline dfloat<ndigits, es, Encoding, bt> rsqrt(const dfloat<ndigits, es, Encoding, bt>& a) {
	dfloat<ndigits, es, Encoding, bt> v = sqrt(a);
	return v.reciprocate();
}

}} // namespace sw::universal
