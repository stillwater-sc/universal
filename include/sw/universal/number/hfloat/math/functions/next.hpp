#pragma once
// next.hpp: nextafter functions for IBM System/360 hexadecimal floating-point hfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

/*
Parameters
	x  Base value.
	t  Value toward which the return value is approximated.
If both parameters compare equal, the function returns t.

Return Value
	The next representable value after x in the direction of t.

	hfloat has no infinity or NaN encodings, so overflow
	saturates to maxpos/maxneg.
*/
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> nextafter(hfloat<ndigits, es, bt> x, hfloat<ndigits, es, bt> target) {
	if (x == target) return target;
	if (x > target) {
		--x;
	}
	else {
		++x;
	}
	return x;
}

}} // namespace sw::universal
