// erational.cpp: erational in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/erational/erational.hpp>
#include "exercise.hpp"

int noexceptions_erational() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = erational;
	return noexceptions::exercise<Number>("erational");
}
