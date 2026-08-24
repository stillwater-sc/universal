#pragma once
// iostream.hpp: stream insertion and extraction for posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The <iostream> half of the posit text layer (#1334). manipulators.hpp is the
// <iomanip> half -- everything that turns a posit into a std::string, which is
// where the library already keeps to_binary/to_hex/color_print. This header holds
// only what genuinely needs the stream types: operator<< and operator>>.
//
// posit.hpp includes both, so existing code is unaffected.
#include <ios>
#include <sstream>
#include <string>
#include <iostream>
#include <universal/number/posit/posit_impl.hpp>
#include <universal/number/posit/manipulators.hpp>   // to_hex/parse used below

namespace sw { namespace universal {

// generate a posit format ASCII format nbits.esxNN...NNp
template<unsigned nbits, unsigned es, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const posit<nbits, es, bt>& p) {
#if POSIT_ERROR_FREE_IO_FORMAT
	std::stringstream ss;
	ss << nbits << '.' << es << 'x' << to_hex(p.bits()) << 'p';
	return ostr << ss.str();
#else
	std::ios_base::fmtflags fmt = ostr.flags();
	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	char fillChar = ostr.fill();
	bool bShowpos    = fmt & std::ios_base::showpos;
	bool bUppercase  = fmt & std::ios_base::uppercase;
	bool bFixed      = fmt & std::ios_base::fixed;
	bool bScientific = fmt & std::ios_base::scientific;
	bool bInternal   = fmt & std::ios_base::internal;
	bool bLeft       = fmt & std::ios_base::left;

	if (p.isnar()) {
		std::string s = bUppercase ? "NAR" : "nar";
		if (width > 0 && s.length() < static_cast<size_t>(width)) {
			size_t pad = static_cast<size_t>(width) - s.length();
			if (bLeft) { s.append(pad, fillChar); }
			else { s.insert(static_cast<std::string::size_type>(0), pad, fillChar); }
		}
		return ostr << s;
	}

	constexpr unsigned pfbits = posit<nbits, es, bt>::fbits;
	if constexpr (pfbits == 0) {
		// degenerate posit with no fraction bits: format via double
		std::ostringstream oss;
		oss.precision(prec);
		if (bFixed) oss << std::fixed;
		if (bScientific) oss << std::scientific;
		if (bUppercase) oss << std::uppercase;
		if (bShowpos) oss << std::showpos;
		oss << static_cast<double>(p);
		std::string s = oss.str();
		if (width > 0 && s.length() < static_cast<size_t>(width)) {
			size_t pad = static_cast<size_t>(width) - s.length();
			if (bInternal) {
				bool hasSign = !s.empty() && (s[0] == '-' || s[0] == '+');
				s.insert(hasSign ? 1u : 0u, pad, fillChar);
			} else if (bLeft) { s.append(pad, fillChar); }
			else { s.insert(0u, pad, fillChar); }
		}
		return ostr << s;
	} else {
		auto v = p.template to_value<BlockTripleOperator::REP>();
		return ostr << v.to_string(prec, width, bFixed, bScientific,
		                            bInternal, bLeft, bShowpos, bUppercase, fillChar);
	}
#endif
}

// parse a posit from a string in either posit hex format (nbits.esxHEXVALUEp)


// read an ASCII float or posit format: nbits.esxNN...NNp, for example: 32.2x80000000p
template<unsigned nbits, unsigned es, typename bt>
inline std::istream& operator>> (std::istream& istr, posit<nbits, es, bt>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a posit value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

}} // namespace sw::universal
