// areal.cpp: areal in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/areal/areal.hpp>
#include "exercise.hpp"

int noexceptions_areal() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = areal<16, 5, std::uint16_t>;
	return noexceptions::exercise<Number>("areal");
}
