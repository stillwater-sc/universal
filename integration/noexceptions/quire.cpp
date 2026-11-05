// quire.cpp: quire in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/quire/quire.hpp>
#include <universal/number/posit/posit.hpp>
#include "exercise.hpp"

int noexceptions_quire() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = posit<16, 1>;
	int fails = 0;
	quire<Number> q(0);
	q += quire_mul(Number(1.5), Number(0.75));
	q += quire_mul(Number(0.75), Number(1.5));
	Number r = quire_resolve(q);
	if (double(r) != 2.25) fails += fail("quire", "fused dot product");
	return fails;
}
