// e8m0.cpp: e8m0 in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/e8m0/e8m0.hpp>
#include "exercise.hpp"

int noexceptions_e8m0() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = e8m0;
	int fails = 0;
	Number a(2.0f), b(0.5f);
	if (float(a) != 2.0f || float(b) != 0.5f) fails += fail("e8m0", "conversion");
	std::ostringstream ostr;
	ostr << a << ' ' << b;
	if (ostr.str().empty()) fails += fail("e8m0", "stream output");
	return fails;
}
