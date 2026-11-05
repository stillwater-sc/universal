// microfloat.cpp: microfloat in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/microfloat/microfloat.hpp>
#include "exercise.hpp"

int noexceptions_microfloat() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = e4m3;
	return noexceptions::exercise<Number>("microfloat");
}
