// convert.cpp: convert in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/convert/convert.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include "exercise.hpp"

int noexceptions_convert() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = posit<16, 2>;
	int fails = 0;
	cfloat<32, 8, std::uint32_t, true, false, false> f(1.5f);
	Number p = universal_cast<Number>(f);
	if (double(p) != 1.5) fails += fail("convert", "posit from cfloat");
	return fails;
}
