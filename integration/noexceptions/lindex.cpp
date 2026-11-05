// lindex.cpp: lindex in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/lindex/lindex.hpp>
#include "exercise.hpp"

int noexceptions_lindex() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = lindex<16, 3>;
	return noexceptions::exercise<Number>("lindex", false);
}
