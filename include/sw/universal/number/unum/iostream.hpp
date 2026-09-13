#pragma once
// iostream.hpp: stream insertion and extraction for unum and ubound
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the unum headers (#1334, Phase 2 group 5c, #1467): the <iostream> half.
// Self-contained.
//
// The dependency runs iostream -> manipulators: operator>> hands its token to parse(),
// which reads it through an istringstream and so lives in manipulators.hpp.
// manipulators.hpp does not include this header back.
#include <ios>        // std::ios::failbit
#include <iostream>   // std::ostream, std::istream, std::cerr
#include <string>

#include <universal/number/unum/core.hpp>
#include <universal/number/unum/manipulators.hpp>   // parse()

namespace sw { namespace universal {

////////////////////// IO operators

template<unsigned esizesize, unsigned fsizesize, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const unum<esizesize, fsizesize, bt>& v) {
	if (v.isnan()) {
		ostr << "NaN";
	}
	else if (v.iszero()) {
		ostr << 0;
	}
	else {
		// use decoded double value
		double d = v.to_double();
		ostr << d;
	}
	return ostr;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
inline std::istream& operator>>(std::istream& istr, unum<esizesize, fsizesize, bt>& v) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit set by >>.
		return istr;
	}
	if (!parse(txt, v)) {
		std::cerr << "unable to parse -" << txt << "- into a unum value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

// ubound: [lower, upper], or [x] for a point
template<unsigned esizesize, unsigned fsizesize, typename bt>
std::ostream& operator<<(std::ostream& ostr, const ubound<esizesize, fsizesize, bt>& ub) {
	if (ub.isnan()) {
		ostr << "[NaN]";
	}
	else if (ub.ispoint()) {
		ostr << '[' << ub.lower() << ']';
	}
	else {
		ostr << '[' << ub.lower() << ", " << ub.upper() << ']';
	}
	return ostr;
}

}} // namespace sw::universal
