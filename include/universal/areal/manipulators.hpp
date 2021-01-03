#pragma once
// manipulators.hpp: definitions of helper functions for areal type manipulation
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

// This file contains functions that manipulate a posit type
// using posit number system knowledge.

namespace sw::universal {

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
	int nrNibbles = int(1 + ((nbits - 1) >> 2));
	for (long n = nrNibbles - 1; n >= 0; --n) {
		uint8_t nibble = v.nibble(n);
		ss << hexChar[nibble];
		if (n > 0 && ((n * 4ll) % bitsInBlock) == 0) ss << '\'';
	}
	return ss.str();
}

// generate a areal format ASCII format nbits.esxNN...NNa
template<size_t nbits, size_t es, typename bt>
inline std::string hex_print(const areal<nbits, es, bt>& v) {
	std::stringstream ss;
	ss << nbits << '.' << es << 'x' << to_hex(v) << 'r';
	return ss.str();
}

template<size_t nbits, size_t es, typename bt>
std::string pretty_print(const areal<nbits, es, bt>& p, int printPrecision = std::numeric_limits<double>::max_digits10) {
	return "TBD";
}

template<size_t nbits, size_t es, typename bt>
std::string info_print(const areal<nbits, es, bt>& p, int printPrecision = 17) {
	return "TBD";
}

template<size_t nbits, size_t es, typename bt>
std::string color_print(const areal<nbits, es, bt>& r) {
	std::stringstream ss;
	bool s{ false };
	int  e{ 0 };
	blockbinary<r.fbits> f;
	bool ubit{ false };
	decode(r, s, e, f, ubit);

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color blue(ColorCode::FG_BLUE);
	Color magenta(ColorCode::FG_MAGENTA);
	Color cyan(ColorCode::FG_CYAN);
	Color white(ColorCode::FG_WHITE);
	Color def(ColorCode::FG_DEFAULT);
	ss << red << (r.isneg() ? "1" : "0");

	/*
	 *
	bitblock<es> e = _exponent.get();
	int exponentBits = (int)_exponent.nrBits();
	int nrOfExponentBitsProcessed = 0;
	for (int i = int(es) - 1; i >= 0; --i) {
		if (exponentBits > nrOfExponentBitsProcessed++) {
			ss << cyan << (_sign ? (e[i] ? '0' : '1') : (e[i] ? '1' : '0'));
		}
	}

	bitblock<posit<nbits, es>::fbits> f = _fraction.get();
	f = (_sign ? twos_complement(f) : f);
	int fractionBits = (int)_fraction.nrBits();
	int nrOfFractionBitsProcessed = 0;
	for (int i = int(p.fbits) - 1; i >= 0; --i) {
		if (fractionBits > nrOfFractionBitsProcessed++) {
			ss << magenta << (f[i] ? "1" : "0");
		}
	}

	ss << def;
	*/
	return ss.str();
}

}  // namespace sw::universal

