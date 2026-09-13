#pragma once
// iostream.hpp: stream insertion and extraction for bfloat16
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the bfloat16 headers (#1334): the <iostream> half. manipulators.hpp is
// the <iomanip> half -- everything that turns a bfloat16 into a std::string.
// Self-contained.
//
// The dependency runs iostream -> manipulators here, because operator>> extracts a
// token and hands it to parse(). manipulators.hpp does NOT include this header back:
// its string builders format from the bit pattern, never through operator<<, so there
// is no cycle for #pragma once to mask (the trap caught on #1427).
#include <ios>           // std::ios::failbit
#include <iostream>      // std::ostream, std::istream, std::cerr
#include <string>        // std::string

#include <universal/number/bfloat16/core.hpp>
#include <universal/number/bfloat16/manipulators.hpp>   // parse()

namespace sw { namespace universal {

// generate an bfloat16 format ASCII format
inline std::ostream& operator<<(std::ostream& ostr, bfloat16 bf) {
	return ostr << float(bf);
}

// read an ASCII bfloat16 format
inline std::istream& operator>>(std::istream& istr, bfloat16& p) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit set by >>.
		return istr;
	}
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a bfloat16 value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

}} // namespace sw::universal
