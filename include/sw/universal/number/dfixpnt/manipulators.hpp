#pragma once
// manipulators.hpp: definitions of helper functions for decimal dfixpnt type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>
#include <iomanip>

namespace sw { namespace universal {

// Generate a type tag for this dfixpnt
template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
std::string type_tag(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& = {}) {
	std::stringstream s;
	s << "dfixpnt<"
		<< std::setw(3) << ndigits << ", "
		<< std::setw(3) << radix << ", "
		<< "BCD, "
		<< (arithmetic ? "    Modulo, " : "Saturating, ")
		<< typeid(bt).name() << '>';
	return s.str();
}

// Generate a type field descriptor
template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
std::string type_field(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& = {}) {
	std::stringstream s;
	s << "fields(i:" << (ndigits - radix) << "|f:" << radix << ')';
	return s.str();
}

// to_binary: show underlying BCD bit pattern
template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
std::string to_binary(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& v, bool nibbleMarker = false) {
	std::stringstream s;
	// sign
	s << (v.sign() ? '1' : '0') << '.';
	// integer digits (BCD nibbles from MSD to LSD)
	for (int i = static_cast<int>(ndigits) - 1; i >= static_cast<int>(radix); --i) {
		unsigned d = v.digit(static_cast<unsigned>(i));
		// show 4-bit BCD nibble
		s << ((d >> 3) & 1) << ((d >> 2) & 1) << ((d >> 1) & 1) << (d & 1);
		if (nibbleMarker && i > static_cast<int>(radix)) s << '\'';
	}
	if (radix > 0) {
		s << '.';
		for (int i = static_cast<int>(radix) - 1; i >= 0; --i) {
			unsigned d = v.digit(static_cast<unsigned>(i));
			s << ((d >> 3) & 1) << ((d >> 2) & 1) << ((d >> 1) & 1) << (d & 1);
			if (nibbleMarker && i > 0) s << '\'';
		}
	}
	return s.str();
}

// color_print with ANSI escape codes
template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
std::string color_print(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& v, bool nibbleMarker = false) {
	std::stringstream s;
	// sign in red
	s << "\033[31m" << (v.sign() ? '1' : '0') << "\033[0m" << '.';
	// integer digits in cyan
	s << "\033[36m";
	for (int i = static_cast<int>(ndigits) - 1; i >= static_cast<int>(radix); --i) {
		s << v.digit(static_cast<unsigned>(i));
		if (nibbleMarker && i > static_cast<int>(radix)) s << '\'';
	}
	s << "\033[0m";
	if (radix > 0) {
		s << '.';
		// fraction digits in magenta
		s << "\033[35m";
		for (int i = static_cast<int>(radix) - 1; i >= 0; --i) {
			s << v.digit(static_cast<unsigned>(i));
			if (nibbleMarker && i > 0) s << '\'';
		}
		s << "\033[0m";
	}
	return s.str();
}

}} // namespace sw::universal
