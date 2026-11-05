// cfloat.cpp: cfloat in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/cfloat/cfloat.hpp>
#include "exercise.hpp"

int noexceptions_cfloat() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = cfloat<16, 5, std::uint16_t, true, false, false>;
	return noexceptions::exercise_fdp<Number>("cfloat", true) + noexceptions::exercise<Number>("cfloat");
}
