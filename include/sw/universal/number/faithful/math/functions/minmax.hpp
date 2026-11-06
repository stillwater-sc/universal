#pragma once
// minmax.hpp: fmin and fmax for faithful
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// As std::fmin and std::fmax: a NaN operand is ignored when the other is a number (#1950). The
// order is that of the represented values, exactly, through faithful's comparisons.
#include <universal/number/faithful/core.hpp>

namespace sw { namespace universal {

template<typename FloatingPointType>
inline faithful<FloatingPointType> fmin(const faithful<FloatingPointType>& a, const faithful<FloatingPointType>& b) {
	if (a.isnan()) return b;
	if (b.isnan()) return a;
	return (b < a) ? b : a;
}

template<typename FloatingPointType>
inline faithful<FloatingPointType> fmax(const faithful<FloatingPointType>& a, const faithful<FloatingPointType>& b) {
	if (a.isnan()) return b;
	if (b.isnan()) return a;
	return (a < b) ? b : a;
}

}} // namespace sw::universal
