#pragma once
// iostream.hpp: stream insertion and extraction for hfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the hfloat headers (#1334): the <iostream> half. manipulators.hpp is the
// <iomanip> half -- everything that turns an hfloat into a std::string. Self-contained.
//
// This header includes only core.hpp. operator<< formats through the core member str()
// and operator>> hands its token to parse(), which is also core, so there is no
// dependency on manipulators.hpp and no cycle for #pragma once to mask (#1427).
#include <ios>           // std::streamsize, std::ios_base, std::ios::failbit
#include <iomanip>       // std::setw, std::setprecision
#include <iostream>      // std::ostream, std::istream, std::cerr
#include <sstream>       // std::stringstream in operator<<
#include <string>        // std::string

#include <universal/number/hfloat/core.hpp>

namespace sw { namespace universal {

////////////////////////  stream operators   /////////////////////////////////

template<unsigned ndigits, unsigned es, typename BlockType>
inline std::ostream& operator<<(std::ostream& ostr, const hfloat<ndigits, es, BlockType>& i) {
	std::stringstream ss;
	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	ss << std::setw(width) << std::setprecision(prec) << i.str(size_t(prec));
	return ostr << ss.str();
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline std::istream& operator>>(std::istream& istr, hfloat<ndigits, es, BlockType>& p) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit set by >>.
		return istr;
	}
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into an hfloat value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

}} // namespace sw::universal
