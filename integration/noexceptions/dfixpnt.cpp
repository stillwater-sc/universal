// dfixpnt.cpp: dfixpnt in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/dfixpnt/dfixpnt.hpp>
#include "exercise.hpp"

int noexceptions_dfixpnt() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = dfixpnt<8, 3>;
	return noexceptions::exercise<Number>("dfixpnt");
}
