#pragma once
// next.hpp: nextafter/nexttoward functions for adaptive precision decimal rationals
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

erational nextafter(erational x, erational target) {
	if (x == target) return target;

		if (x > target) {
			--x;
		}
		else {
			++x;
		}
	
	return x;
}
		
erational nexttoward(erational x, erational target) {
	if (x == target) return x;

		if (x > target) {
			--x;
		}
		else {
			++x;
		}
	
	return x;
}

}} // namespace sw::universal
