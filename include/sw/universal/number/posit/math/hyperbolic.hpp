#pragma once
// hyperbolic.hpp: hyperbolic functions for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// hyperbolic sine of an angle of x radians
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> sinh(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::sinh(double(x)));
}

// hyperbolic cosine of an angle of x radians
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> cosh(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::cosh(double(x)));
}

// hyperbolic tangent of an angle of x radians
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> tanh(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::tanh(double(x)));
}

// hyperbolic cotangent of an angle of x radians
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> atanh(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::atanh(double(x)));
}

// hyperbolic cosecant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> acosh(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::acosh(double(x)));
}

// hyperbolic secant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> asinh(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::asinh(double(x)));
}


}} // namespace sw::universal
