#pragma once
// manipulators.hpp: definitions of helper functions for posit type manipulation
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <iomanip>
#include <cmath>  // for frexp/frexpf
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

// This file contains functions that manipulate a posit type
// using posit number system knowledge.

namespace sw { namespace universal {

// report dynamic range of a type, specialized for a posit
template<size_t nbits, size_t es>
std::string dynamic_range() {
	std::stringstream str;
	str << " posit<" << std::setw(3) << nbits << "," << es << "> ";
	str << "useed scale  " << std::setw(4) << useed_scale<nbits, es>() << "     ";
	str << "minpos scale " << std::setw(10) << minpos_scale<nbits, es>() << "     ";
	str << "maxpos scale " << std::setw(10) << maxpos_scale<nbits, es>();
	return str.str();
}

// report the dynamic range of the type associated with a value
template<size_t nbits, size_t es>
std::string dynamic_range(const posit<nbits, es>& p) {
	std::stringstream str;
	str << " posit<" << std::setw(3) << nbits << "," << es << "> ";
	str << "useed scale  " << std::setw(4) << useed_scale<nbits, es>() << "     ";
	str << "minpos scale " << std::setw(10) << minpos_scale<nbits, es>() << "     ";
	str << "maxpos scale " << std::setw(10) << maxpos_scale<nbits, es>() << "  :  " << p;
	return str.str();
}

// report the dynamic range of a posit
template<size_t nbits, size_t es>
std::string posit_range() {
	std::stringstream str;
	str << " posit<" << std::setw(3) << nbits << "," << es << "> ";
	str << "useed scale  " << std::setw(4) << useed_scale<nbits, es>() << "     ";
	str << "minpos scale " << std::setw(10) << minpos_scale<nbits, es>() << "     ";
	str << "maxpos scale " << std::setw(10) << maxpos_scale<nbits, es>() << "     ";
	str << "minimum " << std::setw(12) << std::numeric_limits<sw::universal::posit<nbits, es>>::min() << "     ";
	str << "maximum " << std::setw(12) << std::numeric_limits<sw::universal::posit<nbits, es>>::max() ;
	return str.str();
}

// Generate a type tag for this posit, for example, posit<8,1>
template<size_t nbits, size_t es>
std::string type_tag(const posit<nbits, es>& = {}) {
	std::stringstream str;
	str << "sw::universal::posit<" 
		<< std::setw(3) << nbits << ", " 
		<< std::setw(1) << es << '>';
	return str.str();
}

// Generate a string representing the posit components: sign, regime, exponent, faction, and value
template<size_t nbits, size_t es>
std::string components(const posit<nbits, es>& p) {
	constexpr size_t fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
	std::stringstream str;
	bool		     	 _sign;
	regime<nbits, es>    _regime;
	exponent<nbits, es>  _exponent;
	fraction<fbits>      _fraction;
	decode(p.get(), _sign, _regime, _exponent, _fraction);

	// TODO: hardcoded field width is governed by pretty printing posit tables, which by construction will always be small posits
	str << std::setw(14) << p.get() << " " << std::setw(14) << decoded(p)
		<< " Sign : " << std::setw(2) << _sign
		<< " Regime : " << std::setw(3) << _regime.regime_k()
		<< " Exponent : " << std::setw(5) << exponent_value(p)
		<< " Fraction : " << std::setw(8) << std::setprecision(21) << _fraction.value()
		<< " Value : " << std::setw(16) << p;

	return str.str();
}

template<size_t nbits, size_t es>
std::string component_values_to_string(const posit<nbits, es>& p) {
	std::stringstream str;
	// TODO: hardcoded field sizes
	if (p.iszero()) {
		str << " zero    " << std::setw(103) << "b" << p.get();
		return str.str();
	}
	else if (p.isinf()) {
		str << " infinite" << std::setw(103) << "b" << p.get();
		return str.str();
	}

	str << std::setw(14) << to_binary(p.get())
		<< " Sign : " << std::setw(2) << p.sign()
		<< " Regime : " << p.regime_int()
		<< " Exponent : " << p.exponent_int()
		<< std::hex
		<< " Fraction : " << p.fraction_int()
		<< " Value : " << p.to_int64()
		<< std::dec;
	return str.str();
}

// generate a posit format ASCII format nbits.esxNN...NNp
template<size_t nbits, size_t es>
inline std::string hex_print(const posit<nbits, es>& p) {
	// we need to transform the posit into a string
	std::stringstream str;
	str << nbits << '.' << es << 'x' << to_hex(p.get()) << 'p';
	return str.str();
}

template<size_t nbits, size_t es>
std::string pretty_print(const posit<nbits, es>& p, int printPrecision = std::numeric_limits<double>::max_digits10) {
	constexpr size_t fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
	std::stringstream str;
	bool		     	 _sign;
	regime<nbits, es>    _regime;
	exponent<nbits, es>  _exponent;
	fraction<fbits>      _fraction;
	decode(p.get(), _sign, _regime, _exponent, _fraction);
	str << ( _sign ? "s1 r" : "s0 r" );
	bitblock<nbits-1> r = _regime.get();
	int regimeBits = (int)_regime.nrBits();
	int nrOfRegimeBitsProcessed = 0;
	for (int i = nbits - 2; i >= 0; --i) {
		if (regimeBits > nrOfRegimeBitsProcessed++) {
			str << (r[static_cast<size_t>(i)] ? "1" : "0");
		}
	}
	str << " e";
	bitblock<es> e = _exponent.get();
	int exponentBits = (int)_exponent.nrBits();
	int nrOfExponentBitsProcessed = 0;
	for (int i = int(es) - 1; i >= 0; --i) {
		if (exponentBits > nrOfExponentBitsProcessed++) {
			str << (e[static_cast<size_t>(i)] ? "1" : "0");
		}
	}
	str << " f";
	bitblock<fbits> f = _fraction.get();
	int fractionBits = (int)_fraction.nrBits();
	int nrOfFractionBitsProcessed = 0;
	//for (int i = int(p.fbits) - 1; i >= 0; --i) {  // this does not look correct
	for (int i = int(fbits) - 1; i >= 0; --i) {
		if (fractionBits > nrOfFractionBitsProcessed++) {
			str << (f[static_cast<size_t>(i)] ? "1" : "0");
		}
	}
	str << " q";
	str << quadrant(p) << " v"
		<< std::setprecision(printPrecision) << p
		<< std::setprecision(0);
	return str.str();
}

template<size_t nbits, size_t es>
std::string info_print(const posit<nbits, es>& p, int printPrecision = 17) {
	constexpr size_t fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
	std::stringstream str;
	bool		     	 _sign;
	regime<nbits, es>    _regime;
	exponent<nbits, es>  _exponent;
	fraction<fbits>      _fraction;
	decode(p.get(), _sign, _regime, _exponent, _fraction);

	str << "raw: " << p.get() << " decoded: " << decoded(p) << " "
		<< quadrant(p) << " "
		<< (_sign ? "negative r" : "positive r")
		<< _regime << " e"
		<< _exponent << " f"
		<< _fraction << " : value "
		<< std::setprecision(printPrecision) << p
		<< std::setprecision(0);
	return str.str();
}

template<size_t nbits, size_t es>
std::string color_print(const posit<nbits, es>& p) {
	constexpr size_t fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
	std::stringstream str;
	bool		     	 _sign;
	regime<nbits, es>    _regime;
	exponent<nbits, es>  _exponent;
	fraction<fbits>      _fraction;
	decode(p.get(), _sign, _regime, _exponent, _fraction);

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color blue(ColorCode::FG_BLUE);
	Color magenta(ColorCode::FG_MAGENTA);
	Color cyan(ColorCode::FG_CYAN);
	Color white(ColorCode::FG_WHITE);
	Color def(ColorCode::FG_DEFAULT);
	str << red << (p.isneg() ? "1" : "0");

	bitblock<nbits - 1> r = _regime.get();
	int regimeBits = (int)_regime.nrBits();
	int nrOfRegimeBitsProcessed = 0;
	for (int i = nbits - 2; i >= 0; --i) {
		if (regimeBits > nrOfRegimeBitsProcessed++) {
			str << yellow << (_sign ? (r[static_cast<size_t>(i)] ? '0' : '1') : (r[static_cast<size_t>(i)] ? '1' : '0'));
		}
	}

	bitblock<es> e = _exponent.get();
	int exponentBits = (int)_exponent.nrBits();
	int nrOfExponentBitsProcessed = 0;
	for (int i = int(es) - 1; i >= 0; --i) {
		if (exponentBits > nrOfExponentBitsProcessed++) {
			str << cyan << (_sign ? (e[static_cast<size_t>(i)] ? '0' : '1') : (e[static_cast<size_t>(i)] ? '1' : '0'));
		}
	}

	bitblock<posit<nbits, es>::fbits> f = _fraction.get();
	f = (_sign ? twos_complement(f) : f);
	int fractionBits = (int)_fraction.nrBits();
	int nrOfFractionBitsProcessed = 0;
	for (int i = int(p.fbits) - 1; i >= 0; --i) {
		if (fractionBits > nrOfFractionBitsProcessed++) {
			str << magenta << (f[static_cast<size_t>(i)] ? "1" : "0");
		}
	}

	str << def;
	return str.str();
}

}} // namespace sw::universal

