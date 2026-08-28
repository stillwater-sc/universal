#pragma once
// iostream.hpp: stream insertion and extraction for dd_cascade
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the dd_cascade headers (#1334): the <iostream> half.
//
// operator<< was an in-class friend DEFINITION, which is what forced <iostream> into the
// arithmetic core. It is a plain free function here: it only calls the PUBLIC to_string()
// member, so it never needed friendship. operator>> reports a parse failure on std::cerr,
// which is why <iostream> belongs at this layer.
//
// to_string() and parse() stay in the CORE: assign(const std::string&) calls parse(), and
// to_string() concatenates a std::string without ever opening a stream.
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <ios>           // std::ios_base flags, std::ios::failbit

#include <universal/number/dd_cascade/core.hpp>

namespace sw { namespace universal {

inline std::ostream& operator<<(std::ostream& ostr, const dd_cascade& v) {
    std::ios_base::fmtflags fmt = ostr.flags();
    std::streamsize precision = ostr.precision();
    std::streamsize width = ostr.width();
    char fillChar = ostr.fill();
    bool showpos = fmt & std::ios_base::showpos;
    bool uppercase = fmt & std::ios_base::uppercase;
    bool fixed = fmt & std::ios_base::fixed;
    bool scientific = fmt & std::ios_base::scientific;
    bool internal = fmt & std::ios_base::internal;
    bool left = fmt & std::ios_base::left;
    return ostr << v.to_string(precision, width, fixed, scientific, internal, left, showpos, uppercase, fillChar);
}

// stream in an ASCII decimal floating-point format and assign it to a dd_cascade
inline std::istream& operator>>(std::istream& istr, dd_cascade& v) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit is set by >>.
		return istr;
	}
	if (!parse(txt, v)) {
		std::cerr << "unable to parse -" << txt << "- into a dd_cascade value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

}} // namespace sw::universal
