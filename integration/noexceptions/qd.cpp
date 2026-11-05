// qd.cpp: qd in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/qd/qd.hpp>
#include "exercise.hpp"

int noexceptions_qd() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = qd;
	return noexceptions::exercise<Number>("qd");
}
