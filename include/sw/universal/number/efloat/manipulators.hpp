#pragma once
// manipulators.hpp: definitions of helper functions for efloat type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the efloat headers (#1334, Phase 2 group 5b, #1455): the <iomanip> half
// -- everything that turns an efloat into a std::string by way of a stringstream.
// iostream.hpp is the <iostream> half. Self-contained.
#include <cstdint>
#include <iomanip>   // std::setw, std::setfill, std::hex, in to_binary()
#include <sstream>
#include <string>
#include <type_traits>  // std::enable_if_t
#include <typeinfo>  // for typeid()
#include <universal/number/efloat/core.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

// This file contains functions that manipulate a efloat type
// using efloat number system knowledge.

namespace sw { namespace universal {

// to_binary formatter for efloat to support test reporters
//
// nibbleMarker is accepted for signature parity with every other type's to_binary and is
// unused: an efloat's limbs are rendered as hex words, which carry their own grouping.
// info_print() is its first caller (#1556), which is what surfaced the -Wextra warning.
template<unsigned nlimbs>
inline std::string to_binary(const efloat<nlimbs>& number, [[maybe_unused]] bool nibbleMarker = false) {
	std::stringstream ss;
	if (number.isnan()) {
		ss << "nan";
	} else if (number.isinf()) {
		ss << (number.sign() == -1 ? "-inf" : "+inf");
	} else if (number.iszero()) {
		ss << "0b0.0.0";
	} else {
		ss << "0b" << (number.sign() == -1 ? "1" : "0") << "."
		   << number.scale() << ".";
		auto limbs = number.bits();
		for (int i = limbs.size() - 1; i >= 0; --i) {
			ss << std::setw(8) << std::setfill('0') << std::hex << limbs[i];
			if (i > 0) ss << "'";
		}
	}
	return ss.str();
}

// Generate a type tag
template<unsigned nlimbs>
std::string type_tag(const efloat<nlimbs>& = {}) {
	return std::string("efloat");
}

// Generate a string representing the efloat components: sign, exponent, faction and value
//
// This never compiled: it called v.exponent(), which efloat does not have -- only
// scale() -- and, like components(areal) before #1453, nothing in the tree instantiated
// it, so the body was never checked. It also reported the sign backwards: sign() returns
// int(-1) or int(+1), never 0, so `v.sign() ? "-" : "+"` rendered every value negative.
// Repaired alongside #1556, which needed a components() it could trust.
template<typename EfloatType,
	std::enable_if_t< is_efloat<EfloatType>, bool> = true
>
inline std::string components(const EfloatType& v) {
	std::stringstream s;
	s << ((v.sign() == -1) ? "(-, " : "(+, ");
	s << v.scale() << ", ";
	auto limbs = v.bits();
	for (std::size_t i = limbs.size(); i > 0; --i) {
		s << std::setw(8) << std::setfill('0') << std::hex << limbs[i - 1] << std::dec << std::setfill(' ');
		if (i > 1) s << '\'';
	}
	s << ')';
	return s.str();
}

template<typename EfloatType,
	std::enable_if_t< is_efloat<EfloatType>, bool> = true
>
inline std::string to_triple(const EfloatType& v) {
	std::stringstream s;
	s << (v.isneg() ? "(-, " : "(+, ");
	s << v.scale() << ", ";
	bool firstLimb{ true };
	for (auto l : v.bits()) {
		if (!firstLimb) s << '\'';
		uint64_t mask = (1ull << 31);
		for (int i = 31; i >= 0; --i) {
			s << ((l & mask) ? '1' : '0');
			if (firstLimb && i == 31) s << '.';
			if (i > 0 && i % 4 == 0) s << '\'';
			mask >>= 1;
		}
		firstLimb = false;
	}
	s << ')';
	return s.str();
}

// Generate a hex string for an efloat: sign, binary scale, and the significand words.
//
// It returned the literal "tbd" over a commented-out body that indexed a nibble(n) and an
// nbits that an efloat does not have (#1582). An efloat is adaptive-precision, so there
// is no fixed nibble count to walk: the significand is a vector of 32-bit words, and the
// hex form is those words, most significant first, the way to_binary() already renders
// them.
//
// Unlike dd and qd, which delegate to their limbs' native hex (dd/manipulators.hpp), an
// efloat's limbs are uint32_t magnitude words rather than floating-point values, so there
// is no native rendering to hand them to.
template<typename EfloatType,
	std::enable_if_t< is_efloat<EfloatType>, bool> = true
>
inline std::string to_hex(const EfloatType& v, bool nibbleMarker = false, bool hexPrefix = true) {
	constexpr char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::stringstream s;
	if (v.isnan()) return std::string("nan");
	if (v.isinf()) return std::string(v.sign() == -1 ? "-inf" : "+inf");
	if (hexPrefix) s << "0x";
	s << (v.sign() == -1 ? '1' : '0') << '.' << v.scale() << '.';
	if (v.iszero()) {
		s << '0';
		return s.str();
	}
	const auto limbs = v.bits();
	for (std::size_t w = limbs.size(); w > 0; --w) {
		const uint32_t word = limbs[w - 1];
		for (int n = 7; n >= 0; --n) {
			s << hexChar[(word >> (n * 4)) & 0xFu];
			if (nibbleMarker && n > 0 && (n % 2) == 0) s << '\'';
		}
		if (w > 1) s << '\'';
	}
	return s.str();
}

// Generate an efloat format ASCII hex format, the analogue of cfloat's nbits.esxNN...NNc.
//
// It returned the literal "tbd" (#1582). An efloat has no nbits or es to name, so the
// configuration field is the limb count -- the one dimension that actually varies -- and
// the type is tagged 'e' as cfloat tags 'c' and areal tags 'r'.
template<typename EfloatType,
	std::enable_if_t< is_efloat<EfloatType>, bool> = true
>
inline std::string hex_print(const EfloatType& c) {
	std::stringstream s;
	s << c.bits().size() << 'x' << to_hex(c) << 'e';
	return s.str();
}

// Render an efloat's fields separated: sign : scale : significand words in binary.
//
// It returned the literal "tbd" (#1582). The colon-separated shape is cfloat's
// pretty_print, so an efloat lines up with the fixed-size types when they are printed
// together.
template<typename EfloatType,
	std::enable_if_t< is_efloat<EfloatType>, bool> = true
>
inline std::string pretty_print(const EfloatType& r) {
	std::stringstream s;
	if (r.isnan()) return std::string("nan");
	if (r.isinf()) return std::string(r.sign() == -1 ? "-inf" : "+inf");
	s << (r.sign() == -1 ? '1' : '0') << ':' << r.scale() << ':';
	if (r.iszero()) {
		s << '0';
		return s.str();
	}
	const auto limbs = r.bits();
	for (std::size_t w = limbs.size(); w > 0; --w) {
		const uint32_t word = limbs[w - 1];
		for (int b = 31; b >= 0; --b) s << ((word >> b) & 1u ? '1' : '0');
		if (w > 1) s << '\'';
	}
	return s.str();
}

// Report the state, the limb layout and the value of an efloat.
//
// It returned the literal "TBD" and ignored both of its arguments (#1556). The shape is
// posit's -- "raw: <bits> <fields> : value <v>" (posit/iostream.hpp) -- but an efloat is
// adaptive-precision: there is no fixed sign/exponent/fraction bit layout to decode, so
// the fields are the ones that do vary, the limb count and the binary scale.
//
// to_string() comes from the core, so the value field is what operator<< renders without
// this layer depending on iostream.hpp (#1334).
template<typename EfloatType,
	std::enable_if_t< is_efloat<EfloatType>, bool> = true
>
inline std::string info_print(const EfloatType& p, int printPrecision = 17) {
	std::stringstream s;
	s << "raw: " << to_binary(p) << ' ' << ((p.sign() == -1) ? "s1" : "s0");
	if (p.isnan())       s << " nan";
	else if (p.isinf())  s << " inf";
	else if (p.iszero()) s << " zero";
	else s << " limbs " << p.bits().size() << " scale " << p.scale();
	s << " : value " << to_string(p, printPrecision, 0, false, false, false, false, false, false, ' ');
	return s.str();
}

// Generate a binary, color-coded representation of the efloat.
//
// It returned the literal "tbd" (#1582). The colour assignment is the one every other
// type uses -- red sign, cyan exponent, magenta significand, yellow separators -- so an
// efloat reads the same as a cfloat or a posit in a terminal.
template<typename EfloatType,
	std::enable_if_t< is_efloat<EfloatType>, bool> = true
>
inline std::string color_print(const EfloatType& r, bool nibbleMarker = false) {
	std::stringstream s;

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color cyan(ColorCode::FG_CYAN);
	Color magenta(ColorCode::FG_MAGENTA);
	Color def(ColorCode::FG_DEFAULT);

	if (r.isnan()) { s << red << "nan" << def; return s.str(); }
	if (r.isinf()) { s << red << (r.sign() == -1 ? "-inf" : "+inf") << def; return s.str(); }

	// sign
	s << red << (r.sign() == -1 ? '1' : '0');
	// exponent
	s << yellow << '.' << cyan << r.scale();
	s << yellow << '.';
	if (r.iszero()) {
		s << magenta << '0' << def;
		return s.str();
	}
	// significand words, most significant first
	const auto limbs = r.bits();
	for (std::size_t w = limbs.size(); w > 0; --w) {
		const uint32_t word = limbs[w - 1];
		for (int b = 31; b >= 0; --b) {
			s << magenta << ((word >> b) & 1u ? '1' : '0');
			if (nibbleMarker && b > 0 && (b % 4) == 0) s << yellow << '\'';
		}
		if (w > 1) s << yellow << '\'';
	}
	s << def;
	return s.str();
}


}} // namespace sw::universal
