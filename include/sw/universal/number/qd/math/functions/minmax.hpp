#pragma once
// minmax.hpp: minmax support for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// the operand itself, every limb of it: the former QUADDOUBLE_NATIVE_MINMAX=0 path returned a
	// double rounding (#1667), so there is only this one now

	////////////////////////    logic functions   /////////////////////////////////

	inline qd max(const qd& a, const qd& b) { return (a > b) ? a : b; }

	inline qd max(const qd& a, const qd& b, const qd& c) { return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c); }

	inline qd min(const qd& a, const qd& b) { return (a < b) ? a : b; }

	inline qd min(const qd& a, const qd& b, const qd& c) { return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c); }


}} // namespace sw::universal
