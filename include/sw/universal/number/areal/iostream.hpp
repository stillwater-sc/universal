#pragma once
// iostream.hpp: stream insertion and extraction for areal
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the areal headers (#1334): the <iostream> half. manipulators.hpp is the
// <iomanip> half -- everything that turns an areal into a std::string. Self-contained.
//
// areal_impl.hpp keeps the friend DECLARATIONS for these two (operator>> reaches
// _fraction directly, so friendship is real here, unlike dbns); only the definitions
// live here, which is what lets the core get by with <iosfwd>.
//
// This header includes only core.hpp -- no cycle for #pragma once to mask (#1427).
#include <iostream>      // std::ostream, std::istream
#include <universal/number/areal/core.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned es, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const areal<nbits,es,bt>& v) {
	// TODO: make it a native conversion
	double d = double(v);
	bool ubit = v.at(0);
	if (ubit) {
		if (v.isnan()) {
			ostr << '[' << d << ']';
		}
		else {
			areal<nbits, es, bt> next(v);
			++next;
			double dnext = double(next);
			ostr << '(' << d << ", " << dnext << ')';
		}
	}
	else { // exact value
		ostr << '[' << d << ']';
	}
	return ostr;
}

template<unsigned nnbits, unsigned nes, typename nbt>
inline std::istream& operator>>(std::istream& istr, const areal<nnbits,nes,nbt>& v) {
	istr >> v._fraction;
	return istr;
}

}} // namespace sw::universal
