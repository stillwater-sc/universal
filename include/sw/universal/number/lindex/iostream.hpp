#pragma once
// iostream.hpp: stream insertion and extraction for lindex
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the lindex headers (#1334).
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <universal/number/lindex/core.hpp>

namespace sw { namespace universal {

// A value a double holds prints as that double, with the stream's flags.  Every value
// from psi(DBL_MAX) = 4.632 up is past it, and prints as phi(psi): maxpos of lindex<16,3>
// is phi(7.999755859375), not inf, because the type has no infinity.
template<unsigned nbits, unsigned lbits, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const lindex<nbits, lbits, bt>& v) {
	if (v.isnar()) return ostr << "nar";
	const double d = double(v);
	if (std::isfinite(d)) return ostr << d;
	std::ostringstream s;
	s.flags(ostr.flags());
	s.precision(ostr.precision());
	s << (v.sign() ? "-" : "") << "phi(" << v.psi() << ')';
	return ostr << s.str();
}

template<unsigned nbits, unsigned lbits, typename bt>
inline std::istream& operator>>(std::istream& istr, lindex<nbits, lbits, bt>& v) {
	// a failed extraction leaves the target alone (#1450)
	double d{};
	if (istr >> d) v = d;
	return istr;
}

}} // namespace sw::universal
