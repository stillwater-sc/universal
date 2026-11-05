// positional.cpp: positional in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/positional/positional.hpp>
#include "exercise.hpp"

int noexceptions_positional() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = positional<8, 10>;
	return noexceptions::exercise_integer<Number>("positional");
}
