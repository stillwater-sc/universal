// posit.cpp: posit in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit/posit.hpp>
#include "exercise.hpp"

int noexceptions_posit() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = posit<16, 2>;
	return noexceptions::exercise_fdp<Number>("posit", true) + noexceptions::exercise<Number>("posit");
}
