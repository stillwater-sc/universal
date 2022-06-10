#pragma once
// manipulators.hpp: definitions of helper functions for areal type manipulation
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

// This file contains functions that manipulate a posit type
// using posit number system knowledge.

namespace sw { namespace universal {

// Generate a type tag for this posit, for example, posit<8,1>
template<size_t nbits, size_t es, typename bt>
std::string type_tag(const areal<nbits, es, bt>& v) {
	std::stringstream ss;
	ss << "areal<" << nbits << "," << es << ">";
	return ss.str();
}

// Generate a string representing the areal components: sign, exponent, faction, uncertainty bit, and value
template<size_t nbits, size_t es, typename bt>
std::string components(const areal<nbits, es, bt>& v) {
	std::stringstream ss;
	bool s{ false };
	int  e{ 0 };
	blockbinary<v.fbits> f;
	bool u{ false };
	decode(v, s, e, f, u);

	// TODO: hardcoded field width is governed by pretty printing areal tables, which by construction will always be small areals
	ss << std::setw(14) << to_binary(v) 
		<< " Sign : " << std::setw(2) << s
		<< " Exponent : " << std::setw(5) << e
		<< " Fraction : " << std::setw(8) << std::setprecision(21) << "TBD"
		<< " Uncertainty : " << std::setw(2) << u
		<< " Value : " << std::setw(16) << u;

	return ss.str();
}

// generate a binary string for areal
template<size_t nbits, size_t es, typename bt>
inline std::string to_hex(const areal<nbits, es, bt>& v) {
	constexpr size_t bitsInByte = 8;
	constexpr size_t bitsInBlock = sizeof(bt) * bitsInByte;
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::stringstream ss;
	ss << "0x" << std::hex;
	long nrNibbles = long(1ull + ((nbits - 1ull) >> 2ull));
	for (long n = nrNibbles - 1; n >= 0; --n) {
		uint8_t nibble = v.nibble(size_t(n));
		ss << hexChar[nibble];
		if (n > 0 && ((n * 4ll) % bitsInBlock) == 0) ss << '\'';
	}
	return ss.str();
}

// generate a areal format ASCII format nbits.esxNN...NNa
template<size_t nbits, size_t es, typename bt>
inline std::string hex_print(const areal<nbits, es, bt>& r) {
	std::stringstream ss;
	ss << nbits << '.' << es << 'x' << to_hex(r) << 'r';
	return ss.str();
}

template<size_t nbits, size_t es, typename bt>
std::string pretty_print(const areal<nbits, es, bt>& r, int printPrecision = std::numeric_limits<double>::max_digits10) {
	std::stringstream ss;
	constexpr size_t fbits = areal<nbits, es, bt>::fbits;
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

template<size_t nbits, size_t es, typename bt>
std::string info_print(const areal<nbits, es, bt>& p, int printPrecision = 17) {
	return "TBD";
}

template<size_t nbits, size_t es, typename bt>
std::string color_print(const areal<nbits, es, bt>& r) {
	std::stringstream ss;
	bool s{ false };
	blockbinary<es,bt> e;
	blockbinary<areal<nbits, es, bt>::fbits> f;
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
	ss << red << (r.isneg() ? "1" : "0");

	// exponent bits
	for (int i = int(es) - 1; i >= 0; --i) {
		ss << cyan << (e.test(i) ? '1' : '0');
	}

	// fraction bits
	for (int i = int(r.fbits) - 1; i >= 0; --i) {
		ss << magenta << (f.test(i) ? '1' : '0');
	}

	// uncertainty bit
	ss << yellow << (r.test(0) ? "1" : "0");

	ss << def;
	return ss.str();
}

}} // namespace sw::universal

