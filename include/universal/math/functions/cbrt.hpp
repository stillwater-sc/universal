#pragma once
// cbrt.hpp: generic implementation of a cubic root of a Real
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace function {

		template<typename Real>
		Real cbrt(const Real& x) {
			assert(x >= Real(0));
			return std::cbrt(x);
		}

	}
}  // namespace sw::function

