#pragma once
// next.hpp: nextafter/nexttoward functions for cfloat
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

	If x is the largest finite value representable in the type,
	and the result is infinite or not representable, an overflow range error occurs.

	If an overflow range error occurs:
	- And math_errhandling has MATH_ERRNO set: the global variable errno is set to ERANGE.
	- And math_errhandling has MATH_ERREXCEPT set: FE_OVERFLOW is raised.
	*/
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> nextafter(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> target) {
	if (x == target) return target;
	if (target.isnan()) {
		if (x.isneg()) {
			--x;
		}
		else {
			++x;
		}
	}
	else {
		if (x > target) {
			--x;
		}
		else {
			++x;
		}
	}
	return x;
}
		
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> nexttoward(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<128, 15, bt, hasSubnormals, hasMaxExpValues, isSaturating> target) {
	cfloat<128, 15, bt, hasSubnormals, hasMaxExpValues, isSaturating> _x(x);
	if (_x == target) return x;
	if (target.isnan()) {
		if (_x.isneg()) {
			--_x;
		}
		else {
			++_x;
		}
	}
	else {
		if (_x > target) {
			--_x;
		}
		else {
			++_x;
		}
	}
	x = _x;
	return x;
}

}} // namespace sw::universal
