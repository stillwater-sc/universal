// bposit.cpp: bposit in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/bposit/bposit.hpp>
#include "exercise.hpp"

int noexceptions_bposit() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = bposit<16, 4, 1>;
	// operator>> parses with strtold, so it must accept decimal text (#1925)
	return noexceptions::exercise_fdp<Number>("bposit", true) + noexceptions::exercise<Number, true, true>("bposit");
}
