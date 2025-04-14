#pragma once
// x_over_one_minus_x.hpp: generic implementation of the function x / (1 - x)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace function {

		template<typename Real>
		Real x_over_one_minus_x(const Real& x) {

			return x / (Real(1.0) - x);
		}

		template<typename Real>
		Real x_over_one_plus_x(const Real& x) {

			return x / (Real(1.0) + x);
		}
	}
}  // namespace sw::function

