#pragma once
// iostream.hpp: stream insertion and extraction for cfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the cfloat headers (#1334): the <iostream> half. manipulators.hpp is the
// <iomanip> half -- everything that turns a cfloat into a std::string. Self-contained.
#include <cstddef>       // std::size_t
#include <ios>
#include <iostream>
#include <sstream>
#include <string>

#include <universal/internal/blocktriple/iostream.hpp>   // operator<< on blocktriple: cfloat's
                                                        // normalize()/to_triple() surface it, and the
                                                        // umbrella supplied it before the split (#1334)
#include <universal/number/cfloat/core.hpp>
#include <universal/number/cfloat/manipulators.hpp>   // parse(), to_decimal_fixpnt_string()

namespace sw { namespace universal {

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
inline std::ostream& operator<<(std::ostream& ostr, const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& v) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	constexpr unsigned cfbits = Cfloat::fbits;

	std::streamsize prec  = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff = ostr.flags();
	bool bFixed      = (ff & std::ios_base::fixed) == std::ios_base::fixed;
	bool bScientific = (ff & std::ios_base::scientific) == std::ios_base::scientific;
	bool bShowpos    = (ff & std::ios_base::showpos) != 0;
	bool bUppercase  = (ff & std::ios_base::uppercase) != 0;
	bool bInternal   = (ff & std::ios_base::internal) != 0;
	bool bLeft       = (ff & std::ios_base::left) != 0;
	char fillChar    = ostr.fill();

	if constexpr (cfbits == 0) {
		// degenerate cfloat with no fraction bits: fall back to double
		std::ostringstream oss;
		oss.precision(prec);
		if (bFixed) oss << std::fixed;
		if (bScientific) oss << std::scientific;
		if (bUppercase) oss << std::uppercase;
		if (bShowpos) oss << std::showpos;
		oss << static_cast<double>(v);
		std::string s = oss.str();
		if (width > 0 && s.length() < static_cast<std::size_t>(width)) {
			std::size_t pad = static_cast<std::size_t>(width) - s.length();
			if (bLeft) { s.append(pad, fillChar); }
			else { s.insert(0u, pad, fillChar); }
		}
		return ostr << s;
	} else {
		blocktriple<cfbits, BlockTripleOperator::REP, bt> a;
		v.normalize(a);
		return ostr << a.to_string(prec, width, bFixed, bScientific,
		                           bInternal, bLeft, bShowpos, bUppercase, fillChar);
	}
}

// parse a cfloat from a string in either cfloat hex format (nbits.esxHEXVALUEc)
// or a decimal floating-point representation

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
inline std::istream& operator>>(std::istream& istr, cfloat<nbits,es,bt,hasSubnormals,hasMaxExpValues,isSaturating>& v) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, v)) {
		std::cerr << "unable to parse -" << txt << "- into a cfloat value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

// encoding helpers

}} // namespace sw::universal
