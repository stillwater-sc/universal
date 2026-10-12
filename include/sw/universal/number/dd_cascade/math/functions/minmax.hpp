#pragma once
// minmax.hpp: minmax support for double-double cascade floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// the operand itself, every limb of it: these used to return a double rounding (#1667)
	inline dd_cascade min(const dd_cascade& x, const dd_cascade& y) { return (y < x) ? y : x; }

	inline dd_cascade max(const dd_cascade& x, const dd_cascade& y) { return (x < y) ? y : x; }

}} // namespace sw::universal
