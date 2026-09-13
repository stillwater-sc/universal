#pragma once
// iostream.hpp: stream insertion and extraction for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the efloat headers (#1334, Phase 2 group 5b, #1455): the <iostream>
// half. manipulators.hpp is the <iomanip> half -- the string producers. Self-contained.
//
// This header includes only core.hpp: operator<< formats through to_string() and
// operator>> hands its token to parse(), and both of those are core. Neither half of
// the text layer depends on the other, so there is no cycle for #pragma once to
// mask (#1427).
#include <ios>        // std::ios_base::fmtflags, std::streamsize
#include <iostream>   // std::ostream / std::istream / std::cerr
#include <string>

#include <universal/number/efloat/core.hpp>

namespace sw { namespace universal {

template<unsigned nlimbs>
inline std::ostream& operator<<(std::ostream& ostr, const efloat<nlimbs>& rhs) {
	std::ios_base::fmtflags fmt = ostr.flags();
	std::streamsize precision   = ostr.precision();
	std::streamsize width       = ostr.width();
	char fillChar               = ostr.fill();
	bool showpos    = (fmt & std::ios_base::showpos)    != 0;
	bool uppercase  = (fmt & std::ios_base::uppercase)  != 0;
	bool fixed      = (fmt & std::ios_base::fixed)      != 0;
	bool scientific = (fmt & std::ios_base::scientific) != 0;
	bool internal   = (fmt & std::ios_base::internal)   != 0;
	bool left       = (fmt & std::ios_base::left)       != 0;
	return ostr << to_string(rhs, precision, width, fixed, scientific, internal, left, showpos, uppercase, fillChar);
}

// read an ASCII efloat format
template<unsigned nlimbs>
inline std::istream& operator>>(std::istream& istr, efloat<nlimbs>& p) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit set by >>.
		return istr;
	}
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into an efloat value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

}} // namespace sw::universal
