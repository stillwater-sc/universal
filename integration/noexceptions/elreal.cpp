// elreal.cpp: elreal in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/elreal/elreal.hpp>
#include "exercise.hpp"

int noexceptions_elreal() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = elreal<double>;
	return noexceptions::exercise<Number>("elreal");
}
