#pragma once
// iostream.hpp: stream insertion and extraction for lns
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the lns headers (#1334): the <iostream> half. lns_impl.hpp declares these
// as friend templates but does not define them, so the core needs only <iosfwd>.
#include <iostream>
#include <universal/internal/abstract/triple_io.hpp>   // operator<< / components() on triple<> (#1334)
#include <universal/number/lns/core.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
inline std::ostream& operator<<(std::ostream& ostr, const lns<nbits, rbits, bt, xtra...>& r) {
	ostr << double(r);
	return ostr;
}

template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
inline std::istream& operator>>(std::istream& istr, lns<nbits, rbits, bt, xtra...>& r) {
	// A failed extraction leaves the target alone, which is what the rest of the tree does
	// (bfloat16, dfloat, hfloat, ereal, einteger all return early on a failed read, and so
	// do e8m0 and microfloat) and what native extraction does on a failed sentry. It also
	// removes the read of an indeterminate value: on an already-failed stream the sentry
	// fails, num_get never runs, and d is never written (#1450).
	double d{};
	if (istr >> d) r = d;
	return istr;
}

}} // namespace sw::universal
