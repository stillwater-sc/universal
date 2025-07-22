#pragma once
// manipulators.hpp: definition of manipulation functions for bfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <iomanip>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/number/bfloat/bfloat16_fwd.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for bfloat8
	std::string type_tag(const bfloat8 & = {}) {
		return std::string("bfloat8");
	}

	// Generate a type tag for bfloat16
	std::string type_tag(const bfloat16& = {}) {
		return std::string("bfloat16");
	}

	// Generate a type field descriptor for this bfloat
	template<typename BfloatType,
		std::enable_if_t< is_bfloat8<BfloatType>, bool> = true
	>
	inline std::string type_field(const BfloatType & = {}) {
		std::stringstream s;

		//	unsigned nbits = BfloatType::nbits;  // total bits
		unsigned ebits = BfloatType::es;     // exponent bits
		unsigned fbits = BfloatType::fbits;  // integer bits
		s << "fields(s:1|e:" << ebits << "|m:" << fbits << ')';
		return s.str();
	}

	// Generate a type field descriptor for this bfloat
	template<typename BfloatType,
		std::enable_if_t< is_bfloat16<BfloatType>, bool> = true
	>
	inline std::string type_field(const BfloatType & = {}) {
		std::stringstream s;

		//	unsigned nbits = BfloatType::nbits;  // total bits
		unsigned ebits = BfloatType::es;     // exponent bits
		unsigned fbits = BfloatType::fbits;  // integer bits
		s << "fields(s:1|e:" << ebits << "|m:" << fbits << ')';
		return s.str();
	}

	// generate a binary string for bfloat16
	inline std::string to_hex(const bfloat16& v, bool nibbleMarker = false, bool hexPrefix = true) {
		constexpr unsigned nbits = 16;
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

	// generate a bfloat format ASCII hex format nbits.esxNN...NNa
	inline std::string hex_print(const bfloat16& c) {
		constexpr unsigned nbits = 16;
		constexpr unsigned es = 8;
		std::stringstream s;
		s << nbits << '.' << es << 'x' << to_hex(c) << 'c';
		return s.str();
	}

	// return in triple form (sign, scale, fraction)
	inline std::string to_triple(const bfloat16& number, bool nibbleMarker = false) {
		std::stringstream s;

		// print sign bit
		s << '(' << (number.sign() ? '-' : '+') << ',';

		// exponent 
		// the exponent value used in the arithmetic is the exponent shifted by a bias 
		// for a bfloat16 case, an exponent value of 127 represents the actual zero 
		// (i.e. for 2^(e - 127) to be one, e must be 127). 
		// Exponents range from ¿126 to +127 because exponents of ¿127 (all 0s) and +128 (all 1s) are reserved for special numbers.
		uint32_t exponent = number.exponent();
		if (exponent == 0u) {
			s << "exp=0,";
		}
		else if (exponent == 0xFFu) {
			s << "exp=1, ";
		}
		int scale = int(exponent) - 127;
		s << scale << ",0b";

		// print fraction bits
		uint32_t fraction = number.fraction();
		uint32_t mask = (uint32_t(1) << 6);
		for (int i = 6; i >= 0; --i) {
			s << ((fraction & mask) ? '1' : '0');
			if (nibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
			mask >>= 1;
		}

		s << ')';
		return s.str();
	}

	// generate a binary, color-coded representation of the bfloat16
	std::string color_print(const bfloat16& r, bool nibbleMarker = false) {
		constexpr unsigned es = 8;
		constexpr unsigned fbits = 7;
		std::stringstream s;
		bool sign{ false };
		blockbinary<es> e{0};
		blockbinary<fbits + 1> f{0};
		sign = r.sign();
		e = r.exponent();
		f = r.fraction();

		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color blue(ColorCode::FG_BLUE);
		Color magenta(ColorCode::FG_MAGENTA);
		Color cyan(ColorCode::FG_CYAN);
		Color white(ColorCode::FG_WHITE);
		Color def(ColorCode::FG_DEFAULT);

		// sign bit
		s << red << (sign ? '1' : '0');

		// exponent bits
		for (int i = int(es) - 1; i >= 0; --i) {
			s << cyan << (e.test(static_cast<size_t>(i)) ? '1' : '0');
			if ((i - es) > 0 && ((i - es) % 4) == 0 && nibbleMarker) s << yellow << '\'';
		}

		// fraction bits
		for (int i = int(fbits) - 1; i >= 0; --i) {
			s << magenta << (f.test(static_cast<size_t>(i)) ? '1' : '0');
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << yellow << '\'';
		}

		s << def;
		return s.str();
	}


}} // namespace sw::universal
