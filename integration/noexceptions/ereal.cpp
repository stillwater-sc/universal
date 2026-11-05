// ereal.cpp: ereal in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/ereal/ereal.hpp>
#include "exercise.hpp"

int noexceptions_ereal() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = ereal<>;
	return noexceptions::exercise<Number>("ereal");
}
