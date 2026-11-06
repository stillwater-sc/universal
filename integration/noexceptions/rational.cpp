// rational.cpp: rational in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/rational/rational.hpp>
#include "exercise.hpp"

int noexceptions_rational() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = rational<16>;
	// operator>> parses decimal text since #1929
	return noexceptions::exercise<Number, true, true>("rational");
}
