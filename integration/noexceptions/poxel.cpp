// poxel.cpp: poxel in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/poxel/poxel.hpp>
#include "exercise.hpp"

int noexceptions_poxel() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = poxel<16, 2>;
	return noexceptions::exercise<Number>("poxel");
}
