#pragma once
// hyperbolic.hpp: hyperbolic functions for classic floating-point cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// hyperbolic sine of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> sinh(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::sinh(double(x)));
}

// hyperbolic cosine of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> cosh(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::cosh(double(x)));
}

// hyperbolic tangent of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> tanh(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::tanh(double(x)));
}

// hyperbolic cotangent of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> atanh(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::atanh(double(x)));
}

// hyperbolic cosecant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> acosh(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::acosh(double(x)));
}

// hyperbolic secant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> asinh(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::asinh(double(x)));
}


}} // namespace sw::universal
