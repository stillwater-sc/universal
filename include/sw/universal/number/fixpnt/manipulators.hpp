#pragma once
// manipulators.hpp: definition of manipulation functions for fixed-point types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/color_print.hpp>   // Color / ColorCode, used by color_print()
#include <universal/number/fixpnt/core.hpp>
#include <string>        // std::string
#include <iomanip>       // std::setw
#include <cstddef>       // std::size_t
#include <cstdint>       // the fixed-width integer types
#include <sstream>
#include <universal/native/integer_type_tag.hpp>
#include <universal/number/fixpnt/fixpnt_fwd.hpp>

namespace sw { namespace universal {

// Generate a type tag for general fixpnt
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
std::string type_tag(const fixpnt<nbits, rbits, arithmetic, bt>& = {}) {
	std::stringstream s;
	s << "fixpnt<"
		<< std::setw(3) << nbits << ", "
		<< std::setw(3) << rbits << ", "
		<< (arithmetic ? "    Modulo, " : 
			             "Saturating, ")
		<< type_tag(bt{}) << '>';
	return s.str();
}

// Generate a type field descriptor for this integer
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline std::string type_field(const fixpnt<nbits, rbits, arithmetic, bt> & = {}) {
	std::stringstream s;
	s << "fields(i:" << (nbits - rbits) << "|r:" << rbits << ')';
	return s.str();
}

// return hex format
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline std::string to_hex(const fixpnt<nbits, rbits, arithmetic, bt>& v, bool nibbleMarker = false, bool hexPrefix = true) {
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::stringstream s;
	if (hexPrefix) s << "0x" << std::hex;
	int nrNibbles = int(1ull + ((nbits - 1ull) >> 2ull));
	for (int n = nrNibbles - 1; n >= 0; --n) {
		uint8_t nibble = v.nibble(unsigned(n));
		s << hexChar[nibble];
		if (nibbleMarker && n > 0 && (n % 4) == 0) s << '\'';
	}
	return s.str();
}


// Report the raw encoding, the decoded fields and the value of a fixpnt.
//
// It returned the literal "TBD" and ignored its argument (#1556). The shape is posit's --
// "raw: <bits> <fields> : value <v>" (posit/iostream.hpp) -- with the bits split at the
// radix point the way pretty_print() splits them.
//
// The bits are walked here rather than borrowed from to_binary(), and the value is
// double(v) rather than convert_to_decimal_string(): both of those live in iostream.hpp,
// and this layer does not depend on the stream layer (#1334). A fixpnt wider than a
// double's significand therefore reports a rounded value field, which is why the exact
// encoding is on the same line.
//
// printPrecision was added here so all of the number systems present one info_print
// signature; fixpnt and integer alone among the ten had none (#1556).
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline std::string info_print(const fixpnt<nbits, rbits, arithmetic, bt>& v, int printPrecision = 17) {
	std::stringstream s;
	s << "raw: 0b";
	if constexpr (nbits > rbits) {
		for (int i = static_cast<int>(nbits) - 1; i >= static_cast<int>(rbits); --i) {
			s << (v.at(static_cast<unsigned>(i)) ? '1' : '0');
		}
	}
	else {
		s << '0';
	}
	s << '.';
	for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
		s << (v.at(static_cast<unsigned>(i)) ? '1' : '0');
	}
	s << ' ' << (v.sign() ? "s1" : "s0")
	  << " i" << (nbits - rbits) << " r" << rbits
	  << (arithmetic ? " modulo" : " saturating")
	  << " : value " << std::setprecision(printPrecision) << double(v);
	return s.str();
}


template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline std::string pretty_print(const fixpnt<nbits, rbits, arithmetic, bt>& v, bool nibbleMarker = false) {
	std::stringstream s;

	// integer bits
	for (int i = static_cast<int>(nbits) - 1; i >= static_cast<int>(rbits); --i) {
		s << (v.at(static_cast<unsigned>(i)) ? '1' : '0');
		if (nibbleMarker && i > 0 && (i % 4) == 0) s << '\'';
	}
	// fraction bits
	if (rbits > 0) s << ':';
	for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
		s << (v.test(static_cast<size_t>(i)) ? '1' : '0');
		if (nibbleMarker && i > 0 && (i % 4) == 0) s << '\'';
	}
	return s.str();
}


template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline std::string color_print(const fixpnt<nbits, rbits, arithmetic, bt>& v, bool nibbleMarker = false) {

	std::stringstream s;

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color blue(ColorCode::FG_BLUE);
	Color magenta(ColorCode::FG_MAGENTA);
	Color cyan(ColorCode::FG_CYAN);
	Color white(ColorCode::FG_WHITE);
	Color def(ColorCode::FG_DEFAULT);

	// integer bits
	for (int i = static_cast<int>(nbits) - 1; i >= static_cast<int>(rbits); --i) {
		s << cyan << (v.at(static_cast<unsigned>(i)) ? '1' : '0');
		if (nibbleMarker && i > 0 && (i % 4) == 0) s << yellow << '\'';
	}
	// fraction bits
	for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
		s << magenta << (v.test(static_cast<size_t>(i)) ? '1' : '0');
		if (nibbleMarker && i > 0 && (i % 4) == 0) s << yellow << '\'';
	}

	s << def;
	return s.str();
}

}} // namespace sw::universal
