// nvblock.cpp: nvblock in a build with exceptions disabled
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/nvblock/nvblock.hpp>
#include "exercise.hpp"

int noexceptions_nvblock() {
	using namespace sw::universal;
	using noexceptions::fail;
	using Number = nvfp4;
	int fails = 0;
	float src[Number::blockSize], dst[Number::blockSize];
	for (unsigned i = 0; i < Number::blockSize; ++i) src[i] = 0.5f * float(i % 4);
	Number blk;
	blk.quantize(src);
	blk.dequantize(dst);
	if (dst[1] != 0.5f) fails += fail("nvblock", "quantize");
	return fails;
}
