#pragma once
// manipulators.hpp: definitions of helper functions for posit type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
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

	// Generate a type tag for this posit, for example, posit<8,1>
	template<unsigned nbits, unsigned es, typename bt>
	std::string type_tag(const posit<nbits, es, bt> & = {}) {
		std::stringstream str;
		str << "sw::universal::posit<"
			<< std::setw(3) << nbits << ", "
			<< std::setw(1) << es << ", "
			<< typeid(bt).name() << '>';
		return str.str();
	}

	// generate a posit format ASCII format nbits.esxNN...NNp
	template<unsigned nbits, unsigned es, typename bt>
	inline std::string hex_print(const posit<nbits, es, bt>& p) {
		// we need to transform the posit into a string
		std::stringstream str;
		str << nbits << '.' << es << 'x' << to_hex(p.bits()) << 'p';
		return str.str();
	}


	// Generate a string representing the posit components: sign, regime, exponent, faction, and value
	template<unsigned nbits, unsigned es, typename bt>
	std::string components(const posit<nbits, es, bt>& p) {
		constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
		std::stringstream str;
		bool		     		     _sign{false};
		positRegime<nbits, es, bt>   _regime;
		positExponent<nbits, es, bt> _exponent;
		positFraction<fbits, bt>     _fraction;
		decode(p.bits(), _sign, _regime, _exponent, _fraction);

		// TODO: hardcoded field width is governed by pretty printing posit tables, which by construction will always be small posits
		str << std::setw(14) << to_binary(p) << " "
			<< " sign     : " << std::setw(2) << _sign
			<< " regime   : " << std::setw(3) << _regime.positRegime_k()
			<< " exponent : " << std::setw(5) << exponent_value(p)
			<< " fraction : " << std::setw(8) << std::setprecision(21) << _fraction.value()
			<< " value    : " << std::setw(16) << p;

		return str.str();
	}

	template<unsigned nbits, unsigned es, typename bt>
	std::string pretty_print(const posit<nbits, es, bt>& p, int printPrecision = std::numeric_limits<double>::max_digits10) {
		constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
		std::stringstream str;
		bool		     		     _sign;
		positRegime<nbits, es, bt>   _regime;
		positExponent<nbits, es, bt> _exponent;
		positFraction<fbits, bt>     _fraction;
		decode(p.bits(), _sign, _regime, _exponent, _fraction);
		str << ( _sign ? "s1 r" : "s0 r" );
		blockbinary<nbits - 1, bt, BinaryNumberType::Unsigned> r = _regime.bits();
		int regimeBits = (int)_regime.nrBits();
		int nrOfRegimeBitsProcessed = 0;
		for (int i = nbits - 2; i >= 0; --i) {
			if (regimeBits > nrOfRegimeBitsProcessed++) {
				str << (r.test(static_cast<unsigned>(i)) ? "1" : "0");
			}
		}
		str << " e";
		std::uint32_t expBits = _exponent.bits();
		int exponentBits = (int)_exponent.nrBits();
		int nrOfExponentBitsProcessed = 0;
		for (int i = int(es) - 1; i >= 0; --i) {
			if (exponentBits > nrOfExponentBitsProcessed++) {
				str << ((expBits >> i) & 1 ? "1" : "0");
			}
		}
		str << " f";
		if constexpr (fbits > 0) {
			blockbinary<fbits, bt, BinaryNumberType::Unsigned> f = _fraction.bits();
			int fractionBits = (int)_fraction.nrBits();
			int nrOfFractionBitsProcessed = 0;
			for (int i = int(fbits) - 1; i >= 0; --i) {
				if (fractionBits > nrOfFractionBitsProcessed++) {
					str << (f.test(static_cast<unsigned>(i)) ? "1" : "0");
				}
			}
		}
		str << " q";
		str << quadrant(p) << " v"
			<< std::setprecision(printPrecision) << p
			<< std::setprecision(0);
		return str.str();
	}

	template<unsigned nbits, unsigned es, typename bt>
	std::string info_print(const posit<nbits, es, bt>& p, int printPrecision = 17) {
		constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
		std::stringstream str;
		bool		     		     _sign;
		positRegime<nbits, es, bt>   _regime;
		positExponent<nbits, es, bt> _exponent;
		positFraction<fbits, bt>     _fraction;
		decode(p.bits(), _sign, _regime, _exponent, _fraction);

		str << "raw: " << p.bits() << " "
			<< quadrant(p) << " "
			<< (_sign ? "s1 r" : "s0 r")
			<< _regime << " e"
			<< _exponent << " f"
			<< _fraction << " : value "
			<< std::setprecision(printPrecision) << p
			<< std::setprecision(0);
		return str.str();
	}

	template<unsigned nbits, unsigned es, typename bt>
	std::string color_print(const posit<nbits, es, bt>& p) {
		constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
		std::stringstream str;
		bool		     		_sign;
		positRegime<nbits, es, bt>   _regime;
		positExponent<nbits, es, bt> _exponent;
		positFraction<fbits, bt>     _fraction;
		decode(p.bits(), _sign, _regime, _exponent, _fraction);

		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color blue(ColorCode::FG_BLUE);
		Color magenta(ColorCode::FG_MAGENTA);
		Color cyan(ColorCode::FG_CYAN);
		Color white(ColorCode::FG_WHITE);
		Color def(ColorCode::FG_DEFAULT);
		str << red << (p.isneg() ? "1" : "0");

		blockbinary<nbits - 1, bt, BinaryNumberType::Unsigned> r = _regime.bits();
		int regimeBits = (int)_regime.nrBits();
		int nrOfRegimeBitsProcessed = 0;
		for (unsigned i = 0; i < nbits - 1; ++i) {
			unsigned bitIndex = nbits - 2ull - i;
			if (regimeBits > nrOfRegimeBitsProcessed++) {
					str << yellow << (_sign ? (r.test(bitIndex) ? '0' : '1') : (r.test(bitIndex) ? '1' : '0'));
			}
		}

		blockbinary<es, bt, BinaryNumberType::Unsigned> e = _exponent.bits();
		int exponentBits = (int)_exponent.nrBits();
		int nrOfExponentBitsProcessed = 0;
		for (int i = es - 1; i >= 0; --i) {
			if (exponentBits > nrOfExponentBitsProcessed++) {
					str << cyan << (_sign ? (e.test(static_cast<unsigned>(i)) ? '0' : '1') : (e.test(static_cast<unsigned>(i)) ? '1' : '0'));
			}
		}

		blockbinary<posit<nbits, es>::fbits, bt, BinaryNumberType::Unsigned> f = _fraction.bits();
		//f = (_sign ? twosComplement(f) : f);
		int fractionBits = (int)_fraction.nrBits();
		int nrOfFractionBitsProcessed = 0;
		for (int i = int(p.fbits) - 1; i >= 0; --i) {
			if (fractionBits > nrOfFractionBitsProcessed++) {
					str << magenta << (f.test(static_cast<unsigned>(i)) ? "1" : "0");
			}
		}

		str << def;
		return str.str();
	}


}} // namespace sw::universal
