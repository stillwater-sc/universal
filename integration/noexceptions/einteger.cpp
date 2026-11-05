// einteger.cpp: einteger in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/einteger/einteger.hpp>
#include "exercise.hpp"

int noexceptions_einteger() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = einteger<std::uint32_t>;
	return noexceptions::exercise_integer<Number>("einteger");
}
