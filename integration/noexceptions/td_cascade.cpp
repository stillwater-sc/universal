// td_cascade.cpp: td_cascade in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/td_cascade/td_cascade.hpp>
#include "exercise.hpp"

int noexceptions_td_cascade() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = td_cascade;
	return noexceptions::exercise<Number>("td_cascade");
}
