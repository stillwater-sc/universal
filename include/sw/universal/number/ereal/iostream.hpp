#pragma once
// iostream.hpp: stream insertion and extraction for ereal
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the ereal headers (#1334, Phase 2 group 5b, #1455): the <iostream> half.
// Self-contained.
//
// This header includes only core.hpp: operator<< formats through the to_string()
// member and operator>> hands its token to parse(), both of which are core. The
// dependency between the text halves runs the other way -- manipulators.hpp's
// to_triple() streams an ereal, so it includes this header -- and including
// manipulators.hpp back here would be a cycle that #pragma once merely masks (#1427).
#include <ios>        // std::ios_base::fmtflags, std::streamsize
#include <iostream>   // std::ostream / std::istream / std::cerr
#include <string>

#include <universal/number/ereal/core.hpp>

namespace sw { namespace universal {

// generate an ereal format ASCII format
template<unsigned nlimbs>
inline std::ostream& operator<<(std::ostream& ostr, const ereal<nlimbs>& rhs) {
	std::ios_base::fmtflags fmt = ostr.flags();
	std::streamsize precision = ostr.precision();
	std::streamsize width = ostr.width();
	char fillChar = ostr.fill();
	bool showpos    = fmt & std::ios_base::showpos;
	bool uppercase  = fmt & std::ios_base::uppercase;
	bool fixed      = fmt & std::ios_base::fixed;
	bool scientific = fmt & std::ios_base::scientific;
	bool internal   = fmt & std::ios_base::internal;
	bool left       = fmt & std::ios_base::left;
	return ostr << rhs.to_string(precision, width, fixed, scientific,
	                              internal, left, showpos, uppercase, fillChar);
}

// read an ASCII ereal format
template<unsigned nlimbs>
inline std::istream& operator>>(std::istream& istr, ereal<nlimbs>& p) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit set by >>.
		return istr;
	}
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into an ereal value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

}} // namespace sw::universal
