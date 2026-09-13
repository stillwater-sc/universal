#pragma once
// iostream.hpp: stream insertion and extraction for dfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the dfloat headers (#1334): the <iostream> half. manipulators.hpp is the
// <iomanip> half -- everything that turns a dfloat into a std::string. Self-contained.
//
// This header includes only core.hpp. operator<< formats through the dfloat's own
// str(), which is a core member, and operator>> hands its token to parse(), which is
// also core -- so unlike bfloat16 there is no dependency on manipulators.hpp at all,
// and certainly no cycle for #pragma once to mask (the trap caught on #1427).
#include <ios>           // std::streamsize, std::ios_base, std::ios::failbit
#include <iostream>      // std::ostream, std::istream, std::cerr
#include <string>        // std::string

#include <universal/number/dfloat/core.hpp>

namespace sw { namespace universal {

////////////////////////  stream operators   /////////////////////////////////

// generate a dfloat format ASCII format
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline std::ostream& operator<<(std::ostream& ostr, const dfloat<ndigits, es, Encoding, BlockType>& i) {
	using Dfloat = dfloat<ndigits, es, Encoding, BlockType>;
	using FmtMode = typename Dfloat::FmtMode;

	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff = ostr.flags();

	// Map iostream format flags to dfloat FmtMode
	FmtMode mode = FmtMode::automatic;
	bool scientific = (ff & std::ios_base::scientific) == std::ios_base::scientific;
	bool fixed      = (ff & std::ios_base::fixed) == std::ios_base::fixed;
	if (scientific && !fixed) mode = FmtMode::scientific;
	else if (fixed && !scientific) mode = FmtMode::fixed;

	// Default to ndigits precision so all stored digits are shown.
	// The iostream default precision is 6, which would silently truncate
	// exact decimal digits. Only use the stream precision when the user
	// has explicitly set scientific or fixed mode.
	size_t effective_prec = (scientific || fixed)
		? static_cast<size_t>(prec)
		: 0;  // 0 tells str() to use ndigits

	std::string representation = i.str(effective_prec, mode);

	// Handle setw and alignment
	std::streamsize repWidth = static_cast<std::streamsize>(representation.size());
	if (width > repWidth) {
		std::streamsize diff = width - repWidth;
		char fill = ostr.fill();
		if ((ff & std::ios_base::left) == std::ios_base::left) {
			representation.append(static_cast<size_t>(diff), fill);
		}
		else {
			representation.insert(0, static_cast<size_t>(diff), fill);
		}
	}

	return ostr << representation;
}

// read an ASCII dfloat format
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline std::istream& operator>>(std::istream& istr, dfloat<ndigits, es, Encoding, BlockType>& p) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit set by >>.
		return istr;
	}
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a dfloat value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

}} // namespace sw::universal
