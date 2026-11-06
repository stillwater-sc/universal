// interval.cpp: interval in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/interval/interval.hpp>
#include "exercise.hpp"

int noexceptions_interval() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = interval<float>;
	int fails = 0;
	Number a(1.0f, 2.0f), b(0.5f, 0.75f), zero(0.0f);
	Number c = a + b;
	c = c * a;
	c = c / b;
	Number q = a / zero;
	std::ostringstream ostr;
	ostr << c << ' ' << q;
	if (ostr.str().empty()) fails += fail("interval", "stream output");
	return fails;
}
