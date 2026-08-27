#pragma once
// iostream.hpp: stream insertion and extraction for the edecimal type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the edecimal headers (#1334): the <iostream> half. manipulators.hpp is the
// string-producing half. operator>> reports a parse failure on std::cerr, which is why
// this layer -- and not the core -- is where <iostream> belongs.
#include <iostream>
#include <sstream>
#include <string>
#include <ios>           // std::ios_base::fmtflags, std::ios::failbit

#include <universal/number/edecimal/core.hpp>
#include <universal/number/edecimal/manipulators.hpp>

namespace sw { namespace universal {

// generate an ASCII edecimal format and send to ostream
inline std::ostream& operator<<(std::ostream& ostr, const edecimal& d) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the integer into a string
	std::stringstream ss;

	//std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	if (d.isneg()) ss << '-';
	for (edecimal::const_reverse_iterator rit = d.rbegin(); rit != d.rend(); ++rit) {
		ss << (int)*rit;
	}
	return ostr << ss.str();
}

// read an ASCII edecimal format from an istream
inline std::istream& operator>>(std::istream& istr, edecimal& p) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit set by >>.
		return istr;
	}
	if (!p.parse(txt)) {
		std::cerr << "unable to parse -" << txt << "- into an edecimal value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

}} // namespace sw::universal
