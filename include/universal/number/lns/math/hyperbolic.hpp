#pragma once
// hyperbolic.hpp: hyperbolic functions for logarithmic floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// hyperbolic sine of an angle of x radians
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> sinh(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::sinh(double(x)));
}

// hyperbolic cosine of an angle of x radians
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> cosh(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::cosh(double(x)));
}

// hyperbolic tangent of an angle of x radians
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> tanh(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::tanh(double(x)));
}

// hyperbolic cotangent of an angle of x radians
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> atanh(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::atanh(double(x)));
}

// hyperbolic cosecant of an angle of x radians
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> acosh(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::acosh(double(x)));
}

// hyperbolic secant of an angle of x radians
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> asinh(lns<nbits, rbits, bt, xtra...> x) {
	return lns<nbits, rbits, bt, xtra...>(std::asinh(double(x)));
}


}} // namespace sw::universal
