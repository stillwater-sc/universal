// fixpnt.cpp: fixpnt in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/fixpnt/fixpnt.hpp>
#include "exercise.hpp"

int noexceptions_fixpnt() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = fixpnt<16, 8>;
	// operator>> parses decimal text since #1931
	return noexceptions::exercise_fdp<Number>("fixpnt", true) + noexceptions::exercise<Number, true, true>("fixpnt");
}
