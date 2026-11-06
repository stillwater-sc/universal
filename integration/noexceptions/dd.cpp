// dd.cpp: dd in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/dd/dd.hpp>
#include "exercise.hpp"

int noexceptions_dd() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = dd;
	return noexceptions::exercise<Number>("dd");
}
