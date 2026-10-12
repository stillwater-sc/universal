#pragma once
// minmax.hpp: min and max for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// NaR propagates: the minimum or maximum of NaR and anything is NaR.  (The encoding
// order alone would make NaR the minimum of everything and never the maximum.)

namespace sw { namespace universal {

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> min(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y) {
	if (x.isnar() || y.isnar()) return bposit<nbits, rs, es, bt>(SpecificValue::nar);
	return (y < x) ? y : x;
}

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> max(const bposit<nbits, rs, es, bt>& x, const bposit<nbits, rs, es, bt>& y) {
	if (x.isnar() || y.isnar()) return bposit<nbits, rs, es, bt>(SpecificValue::nar);
	return (x < y) ? y : x;
}

}} // namespace sw::universal
