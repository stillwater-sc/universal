#pragma once
// manipulators.hpp: definitions of helper functions for takum number manipulation
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for this takum
	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline std::string type_tag(const TakumType & = {}) {
		std::stringstream s;
		typename TakumType::BlockType bt{0};
		s << "takum<"
			<< std::setw(3) << TakumType::nbits << ", "
			<< type_tag(bt) << ">";
		return s.str();
	}

	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline std::string range(const TakumType & = {}) {
		std::stringstream s;
		TakumType b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
		s << "[" << b << " ... " << c << ", 0, " << d << " ... " << e << "]\n";
		return s.str();
	}

	// report if a native floating-point value is within the dynamic range of the takum configuration
	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline bool isInRange(double v) {
		TakumType a{};

		bool inside = true;
		if (v > double(a.maxpos()) || v < double(a.maxneg())) inside = false;
		return inside;
	}


	// Generate a string representing the cfloat components: sign, exponent, faction and value
	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline std::string components(const TakumType& v) {
		constexpr unsigned es    = TakumType::es;
		constexpr unsigned fbits = TakumType::fbits;
		using bt = typename TakumType::BlockType;

		bool sign{ false };
		blockbinary<es, bt> e;
		blockbinary<fbits, bt> f;
		decode(v, sign, e, f);

		// TODO: hardcoded field width is governed by pretty printing cfloat tables, which by construction will always be small cfloats
		std::stringstream s; 
		s << std::setw(14) << to_binary(v)
			<< " Sign : " << std::setw(2) << sign
			<< " Exponent : " << std::setw(5) << e
			<< " Fraction : " << std::setw(8) << f
			<< " Value : " << std::setw(16) << v;

		return s.str();
	}

	// generate a binary string for cfloat
	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline std::string to_hex(const TakumType& v, bool nibbleMarker = false, bool hexPrefix = true) {
		constexpr unsigned nbits = TakumType::nbits; 
		constexpr char hexChar[16] = {
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

	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline std::string hex_print(const TakumType& c) {
		constexpr unsigned nbits = TakumType::nbits;
		
		std::stringstream s;
		s << nbits << 'x' << to_hex(c) << 't';
		return s.str();
	}

	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline std::string pretty_print(const TakumType& number, bool nibbleMarker = false) {
		std::stringstream s;
		// sign bit
		s << (number.sign() ? '1' : '0');
		s << '.';
		// direction bit
		bool D = number.direct();
		s << (D ? '1' : '0');
		s << '.';
		// regime field
		int bit = static_cast<int>(TakumType::nbits) - 3;
		for (int i = 0; (i < 3) && (bit >= 0); ++i) {
			s << (number.at(static_cast<unsigned>(bit--)) ? '1' : '0');
		}
		s << '.';
		// exponent field
		unsigned regime = number.regime();
		int r = static_cast<int>(D ? regime : 7 - regime);
		for (int i = r - 1; i >= 0 && bit >= 0; --i) {
			s << (number.at(static_cast<unsigned>(bit--)) ? '1' : '0');
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
		}
		s << '.';
		// fraction field
		while (bit >= 0) {
			s << (number.at(static_cast<unsigned>(bit)) ? '1' : '0');
			if (bit > 0 && (bit % 4) == 0 && nibbleMarker) s << '\'';
			--bit;
		}
		return s.str();
	}

	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline std::string info_print(const TakumType& p, int printPrecision = 17) {
		return std::string("TBD");
	}

	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline std::string color_print(const TakumType& number, bool nibbleMarker = false) {
		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color blue(ColorCode::FG_BLUE);
		Color green(ColorCode::FG_GREEN);
		Color magenta(ColorCode::FG_MAGENTA);
		Color cyan(ColorCode::FG_CYAN);
		Color white(ColorCode::FG_WHITE);
		Color def(ColorCode::FG_DEFAULT);


		std::stringstream s;
		// sign bit
		s << red << (number.sign() ? '1' : '0');
		// direction bit
		bool D = number.direct();
		s << green << (D ? '1' : '0');
		// regime field
		s << yellow;
		int bit = static_cast<int>(TakumType::nbits) - 3;
		for (int i = 0; (i < 3) && (bit >= 0); ++i) {
			s << (number.at(static_cast<unsigned>(bit--)) ? '1' : '0');
		}
		// exponent field
		s << cyan;
		unsigned regime = number.regime();
		int r = static_cast<int>(D ? regime : 7 - regime);
		for (int i = r - 1; i >= 0 && bit >= 0; --i) {
			s << (number.at(static_cast<unsigned>(bit--)) ? '1' : '0');
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
		}
		// fraction field
		s << magenta;
		while (bit >= 0) {
			s << (number.at(static_cast<unsigned>(bit)) ? '1' : '0');
			if (bit > 0 && (bit % 4) == 0 && nibbleMarker) s << '\'';
			--bit;

		}
		s << def;
		return s.str();
	}

}} // namespace sw::universal
