#pragma once
// hyperbolic.hpp: hyperbolic functions for fixed-points
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// hyperbolic sine of an angle of x radians
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> sinh(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::sinh(double(x)));
}

// hyperbolic cosine of an angle of x radians
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> cosh(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::cosh(double(x)));
}

// hyperbolic tangent of an angle of x radians
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> tanh(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::tanh(double(x)));
}

// hyperbolic cotangent of an angle of x radians
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> atanh(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::atanh(double(x)));
}

// hyperbolic cosecant of an angle of x radians
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> acosh(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::acosh(double(x)));
}

// hyperbolic secant of an angle of x radians
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> asinh(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return fixpnt<nbits, rbits, arithmetic, bt>(std::asinh(double(x)));
}


}} // namespace sw::universal
