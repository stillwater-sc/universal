#pragma once
// next.hpp: nextafter for lindex (#1696)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The next encoding from x toward y: one step of the integer order, which is the value
// order.  The step saturates at maxpos and maxneg, as the type has no infinity to step to.

namespace sw { namespace universal {

template<unsigned nbits, unsigned lbits, typename bt>
lindex<nbits, lbits, bt> nextafter(const lindex<nbits, lbits, bt>& x, const lindex<nbits, lbits, bt>& y) {
	using L = lindex<nbits, lbits, bt>;
	if (x.isnar() || y.isnar()) return L(SpecificValue::nar);
	if (x == y) return y;
	L r(x);
	if (x < y) ++r; else --r;
	return r;
}

}}  // namespace sw::universal
