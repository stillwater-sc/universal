// zfpblock.cpp: zfpblock in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/zfpblock/zfpblock.hpp>
#include "exercise.hpp"

int noexceptions_zfpblock() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = zfpblock<float, 1>;
	int fails = 0;
	float src[4] = { 1.0f, 1.5f, 2.0f, 2.5f }, dst[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	Number blk;
	blk.compress(src, zfp_mode::fixed_rate, 16.0);
	blk.decompress(dst);
	if (dst[0] == 0.0f) fails += fail("zfpblock", "round trip");
	return fails;
}
