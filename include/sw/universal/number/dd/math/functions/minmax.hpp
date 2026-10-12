#pragma once
// minmax.hpp: minmax support for double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// the operand itself, every limb of it: these used to return a double rounding (#1667)
	inline dd min(const dd& x, const dd& y) { return (y < x) ? y : x; }

	inline dd max(const dd& x, const dd& y) { return (x < y) ? y : x; }

}} // namespace sw::universal
