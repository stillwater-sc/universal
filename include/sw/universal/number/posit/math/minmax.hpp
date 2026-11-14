#pragma once
// math_minmax.hpp: min/max functions for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// NaR propagates (#2071, user decision): min and max of NaR and anything are NaR. min(NaR, x) was
// NaR but max(NaR, x) was x, because NaR orders below every posit.

template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> min(posit<nbits,es,bt> x, posit<nbits, es, bt> y) {
	if (x.isnar() || y.isnar()) { x.setnar(); return x; }
	return (x < y) ? x : y;
}

template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> max(posit<nbits,es,bt> x, posit<nbits, es, bt> y) {
	if (x.isnar() || y.isnar()) { x.setnar(); return x; }
	return (x < y) ? y : x;
}

}} // namespace sw::universal
