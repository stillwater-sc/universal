#pragma once
// iostream.hpp: stream insertion and extraction for quad-double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// NOTE on layer order: this header does NOT include manipulators.hpp, and must not.
// operator<< calls only the to_string() member and operator>> only parse(), both core,
// so the dependency runs the OTHER way: qd's string builders format qd values THROUGH
// operator<<, so manipulators.hpp includes THIS header. Keeping the include out here is
// what stops that being a cycle.
//
// Layer 2b of the qd headers (#1334): the <iostream> half. manipulators.hpp is the
// string-producing half. operator>> reports a parse failure on std::cerr, which is why
// <iostream> belongs at this layer and not in the core.
//
// to_string() and parse() are NOT here: they stay in the core, because assign() calls
// parse() and to_string() concatenates a std::string without touching a stream.
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <ios>           // std::ios_base flags, std::ios::failbit

#include <universal/number/qd/core.hpp>
#include <universal/number/qd/manipulators.hpp>

namespace sw { namespace universal {


// stream out a decimal floating-point representation of the quad-double
inline std::ostream& operator<<(std::ostream& ostr, const qd& v) {
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

// stream in an ASCII decimal floating-point format and assign it to a quad-double
inline std::istream& operator>>(std::istream& istr, qd& v) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit is set by >>.
		return istr;
	}
	if (!parse(txt, v)) {
		std::cerr << "unable to parse -" << txt << "- into a quad-double value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

}} // namespace sw::universal
