#pragma once
// manipulators.hpp: definitions of helper functions for areal type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the areal headers (#1334): the <iomanip> half -- everything that turns an
// areal into a std::string through a stringstream. iostream.hpp is the <iostream> half.
// This header does NOT include iostream.hpp: areal's builders format from the bit
// pattern rather than streaming the value, so the dependency runs one way only.
// Self-contained.
#include <string>        // std::string
#include <sstream>       // std::stringstream
#include <iomanip>
#include <typeinfo>  // for typeid()

#include <universal/number/areal/core.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

// This file contains functions that manipulate a posit type
// using posit number system knowledge.

namespace sw { namespace universal {

// Generate a type tag for this posit, for example, posit<8,1>
template<unsigned nbits, unsigned es, typename bt>
std::string type_tag(const areal<nbits, es, bt>& = {}) {
	std::stringstream ss;
	ss << "areal<" << nbits << "," << es << ">";
	return ss.str();
}

// Generate a string representing the areal components: sign, exponent, faction, uncertainty bit, and value
template<unsigned nbits, unsigned es, typename bt>
std::string components(const areal<nbits, es, bt>& v) {
	std::stringstream ss;
	// decode() takes the exponent and fraction as blockbinary fields, and fbits is a
	// static member of the type -- not of the value. Reading them off an instance, with
	// an int exponent, is why this template never compiled: nothing instantiated it, and
	// a function template's body is not checked until it is (#1453).
	constexpr unsigned fbits = areal<nbits, es, bt>::fbits;
	bool s{ false };
	blockbinary<es, bt> e;
	blockbinary<fbits, bt> f;
	bool u{ false };
	decode(v, s, e, f, u);

	// TODO: hardcoded field width is governed by pretty printing areal tables, which by construction will always be small areals
	// the fields are rendered bit by bit, the way pretty_print in this header does:
	// to_binary(blockbinary) is not reachable from the manipulator layer (#1334)
	std::string ebits, fbits_str;
	for (int i = int(es) - 1; i >= 0; --i)    ebits     += (e.test(static_cast<size_t>(i)) ? '1' : '0');
	for (int i = int(fbits) - 1; i >= 0; --i) fbits_str += (f.test(static_cast<size_t>(i)) ? '1' : '0');

	ss << std::setw(14) << to_binary(v)
		<< " Sign : " << std::setw(2) << (s ? '1' : '0')
		<< " Exponent : " << std::setw(5) << ebits
		<< " Fraction : " << std::setw(8) << fbits_str
		<< " Uncertainty : " << std::setw(2) << (u ? '1' : '0')
		<< " Value : " << std::setw(16) << double(v);

	return ss.str();
}

// generate a binary string for areal
template<unsigned nbits, unsigned es, typename bt>
inline std::string to_hex(const areal<nbits, es, bt>& v, bool nibbleMarker = false, bool hexPrefix = true) {
	constexpr unsigned bitsInByte = 8;
	constexpr unsigned bitsInBlock = sizeof(bt) * bitsInByte;
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::stringstream ss;
	if (hexPrefix) ss << "0x";
	long nrNibbles = long(1ull + ((nbits - 1ull) >> 2ull));
	for (long n = nrNibbles - 1; n >= 0; --n) {
		uint8_t nibble = v.nibble(unsigned(n));
		ss << hexChar[nibble];
		if (nibbleMarker && n > 0 && ((n * 4ll) % bitsInBlock) == 0) ss << '\'';
	}
	return ss.str();
}

// generate a areal format ASCII format nbits.esxNN...NNa
template<unsigned nbits, unsigned es, typename bt>
inline std::string hex_print(const areal<nbits, es, bt>& r) {
	std::stringstream ss;
	ss << nbits << '.' << es << 'x' << to_hex(r) << 'r';
	return ss.str();
}

template<unsigned nbits, unsigned es, typename bt>
std::string pretty_print(const areal<nbits, es, bt>& r) {
	std::stringstream ss;
	constexpr unsigned fbits = areal<nbits, es, bt>::fbits;
	bool s{ false };
	blockbinary<es, bt> e;
	blockbinary<fbits, bt> f;
	bool ubit{ false };
	decode(r, s, e, f, ubit);

	// sign bit
	ss << (r.isneg() ? '1' : '0');

	// exponent bits
	ss << '-';
	for (int i = int(es) - 1; i >= 0; --i) {
		ss << (e.test(i) ? '1' : '0');
	}

	// fraction bits
	ss << '-';
	for (int i = int(r.fbits) - 1; i >= 0; --i) {
		ss << (f.test(i) ? '1' : '0');
	}

	// uncertainty bit
	ss << '-';
	ss << (r.test(0) ? "1" : "0");
	return ss.str();
}

// Report the raw encoding, the decoded fields and the value of an areal.
//
// It returned the literal "TBD" and ignored both of its arguments (#1556). The shape is
// posit's -- "raw: <bits> <fields> : value <v>" (posit/iostream.hpp) -- built from the
// same decode() the other builders in this header use, rather than by streaming the
// areal: this layer must not depend on iostream.hpp (#1334).
//
// The value is the interval to_string() renders: [v] when the encoding is exact and
// (v, next) when the uncertainty bit is set, since the ubit is what an areal is for.
template<unsigned nbits, unsigned es, typename bt>
std::string info_print(const areal<nbits, es, bt>& p, int printPrecision = 17) {
	constexpr unsigned fbits = areal<nbits, es, bt>::fbits;
	bool s{ false };
	blockbinary<es, bt> e;
	blockbinary<fbits, bt> f;
	bool u{ false };
	decode(p, s, e, f, u);

	std::stringstream str;
	str << "raw: " << to_binary(p) << ' ' << (s ? "s1 e" : "s0 e");
	for (int i = int(es) - 1; i >= 0; --i)    str << (e.test(static_cast<size_t>(i)) ? '1' : '0');
	str << " f";
	for (int i = int(fbits) - 1; i >= 0; --i) str << (f.test(static_cast<size_t>(i)) ? '1' : '0');
	str << " u" << (u ? '1' : '0') << " : value " << std::setprecision(printPrecision);

	const double d = double(p);
	if (u && !p.isnan()) {
		areal<nbits, es, bt> next(p);
		++next;
		str << '(' << d << ", " << double(next) << ')';
	}
	else {
		str << '[' << d << ']';
	}
	return str.str();
}

template<unsigned nbits, unsigned es, typename bt>
std::string color_print(const areal<nbits, es, bt>& r) {
	std::stringstream str;
	constexpr unsigned fbits = areal<nbits, es, bt>::fbits;
	bool s{ false };
	blockbinary<es, bt> e;
	blockbinary<fbits, bt> f;
	bool ubit{ false };
	decode(r, s, e, f, ubit);

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color blue(ColorCode::FG_BLUE);
	Color magenta(ColorCode::FG_MAGENTA);
	Color cyan(ColorCode::FG_CYAN);
	Color white(ColorCode::FG_WHITE);
	Color def(ColorCode::FG_DEFAULT);

	// sign bit
	str << red << (s ? "1" : "0");

	// exponent bits
	for (unsigned i = 0; i < es; ++i) {
		unsigned bitIndex = es - 1 - i;
		str << cyan << (e.test(bitIndex) ? '1' : '0');
	}

	// fraction bits
	for (unsigned i = 0; i < fbits; ++i) {
		unsigned bitIndex = fbits - 1 - i;
		str << magenta << (f.test(bitIndex) ? '1' : '0');
	}

	// uncertainty bit
	str << yellow << (ubit ? "1" : "0");
	str << def;
	return str.str();
}


// Moved out of areal_impl.hpp (#1334): both format an areal through a stringstream,
// which is what keeps them out of the core.
// convert to std::string
template<unsigned nbits, unsigned es, typename bt>
inline std::string to_string(const areal<nbits,es,bt>& v) {
	// Rendered the way operator<< renders it: [v] for an exact value and (v, next) for an
	// uncertain one, since the uncertainty bit is what an areal is for. The rendering is
	// repeated here rather than borrowed from operator<<, which lives in iostream.hpp:
	// the manipulator layer does not depend on the stream layer (#1334).
	//
	// This used to return an EMPTY string for every value but zero and infinity -- its
	// rendering line was commented out, calling a fraction() that no longer takes that
	// form -- and those two came back as " zero b" and " infinite b" (#1554).
	std::stringstream s;
	const double d = double(v);
	if (v.at(0) && !v.isnan()) {
		areal<nbits, es, bt> next(v);
		++next;
		s << '(' << d << ", " << double(next) << ')';
	}
	else {
		s << '[' << d << ']';
	}
	return s.str();
}

// transform areal to a binary representation
template<unsigned nbits, unsigned es, typename bt>
inline std::string to_binary(const areal<nbits, es, bt>& number, bool nibbleMarker = false) {
	std::stringstream ss;
	ss << 'b';
	unsigned index = nbits;
	for (unsigned i = 0; i < nbits; ++i) {
		ss << (number.at(--index) ? '1' : '0');
		if (index > 0 && (index % 4) == 0 && nibbleMarker) ss << '\'';
	}
	return ss.str();
}

}} // namespace sw::universal
