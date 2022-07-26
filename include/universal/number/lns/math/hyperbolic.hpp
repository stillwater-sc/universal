#pragma once
// hyperbolic.hpp: hyperbolic functions for logarithmic floating-point
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// hyperbolic sine of an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> sinh(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::sinh(double(x)));
}

// hyperbolic cosine of an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> cosh(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::cosh(double(x)));
}

// hyperbolic tangent of an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> tanh(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::tanh(double(x)));
}

// hyperbolic cotangent of an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> atanh(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::atanh(double(x)));
}

// hyperbolic cosecant of an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> acosh(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::acosh(double(x)));
}

// hyperbolic secant of an angle of x radians
template<size_t nbits, size_t rbits, typename bt>
lns<nbits, rbits, bt> asinh(lns<nbits, rbits, bt> x) {
	return lns<nbits, rbits, bt>(std::asinh(double(x)));
}


}} // namespace sw::universal
