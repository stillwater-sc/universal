#pragma once
// next.hpp: nextafter and nexttoward for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Encodings are monotonic, so the adjacent value is the adjacent encoding.  These are
// exact for every configuration.  NaR in either argument gives NaR.

namespace sw { namespace universal {

// the next representable value after x in the direction of target; target if they are equal
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> nextafter(bposit<nbits, rs, es, bt> x, const bposit<nbits, rs, es, bt>& target) {
	if (x.isnar() || target.isnar()) return bposit<nbits, rs, es, bt>(SpecificValue::nar);
	if (x < target) ++x;
	else if (target < x) --x;
	return x;
}

// as nextafter, with the direction given by a long double
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> nexttoward(bposit<nbits, rs, es, bt> x, long double target) {
	if (x.isnar() || target != target) return bposit<nbits, rs, es, bt>(SpecificValue::nar);
	const bposit<nbits, rs, es, bt> t(target);
	if (x < t) ++x;
	else if (t < x) --x;
	else {
		// target rounds to x but need not equal it: step toward it all the same, but
		// not past maxpos or maxneg, whose neighbor encoding is NaR
		const long double v = static_cast<long double>(x);
		bposit<nbits, rs, es, bt> r(x);
		if (v < target) ++r;
		else if (target < v) --r;
		if (!r.isnar()) x = r;
	}
	return x;
}

}} // namespace sw::universal
