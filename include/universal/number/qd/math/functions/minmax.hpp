#pragma once
// minmax.hpp: minmax support for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

#if !QUADDOUBLE_NATIVE_MINMAX

	inline qd min(const qd& x, const qd& y) {
		return qd(std::min(double(x), double(y)));
	}

	inline qd max(const qd& x, const qd& y) {
		return qd(std::max(double(x), double(y)));
	}

#else 
	////////////////////////    logic functions   /////////////////////////////////

	inline qd max(const qd& a, const qd& b) { return (a > b) ? a : b; }

	inline qd max(const qd& a, const qd& b, const qd& c) { return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c); }

	inline qd min(const qd& a, const qd& b) { return (a < b) ? a : b; }

	inline qd min(const qd& a, const qd& b, const qd& c) { return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c); }

#endif

}} // namespace sw::universal
