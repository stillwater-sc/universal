// dd_cascade.cpp: dd_cascade in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include "exercise.hpp"

int noexceptions_dd_cascade() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = dd_cascade;
	return noexceptions::exercise<Number>("dd_cascade");
}
