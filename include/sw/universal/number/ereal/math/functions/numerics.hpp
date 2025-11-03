#pragma once
// numerics.hpp: numeric support functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// frexp: break into normalized fraction and exponent
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion arithmetic
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> frexp(const ereal<maxlimbs>& x, int* exp) {
		double result = std::frexp(double(x), exp);
		return ereal<maxlimbs>(result);
	}

	// ldexp: multiply by power of 2
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using expansion arithmetic (efficient power-of-2 scaling)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> ldexp(const ereal<maxlimbs>& x, int exp) {
		return ereal<maxlimbs>(std::ldexp(double(x), exp));
	}

	// copysign: copy sign from one value to another
	// Phase 0: stub using double conversion
	// TODO Phase 1: implement using component sign manipulation
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> copysign(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		return ereal<maxlimbs>(std::copysign(double(x), double(y)));
	}

}} // namespace sw::universal
