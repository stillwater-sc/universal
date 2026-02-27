#pragma once
// hypot.hpp: hypotenuse functions for IBM System/360 hexadecimal floating-point hfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// hypotenuse function: sqrt(x^2 + y^2) without undue overflow or underflow
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> hypot(hfloat<ndigits, es, bt> x, hfloat<ndigits, es, bt> y) {
	return hfloat<ndigits, es, bt>(std::hypot(double(x), double(y)));
}

}} // namespace sw::universal
