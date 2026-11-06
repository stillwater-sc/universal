// dfloat.cpp: dfloat in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/dfloat/dfloat.hpp>
#include "exercise.hpp"

int noexceptions_dfloat() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = dfloat<7, 6>;
	return noexceptions::exercise<Number>("dfloat");
}
