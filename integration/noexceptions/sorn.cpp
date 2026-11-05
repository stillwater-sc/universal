// sorn.cpp: sorn in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/sorn/sorn.hpp>
#include "exercise.hpp"

int noexceptions_sorn() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = sorn<0, 4, 8>;
	Number a(1.5), b(0.75);
	Number c = a + b;
	std::ostringstream ostr;
	ostr << c;
	return 0;
}
