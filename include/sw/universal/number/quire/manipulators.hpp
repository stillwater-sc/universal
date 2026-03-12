#pragma once
// manipulators.hpp: definitions of helper functions for manipulation of generalized quire types
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

namespace sw { namespace universal {

	// Generate a type tag for this generalized quire
	template<typename NumberType, unsigned capacity, typename LimbType>
	std::string type_tag(const quire<NumberType, capacity, LimbType>& = {}) {
		std::stringstream str;
		str << "quire<"
			<< type_tag(NumberType{}) << ", "
			<< capacity << ", "
			<< type_tag(LimbType{}) << '>';
		return str.str();
	}

	// report the dynamic range of a quire
	template<typename NumberType, unsigned capacity, typename LimbType>
	std::string quire_range() {
		std::stringstream str;
	    str << type_tag(quire<NumberType, capacity, LimbType>{}) << " range: ";
		str << "minimum " << std::setw(12) << std::numeric_limits<sw::universal::quire<NumberType, capacity, LimbType>>::min() << "     ";
		str << "maximum " << std::setw(12) << std::numeric_limits<sw::universal::quire<NumberType, capacity, LimbType>>::max() ;
		return str.str();
	}


	template<typename NumberType, unsigned capacity, typename LimbType>
	std::string color_print(const quire<NumberType, capacity, LimbType>& p) {
		std::stringstream str;
		bool		     		_sign;
	
		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color blue(ColorCode::FG_BLUE);
		Color magenta(ColorCode::FG_MAGENTA);
		Color cyan(ColorCode::FG_CYAN);
		Color white(ColorCode::FG_WHITE);
		Color def(ColorCode::FG_DEFAULT);
		str << red << (p.isneg() ? "1" : "0");
	    /*
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
		*/
		str << def;
		return str.str();
	}

}} // namespace sw::universal
