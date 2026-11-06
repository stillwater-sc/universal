// takum.cpp: takum in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/takum/takum.hpp>
#include "exercise.hpp"

int noexceptions_takum() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = takum<16>;
	return noexceptions::exercise<Number>("takum", false);
}
