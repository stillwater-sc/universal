// valid.cpp: valid in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/valid/valid.hpp>
#include "exercise.hpp"

int noexceptions_valid() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = valid<16, 1>;
	int fails = 0;
	Number a(1.5), b(0.75);
	Number c = a + b;
	std::ostringstream ostr;
	ostr << c;
	if (ostr.str().empty()) fails += fail("valid", "stream output");
	return fails;
}
