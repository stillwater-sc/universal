#pragma once
// iostream.hpp: stream insertion and extraction for double-double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the dd headers (#1334): the <iostream> half. manipulators.hpp is the
// string-producing half. operator>> reports a parse failure on std::cerr, which is why
// <iostream> belongs at this layer and not in the core.
//
// to_string() and parse() are NOT here: they stay in the core because assign() calls
// parse(), and to_string() concatenates a std::string without ever touching a stream.
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <ios>           // std::ios_base flags, std::ios::failbit

#include <universal/number/dd/core.hpp>

namespace sw { namespace universal {

	// stream a vector<char> digit buffer; used by the bTraceDecimal* paths when a caller
	// wants them on an ostream rather than through the core's fprintf traces.
	inline std::ostream& operator<<(std::ostream& ostr, const std::vector<char>& s) {
		for (auto c : s) {
			ostr << c;
		}
		return ostr;
	}

////////////////////////  stream operators   /////////////////////////////////

// stream out a decimal floating-point representation of the double-double
inline std::ostream& operator<<(std::ostream& ostr, const dd& v) {
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

// stream in an ASCII decimal floating-point format and assign it to a double-double
inline std::istream& operator>>(std::istream& istr, dd& v) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit is set by >>.
		return istr;
	}
	if (!parse(txt, v)) {
		std::cerr << "unable to parse -" << txt << "- into a double-double value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

}} // namespace sw::universal
