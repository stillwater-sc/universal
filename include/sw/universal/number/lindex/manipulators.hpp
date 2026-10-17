#pragma once
// manipulators.hpp: definitions of helper functions for lindex number manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the lindex headers (#1334): the string producers.
#include <cstdint>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <type_traits>
#include <universal/number/lindex/core.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

// Generate a type tag for this lindex
template<typename LindexType,
	std::enable_if_t< is_lindex<LindexType>, bool> = true
>
inline std::string type_tag(const LindexType& = {}) {
	std::stringstream s;
	typename LindexType::BlockType bt{ 0 };
	s << "lindex<"
	  << std::setw(3) << LindexType::nbits << ", "
	  << std::setw(1) << LindexType::lbits << ", "
	  << type_tag(bt) << ">";
	return s.str();
}

// the raw encoding as sign . level . index, e.g. 0b0.011.0101'1000
template<unsigned nbits, unsigned lbits, typename bt>
std::string to_binary(const lindex<nbits, lbits, bt>& v, bool nibbleMarker = false) {
	constexpr unsigned fbits = lindex<nbits, lbits, bt>::fbits;
	std::stringstream s;
	s << "0b" << (v.at(nbits - 1) ? '1' : '0') << '.';
	for (int i = static_cast<int>(nbits) - 2; i >= static_cast<int>(fbits); --i) s << (v.at(static_cast<unsigned>(i)) ? '1' : '0');
	s << '.';
	for (int i = static_cast<int>(fbits) - 1; i >= 0; --i) {
		s << (v.at(static_cast<unsigned>(i)) ? '1' : '0');
		if (nibbleMarker && i > 0 && (i % 4) == 0) s << '\'';
	}
	return s.str();
}

// the raw encoding in hexadecimal
template<unsigned nbits, unsigned lbits, typename bt>
std::string to_hex(const lindex<nbits, lbits, bt>& v, bool nibbleMarker = false) {
	constexpr unsigned nrNibbles = 1u + ((nbits - 1u) >> 2);
	const char hexChar[] = "0123456789ABCDEF";
	std::stringstream s;
	s << "0x";
	for (int n = static_cast<int>(nrNibbles) - 1; n >= 0; --n) {
		s << hexChar[v.nibble(static_cast<unsigned>(n))];
		if (nibbleMarker && n > 0 && (n % 4) == 0) s << '\'';
	}
	return s.str();
}

// the value of v as text: the double where one holds it, otherwise phi(psi), which is the
// value written in the level-index map itself; precision as for a double
template<unsigned nbits, unsigned lbits, typename bt>
std::string to_value_string(const lindex<nbits, lbits, bt>& v, int precision = 17) {
	if (v.isnar()) return "nar";
	std::stringstream s;
	s << std::setprecision(precision);
	const double d = double(v);
	if (std::isfinite(d)) s << d;
	else s << (v.sign() ? "-" : "") << "phi(" << v.psi() << ')';
	return s.str();
}

// the encoding, the fields of |value| and the value
template<unsigned nbits, unsigned lbits, typename bt>
std::string info_print(const lindex<nbits, lbits, bt>& v, int printPrecision = 17) {
	std::stringstream s;
	s << "raw: " << to_binary(v) << ' ';
	if (v.isnar()) { s << "NaR : value nar"; return s.str(); }
	s << (v.sign() ? "s1" : "s0") << " level " << v.level()
	  << " index " << std::setprecision(printPrecision) << v.index()
	  << " : value " << to_value_string(v, printPrecision);
	return s.str();
}

// the fields of an encoding, as one line: sign, level and index of |value|, psi, value
template<unsigned nbits, unsigned lbits, typename bt>
std::string components(const lindex<nbits, lbits, bt>& v) {
	std::stringstream s;
	s << type_tag(v) << ": ";
	if (v.isnar()) { s << "NaR"; return s.str(); }
	s << (v.sign() ? "-" : "+") << " level " << v.level() << " index " << std::setprecision(10) << v.index()
	  << " psi " << v.psi() << " value " << to_value_string(v, 10);
	return s.str();
}

// sign in red, level in yellow, index in magenta
template<unsigned nbits, unsigned lbits, typename bt>
std::string color_print(const lindex<nbits, lbits, bt>& v, bool nibbleMarker = false) {
	constexpr unsigned fbits = lindex<nbits, lbits, bt>::fbits;
	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color magenta(ColorCode::FG_MAGENTA);
	Color def(ColorCode::FG_DEFAULT);
	std::stringstream s;
	s << red << (v.at(nbits - 1) ? '1' : '0');
	s << yellow;
	for (int i = static_cast<int>(nbits) - 2; i >= static_cast<int>(fbits); --i) s << (v.at(static_cast<unsigned>(i)) ? '1' : '0');
	s << magenta;
	for (int i = static_cast<int>(fbits) - 1; i >= 0; --i) {
		s << (v.at(static_cast<unsigned>(i)) ? '1' : '0');
		if (nibbleMarker && i > 0 && (i % 4) == 0) s << '\'';
	}
	s << def;
	return s.str();
}

// the value range, maxneg .. maxpos
template<typename LindexType,
	std::enable_if_t< is_lindex<LindexType>, bool> = true
>
inline std::string range(const LindexType& = {}) {
	LindexType b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
	std::stringstream s;
	s << "[" << to_value_string(b) << " ... " << to_value_string(c) << ", 0, "
	  << to_value_string(d) << " ... " << to_value_string(e) << "]";
	return s.str();
}

}} // namespace sw::universal
