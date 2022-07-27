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
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> sinh(lns<nbits, rbits, behavior, bt> x) {
	return lns<nbits, rbits, behavior, bt>(std::sinh(double(x)));
}

// hyperbolic cosine of an angle of x radians
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> cosh(lns<nbits, rbits, behavior, bt> x) {
	return lns<nbits, rbits, behavior, bt>(std::cosh(double(x)));
}

// hyperbolic tangent of an angle of x radians
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> tanh(lns<nbits, rbits, behavior, bt> x) {
	return lns<nbits, rbits, behavior, bt>(std::tanh(double(x)));
}

// hyperbolic cotangent of an angle of x radians
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> atanh(lns<nbits, rbits, behavior, bt> x) {
	return lns<nbits, rbits, behavior, bt>(std::atanh(double(x)));
}

// hyperbolic cosecant of an angle of x radians
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> acosh(lns<nbits, rbits, behavior, bt> x) {
	return lns<nbits, rbits, behavior, bt>(std::acosh(double(x)));
}

// hyperbolic secant of an angle of x radians
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> asinh(lns<nbits, rbits, behavior, bt> x) {
	return lns<nbits, rbits, behavior, bt>(std::asinh(double(x)));
}


}} // namespace sw::universal
