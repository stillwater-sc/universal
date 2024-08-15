#pragma once
// minmax.hpp: minmax support for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	qd min(qd x, qd y) {
		return qd(std::min(double(x), double(y)));
	}

	qd max(qd x, qd y) {
		return qd(std::max(double(x), double(y)));
	}

}} // namespace sw::universal