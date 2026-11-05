// bisection.cpp: bisection in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/bisection/bisection.hpp>
#include "exercise.hpp"

int noexceptions_bisection() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = bisection_posit<16, 1>;
	return noexceptions::exercise<Number>("bisection", false);
}
