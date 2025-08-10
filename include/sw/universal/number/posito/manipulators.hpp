#pragma once
// manipulators.hpp: definitions of helper functions for posit type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <universal/utility/color_print.hpp>  // base class for color printing in shells

// This file contains functions that manipulate a posit type
// using posit number system knowledge.

namespace sw { namespace universal {


	// Generate a type tag for this posit, for example, posit<8,1>
	template<unsigned nbits, unsigned es>
	std::string type_tag(const posito<nbits, es> & = {}) {
		std::stringstream str;
		str << "posito<"
			<< std::setw(3) << nbits << ", "
			<< std::setw(1) << es << '>';
		return str.str();
	}

	// Generate a type field descriptor for this cfloat
	template<typename PositoType,
		std::enable_if_t< is_posito<PositoType>, bool> = true
	>
	inline std::string type_field(const PositoType & = {}) {
		std::stringstream s;
//		unsigned nbits = PositoType::nbits;  // total bits
		unsigned ebits = PositoType::es;     // exponent bits
		unsigned fbits = PositoType::fbits;  // integer bits
		s << "fields(s:1|r:[2]+|e:" << ebits << "|m:" << fbits << ')';
		return s.str();
	}

// Generate a string representing the posit components: sign, regime, exponent, faction, and value
template<unsigned nbits, unsigned es>
std::string components(const posito<nbits, es>& p) {
	constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
	std::stringstream str;
	bool                      _sign;
	positRegime<nbits, es>    _regime;
	positExponent<nbits, es>  _exponent;
	positFraction<fbits>      _fraction;
	decode(p.get(), _sign, _regime, _exponent, _fraction);

	// TODO: hardcoded field width is governed by pretty printing posit tables, which by construction will always be small posits
	str << std::setw(14) << p.get() << " " << std::setw(14) << decoded(p)
		<< " sign     : " << std::setw(2) << _sign
		<< " regime   : " << std::setw(3) << _regime.regime_k()
		<< " exponent : " << std::setw(5) << exponent_value(p)
		<< " fraction : " << std::setw(8) << std::setprecision(21) << _fraction.value()
		<< " value    : " << std::setw(16) << p;

	return str.str();
}

// generate a binary string for posit
template<unsigned nbits, unsigned es>
inline std::string to_hex(const posito<nbits, es>& v, bool nibbleMarker = false, bool hexPrefix = true) {
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

// generate a posit format ASCII format nbits.esxNN...NNp
template<unsigned nbits, unsigned es>
inline std::string hex_print(const posito<nbits, es>& p) {
	// we need to transform the posit into a string
	std::stringstream str;
	str << nbits << '.' << es << 'x' << to_hex(p.get()) << 'p';
	return str.str();
}

template<unsigned nbits, unsigned es>
std::string pretty_print(const posito<nbits, es>& p, int printPrecision = std::numeric_limits<double>::max_digits10) {
	constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
	std::stringstream str;
	bool		     	 _sign;
	positRegime<nbits, es>    _regime;
	positExponent<nbits, es>  _exponent;
	positFraction<fbits>      _fraction;
	decode(p.get(), _sign, _regime, _exponent, _fraction);
	str << ( _sign ? "s1 r" : "s0 r" );
	bitblock<nbits-1> r = _regime.get();
	int regimeBits = (int)_regime.nrBits();
	int nrOfRegimeBitsProcessed = 0;
	for (int i = nbits - 2; i >= 0; --i) {
		if (regimeBits > nrOfRegimeBitsProcessed++) {
			str << (r[static_cast<unsigned>(i)] ? "1" : "0");
		}
	}
	str << " e";
	bitblock<es> e = _exponent.get();
	int exponentBits = (int)_exponent.nrBits();
	int nrOfExponentBitsProcessed = 0;
	for (int i = int(es) - 1; i >= 0; --i) {
		if (exponentBits > nrOfExponentBitsProcessed++) {
			str << (e[static_cast<unsigned>(i)] ? "1" : "0");
		}
	}
	str << " f";
	bitblock<fbits> f = _fraction.get();
	int fractionBits = (int)_fraction.nrBits();
	int nrOfFractionBitsProcessed = 0;
	for (int i = int(fbits) - 1; i >= 0; --i) {
		if (fractionBits > nrOfFractionBitsProcessed++) {
			str << (f[static_cast<unsigned>(i)] ? "1" : "0");
		}
	}
	str << " q";
	str << quadrant(p) << " v"
		<< std::setprecision(printPrecision) << p
		<< std::setprecision(0);
	return str.str();
}

template<unsigned nbits, unsigned es>
std::string info_print(const posito<nbits, es>& p, int printPrecision = 17) {
	constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
	std::stringstream str;
	bool		     	 _sign;
	positRegime<nbits, es>    _regime;
	positExponent<nbits, es>  _exponent;
	positFraction<fbits>      _fraction;
	decode(p.get(), _sign, _regime, _exponent, _fraction);

	str << "raw: " << p.get() << " " // << " decoded: " << decoded(p) << " "
		<< quadrant(p) << " "
		<< (_sign ? "s1 r" : "s0 r")
		<< _regime << " e"
		<< _exponent << " f"
		<< _fraction << " : value "
		<< std::setprecision(printPrecision) << p
		<< std::setprecision(0);
	return str.str();
}

template<unsigned nbits, unsigned es>
std::string color_print(const posito<nbits, es>& p) {
	constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
	std::stringstream str;
	bool		     	 _sign;
	positRegime<nbits, es>    _regime;
	positExponent<nbits, es>  _exponent;
	positFraction<fbits>      _fraction;
	extract_fields(p.get(), _sign, _regime, _exponent, _fraction);

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color blue(ColorCode::FG_BLUE);
	Color magenta(ColorCode::FG_MAGENTA);
	Color cyan(ColorCode::FG_CYAN);
	Color white(ColorCode::FG_WHITE);
	Color def(ColorCode::FG_DEFAULT);
	str << red << (p.isneg() ? "1" : "0");

	if (p.isnar()) {
		for (unsigned i = 0; i < nbits - 1; ++i) str << yellow << '0';
	}
	else {
		bitblock<nbits - 1> r = _regime.get();
		int regimeBits = (int)_regime.nrBits();
		int nrOfRegimeBitsProcessed = 0;
		for (unsigned i = 0; i < nbits - 1; ++i) {
			unsigned bitIndex = nbits - 2u - i;
			if (regimeBits > nrOfRegimeBitsProcessed++) {
				str << yellow << (r[bitIndex] ? '1' : '0');
			}
		}
	}

	int exponentBits = (int)_exponent.nrBits();
	int nrOfExponentBitsProcessed = 0;
	if constexpr (es > 0) {
		for (unsigned i = 0; i < es; ++i) {
			bitblock<es> e = _exponent.get();
			unsigned bitIndex = es - 1u - i;
			if (exponentBits > nrOfExponentBitsProcessed++) {
				str << cyan << (e[bitIndex] ? '1' : '0');
			}
		}
	}

	bitblock<posito<nbits, es>::fbits> f = _fraction.get();
	int fractionBits = (int)_fraction.nrBits();
	int nrOfFractionBitsProcessed = 0;
	for (unsigned i = 0; i < p.fbits; ++i) {
		unsigned bitIndex = p.fbits - 1u - i;
		if (fractionBits > nrOfFractionBitsProcessed++) {
			str << magenta << (f[bitIndex] ? "1" : "0");
		}
	}

	str << def;
	return str.str();
}

}} // namespace sw::universal

