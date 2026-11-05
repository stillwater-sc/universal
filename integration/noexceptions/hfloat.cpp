// hfloat.cpp: hfloat in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/hfloat/hfloat.hpp>
#include "exercise.hpp"

int noexceptions_hfloat() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = hfloat<6, 7>;
	return noexceptions::exercise<Number>("hfloat");
}
