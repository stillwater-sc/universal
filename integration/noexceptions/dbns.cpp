// dbns.cpp: dbns in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/dbns/fdp.hpp>
#include "exercise.hpp"

int noexceptions_dbns() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = dbns<8, 3>;
	return noexceptions::exercise_fdp<Number>("dbns", false) + noexceptions::exercise<Number>("dbns", false);
}
