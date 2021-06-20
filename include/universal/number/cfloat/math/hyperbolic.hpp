#pragma once
// hyperbolic.hpp: hyperbolic functions for classic floating-point cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// hyperbolic sine of an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> sinh(cfloat<nbits,es,bt> x) {
	return cfloat<nbits,es,bt>(std::sinh(double(x)));
}

// hyperbolic cosine of an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> cosh(cfloat<nbits,es,bt> x) {
	return cfloat<nbits,es,bt>(std::cosh(double(x)));
}

// hyperbolic tangent of an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> tanh(cfloat<nbits,es,bt> x) {
	return cfloat<nbits,es,bt>(std::tanh(double(x)));
}

// hyperbolic cotangent of an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> atanh(cfloat<nbits,es,bt> x) {
	return cfloat<nbits,es,bt>(std::atanh(double(x)));
}

// hyperbolic cosecant of an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> acosh(cfloat<nbits,es,bt> x) {
	return cfloat<nbits,es,bt>(std::acosh(double(x)));
}

// hyperbolic secant of an angle of x radians
template<size_t nbits, size_t es, typename bt>
cfloat<nbits,es,bt> asinh(cfloat<nbits,es,bt> x) {
	return cfloat<nbits,es,bt>(std::asinh(double(x)));
}


}  // namespace sw::universal
