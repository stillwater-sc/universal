// integer.cpp: integer in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/integer/integer.hpp>
#include "exercise.hpp"

int noexceptions_integer() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = integer<32>;
	return noexceptions::exercise_integer<Number>("integer");
}
