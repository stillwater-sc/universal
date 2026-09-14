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
// areal_impl.hpp keeps the friend DECLARATION of operator<<; only the definition lives
// here, which is what lets the core get by with <iosfwd>. operator>> is a plain function
// over parse(), which the core defines (#1454).
//
// This header includes only core.hpp -- no cycle for #pragma once to mask (#1427).
#include <iostream>      // std::ostream, std::istream
#include <string>        // operator>> reads a token
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

// read an areal in any form parse() accepts. The exact form [d] and the uncertain form
// (d, dnext) are read through their closing bracket, since operator<< puts a space inside the
// interval; anything else is one whitespace-delimited token. On a parse failure: a diagnostic
// on std::cerr and failbit on the stream, as cfloat, bfloat16 and integer do.
template<unsigned nbits, unsigned es, typename bt>
inline std::istream& operator>>(std::istream& istr, areal<nbits, es, bt>& v) {
	std::string txt;
	istr >> std::ws;
	const auto first = istr.peek();
	if (first == '[' || first == '(') {
		const char close = (first == '[') ? ']' : ')';
		std::getline(istr, txt, close);
		if (!istr) return istr;  // no closing bracket before the end of the stream
		txt.push_back(close);
	}
	else {
		istr >> txt;
		if (!istr) return istr;
	}
	if (!parse(txt, v)) {
		std::cerr << "unable to parse -" << txt << "- into an areal value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

}} // namespace sw::universal
