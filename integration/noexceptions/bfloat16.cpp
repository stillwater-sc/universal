// bfloat16.cpp: bfloat16 in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/bfloat16/bfloat16.hpp>
#include "exercise.hpp"

int noexceptions_bfloat16() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = bfloat16;
	return noexceptions::exercise<Number>("bfloat16");
}
