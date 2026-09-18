#pragma once
// iostream.hpp: stream insertion and extraction for takum and takum_log
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the takum headers (#1334). The impl headers declare these as friends but
// do not define them, so the core needs only <iosfwd>.
#include <string>        // std::string
#include <iostream>
#include <universal/number/takum/core.hpp>

namespace sw { namespace universal {

////////////////////// operators
template<unsigned nnbits, unsigned nrbits, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const takum<nnbits, nrbits, nbt>& v) {
	ostr << double(v);
	return ostr;
}

template<unsigned nnbits, unsigned nrbits, typename nbt>
inline std::istream& operator>>(std::istream& istr, takum<nnbits, nrbits, nbt>& v) {
	// A failed extraction leaves the target alone, which is what the rest of the tree does
	// (bfloat16, dfloat, hfloat, ereal, einteger all return early on a failed read, and so
	// do e8m0 and microfloat) and what native extraction does on a failed sentry. It also
	// removes the read of an indeterminate value: on an already-failed stream the sentry
	// fails, num_get never runs, and d is never written (#1450).
	double d{};
	if (istr >> d) v = d;
	return istr;
}
////////////////////// operators
template<unsigned nn, unsigned nr, typename nb>
inline std::ostream& operator<<(std::ostream& ostr, const takum_log<nn, nr, nb>& v) { ostr << double(v); return ostr; }
template<unsigned nn, unsigned nr, typename nb>
inline std::istream&
operator>>(std::istream& istr, takum_log<nn, nr, nb>& v) {
	double d; istr >> d; v = d; return istr;
}

}} // namespace sw::universal
