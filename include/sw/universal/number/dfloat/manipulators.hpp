#pragma once
// manipulators.hpp: definitions of helper functions for decimal dfloat type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the dfloat headers (#1334): the <iomanip> half -- everything that turns a
// dfloat into a std::string through a stringstream. iostream.hpp is the <iostream> half.
// Self-contained: it names every header it uses rather than relying on the umbrella's
// include order, which is how it reached Color, type_tag and <sstream> before.
#include <string>        // std::string
#include <sstream>       // std::stringstream
#include <iomanip>       // std::setw
// pull in type_tag overloads for native integer block types
#include <universal/native/integer_type_tag.hpp>
#include <universal/number/dfloat/core.hpp>
// blockbinary's own string producers (type_tag, to_binary, to_hex). dfloat_impl.hpp used
// to include this and so handed them to every dfloat translation unit; the core now takes
// only to_decimal.hpp, so the include lives here to keep the umbrella's surface identical
// (#1334).
#include <universal/internal/blockbinary/manipulators.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {


	
	// Generate a type tag for this decimal encoding
	inline std::string type_tag(const DecimalEncoding encoding = {}) {
		std::stringstream s;
		switch (encoding) {
		case DecimalEncoding::BCD:
		    s << "BCD";  // Binary-Coded Decimal: 4 bits per digit
			break;
		case DecimalEncoding::BID:
			s << "BID";  // Binary Integer Decimal: significand stored as binary integer
			break;
		case DecimalEncoding::DPD:
			s << "DPD";  // Densely Packed Decimal: significand stored as 10-bit declets
			break;
		default:
			s << "UnknownDecimalEncoding";
			break;
		}
		return s.str();
	}

	// Generate a type tag for this dfloat
	template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
	std::string type_tag(const dfloat<ndigits, es, Encoding, bt>& = {}) {
		std::stringstream s;
		s << "dfloat<"
			<< std::setw(3) << ndigits << ", "
			<< std::setw(3) << es << ", "
			<< type_tag(Encoding) << ", "
			<< type_tag(bt{}) << '>';
		return s.str();
	}

	// type field descriptor
	template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
	std::string type_field(const dfloat<ndigits, es, Encoding, bt>& = {}) {
		using Dfloat = dfloat<ndigits, es, Encoding, bt>;
		std::stringstream s;
		s << "fields(s:1|comb:" << Dfloat::combBits
		  << "|exp:" << es
		  << "|trail:" << Dfloat::t << ')';
		return s.str();
	}

	// color print: show sign, combination, exponent, trailing with ANSI colors
	template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
	std::string color_print(const dfloat<ndigits, es, Encoding, bt>& number, bool nibbleMarker = false) {
		using Dfloat = dfloat<ndigits, es, Encoding, bt>;
		std::stringstream s;

		Color red(ColorCode::FG_RED);
	    Color yellow(ColorCode::FG_YELLOW);
	    Color magenta(ColorCode::FG_MAGENTA);
	    Color cyan(ColorCode::FG_CYAN);
	    Color def(ColorCode::FG_DEFAULT);

		s << red << (number.sign() ? '1' : '0') << def;

		unsigned combStart = Dfloat::nbits - 2;
		for (unsigned i = 0; i < Dfloat::combBits; ++i) {
			s << yellow << (number.getbit(combStart - i) ? '1' : '0');
		}

		// exponent 
		unsigned expStart = Dfloat::nbits - 1 - 1 - Dfloat::combBits;
		for (unsigned i = 0; i < es; ++i) {
			s << cyan <<(number.getbit(expStart - i) ? '1' : '0');
		}

		// trailing significand
		for (int i = static_cast<int>(Dfloat::t) - 1; i >= 0; --i) {
			s << magenta <<(number.getbit(static_cast<unsigned>(i)) ? '1' : '0');
			if (nibbleMarker && i > 0 && (i % 4 == 0)) s << yellow << '\'';
		}
	    s << def;

		return s.str();
	}

	// components: show unpacked sign, exponent, significand
	template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
	std::string components(const dfloat<ndigits, es, Encoding, bt>& number) {
		using Dfloat = dfloat<ndigits, es, Encoding, bt>;
		std::stringstream s;
		s << "sign: " << (number.sign() ? '-' : '+');
		if (number.isnan()) {
			s << ", nan";
		}
		else if (number.isinf()) {
			s << ", inf";
		}
		else if (number.iszero()) {
			s << ", zero";
		}
		else {
			bool sign; int exp; typename Dfloat::significand_t sig;
			number.unpack(sign, exp, sig);
			s << ", decimal scale: " << exp
			  << ", coefficient: " << Dfloat::sig_to_string(sig);
		}
		return s.str();
	}


// Moved out of dfloat_impl.hpp (#1334): these format a dfloat through a stringstream,
// which is what keeps them out of the core.

template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline std::string to_binary(const dfloat<ndigits, es, Encoding, BlockType>& number, bool nibbleMarker = false) {
	using Dfloat = dfloat<ndigits, es, Encoding, BlockType>;
	std::stringstream s;

	// sign bit
	s << "0b" << (number.sign() ? '1' : '0') << '.';

	// combination field (5 bits)
	unsigned combStart = Dfloat::nbits - 2;
	for (unsigned i = 0; i < Dfloat::combBits; ++i) {
		s << (number.getbit(combStart - i) ? '1' : '0');
	}
	s << '.';

	// exponent continuation (es bits)
	unsigned expStart = Dfloat::nbits - 1 - 1 - Dfloat::combBits;
	for (unsigned i = 0; i < es; ++i) {
		s << (number.getbit(expStart - i) ? '1' : '0');
	}
	s << '.';

	// trailing significand (t bits, MSB first)
	for (int i = static_cast<int>(Dfloat::t) - 1; i >= 0; --i) {
		s << (number.getbit(static_cast<unsigned>(i)) ? '1' : '0');
		if (nibbleMarker && i > 0 && (i % 4 == 0)) s << '\'';
	}

	return s.str();
}

// native semantic representation: radix-10, shows decimal coefficient and exponent
// Format: +DDDDDDDDDDDDDDDDe+EEE (fixed-width for visual alignment)
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
inline std::string to_native(const dfloat<ndigits, es, Encoding, BlockType>& number, bool = false) {
	using Dfloat = dfloat<ndigits, es, Encoding, BlockType>;
	std::stringstream s;

	if (number.isnan()) { s << "NaN"; return s.str(); }
	if (number.isinf()) { s << (number.sign() ? "-inf" : "+inf"); return s.str(); }
	if (number.iszero()) {
		s << (number.sign() ? '-' : '+');
		s << std::string(ndigits, '0') << "e+0";
		return s.str();
	}

	bool sign; int exp; typename Dfloat::significand_t sig;
	number.unpack(sign, exp, sig);

	s << (sign ? '-' : '+');

	// Convert significand to decimal string, left-pad to ndigits
	std::string digits = Dfloat::sig_to_string(sig);
	while (digits.size() < ndigits) digits = "0" + digits;

	s << digits << 'e' << std::showpos << exp;
	return s.str();
}

}} // namespace sw::universal
