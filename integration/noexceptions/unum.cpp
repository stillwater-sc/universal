// unum.cpp: unum in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/unum/unum.hpp>
#include "exercise.hpp"

int noexceptions_unum() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = unum<2, 2>;
	Number a;
	a.setzero();
	std::ostringstream ostr;
	ostr << a;
	return 0;
}
