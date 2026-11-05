// qd_cascade.cpp: qd_cascade in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include "exercise.hpp"

int noexceptions_qd_cascade() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = qd_cascade;
	return noexceptions::exercise<Number>("qd_cascade");
}
