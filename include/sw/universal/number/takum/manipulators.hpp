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
			<< std::setw(1) << TakumType::rbits << ", "
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

	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline bool isInRange(double v) {
		TakumType a{};
		bool inside = true;
		if (v > double(a.maxpos()) || v < double(a.maxneg())) inside = false;
		return inside;
	}

	// Generate a string representing the takum components
	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline std::string components(const TakumType& v) {
		std::stringstream s;
		if (v.iszero()) {
			s << " zero " << to_binary(v);
			return s.str();
		}
		if (v.isnar()) {
			s << " NaR " << to_binary(v);
			return s.str();
		}
		s << std::setw(14) << to_binary(v)
			<< " Sign : " << std::setw(2) << v.sign()
			<< " Characteristic : " << std::setw(5) << v.characteristic()
			<< " Scale : " << std::setw(5) << v.scale()
			<< " Value : " << std::setw(16) << double(v);
		return s.str();
	}

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
		constexpr unsigned nbits = TakumType::nbits;
		constexpr unsigned rbits = TakumType::rbits;
		uint64_t mag = number.magnitude_bits();
		std::stringstream s;
		// sign bit
		s << (number.sign() ? '1' : '0');
		s << '.';
		// direction bit from magnitude
		bool D = static_cast<bool>((mag >> (nbits - 2)) & 1);
		s << (D ? '1' : '0');
		s << '.';
		// regime field from magnitude
		unsigned regime = static_cast<unsigned>((mag >> (nbits - TakumType::overhead)) & TakumType::r_mask);
		for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
			s << ((regime >> i) & 1 ? '1' : '0');
		}
		s << '.';
		// characteristic and mantissa fields
		unsigned dr = (D ? (1u << rbits) : 0) + regime;
		unsigned r_val = TakumType::dr_to_r(dr);
		unsigned avail = TakumType::maxCharBits;
		unsigned c_stored = (r_val < avail) ? r_val : avail;
		unsigned p = (r_val < avail) ? (avail - r_val) : 0;
		int bit = static_cast<int>(nbits) - static_cast<int>(TakumType::overhead) - 1;
		for (unsigned i = 0; i < c_stored && bit >= 0; ++i) {
			s << ((mag >> bit) & 1 ? '1' : '0');
			--bit;
		}
		s << '.';
		for (unsigned i = 0; i < p && bit >= 0; ++i) {
			s << ((mag >> bit) & 1 ? '1' : '0');
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
		constexpr unsigned nbits = TakumType::nbits;
		constexpr unsigned rbits = TakumType::rbits;
		uint64_t mag = number.magnitude_bits();

		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color green(ColorCode::FG_GREEN);
		Color magenta(ColorCode::FG_MAGENTA);
		Color cyan(ColorCode::FG_CYAN);
		Color def(ColorCode::FG_DEFAULT);

		std::stringstream s;
		// sign bit
		s << red << (number.sign() ? '1' : '0');
		// direction bit from magnitude
		bool D = static_cast<bool>((mag >> (nbits - 2)) & 1);
		s << green << (D ? '1' : '0');
		// regime field from magnitude
		s << yellow;
		unsigned regime = static_cast<unsigned>((mag >> (nbits - TakumType::overhead)) & TakumType::r_mask);
		for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
			s << ((regime >> i) & 1 ? '1' : '0');
		}
		// characteristic and mantissa fields
		unsigned dr = (D ? (1u << rbits) : 0) + regime;
		unsigned r_val = TakumType::dr_to_r(dr);
		unsigned avail = TakumType::maxCharBits;
		unsigned c_stored = (r_val < avail) ? r_val : avail;
		unsigned p = (r_val < avail) ? (avail - r_val) : 0;
		s << cyan;
		int bit = static_cast<int>(nbits) - static_cast<int>(TakumType::overhead) - 1;
		for (unsigned i = 0; i < c_stored && bit >= 0; ++i) {
			s << ((mag >> bit) & 1 ? '1' : '0');
			--bit;
		}
		s << magenta;
		for (unsigned i = 0; i < p && bit >= 0; ++i) {
			s << ((mag >> bit) & 1 ? '1' : '0');
			if (bit > 0 && (bit % 4) == 0 && nibbleMarker) s << '\'';
			--bit;
		}
		s << def;
		return s.str();
	}

}} // namespace sw::universal
