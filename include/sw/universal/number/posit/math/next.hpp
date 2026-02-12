#pragma once
// math_next.hpp: nextafter/nexttoward functions for posits
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
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> nextafter(posit<nbits,es,bt> x, posit<nbits, es, bt> target) {
	if (x == target || x.isnar()) return x;
	if (target.isnar()) {
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
		
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> nexttoward(posit<nbits,es,bt> x, posit<256, 5> target) {
	posit<256, 5> _x(x);
	if (_x == target || x.isnar()) return x;
	if (target.isnar()) {
		if (x.isneg()) {
			--x;
		}
		else {
			++x;
		}
	}
	else {
		if (_x > target) {
			--x;
		}
		else {
			++x;
		}
	}
	return x;
}

}} // namespace sw::universal
