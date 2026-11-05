// lns.cpp: lns in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/lns/lns.hpp>
#include "exercise.hpp"

int noexceptions_lns() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = lns<16, 8>;
	return noexceptions::exercise_fdp<Number>("lns", false) + noexceptions::exercise<Number>("lns", false);
}
