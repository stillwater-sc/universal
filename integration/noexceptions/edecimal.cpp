// edecimal.cpp: edecimal in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/edecimal/edecimal.hpp>
#include "exercise.hpp"

int noexceptions_edecimal() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = edecimal;
	return noexceptions::exercise_integer<Number>("edecimal");
}
