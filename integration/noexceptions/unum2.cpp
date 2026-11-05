// unum2.cpp: unum2 in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/unum2/unum2.hpp>
#include "exercise.hpp"

int noexceptions_unum2() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = unum2<linear_5bit>;
	Number a(1), b(2);
	Number c = a + b;
	std::ostringstream ostr;
	ostr << c;
	return 0;
}
