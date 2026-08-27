#pragma once
// iostream.hpp: stream insertion and extraction for erational
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the erational headers (#1334): the <iostream> half. operator>> reports a
// parse failure on std::cerr, which is why this layer -- and not the core -- is where
// <iostream> belongs.
#include <iostream>
#include <sstream>
#include <string>
#include <ios>           // std::ios::failbit

#include <universal/number/edecimal/iostream.hpp>   // operator<<(ostream, edecimal): erational's own
                                                     // operator<< streams its numerator/denominator through it
#include <universal/number/erational/core.hpp>
#include <universal/number/erational/manipulators.hpp>

namespace sw { namespace universal {

// generate an ASCII erational format and send to ostream
inline std::ostream& operator<<(std::ostream& ostr, const erational& d) {
	// make certain that setw and left/right operators work properly
	std::stringstream str;
	if (d.isneg()) str << '-';
	str << d.numerator << '/' << d.denominator;
	return ostr << str.str();
}

// read an ASCII erational format from an istream
inline std::istream& operator>>(std::istream& istr, erational& p) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit set by >>.
		return istr;
	}
	if (!p.parse(txt)) {
		std::cerr << "unable to parse -" << txt << "- into an erational value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

}} // namespace sw::universal
