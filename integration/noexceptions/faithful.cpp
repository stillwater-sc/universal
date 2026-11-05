// faithful.cpp: faithful in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/faithful/faithful.hpp>
#include "exercise.hpp"

int noexceptions_faithful() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = faithful<double>;
	int fails = 0;
	Number a(1.5), b(0.75);
	a += b;
	a -= b;
	a *= b;
	Number n = -a;   // unary minus compiles since #1930
	if (n.high() != -a.high()) fails += fail("faithful", "negation");
	a /= Number(0.0);
	std::ostringstream ostr;
	ostr << a;
	if (ostr.str().empty()) fails += fail("faithful", "stream output");
	return fails;
}
