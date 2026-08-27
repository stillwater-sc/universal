#pragma once
// manipulators.hpp: definitions of helper functions for takum number manipulation
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>       // std::stringstream
#include <string>        // std::string
#include <cstddef>       // std::size_t
#include <universal/number/takum/core.hpp>
#include <cstdint>       // the fixed-width integer types
#include <type_traits>   // std::enable_if_t
#include <iomanip>
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {
template<unsigned nbits, unsigned rbits, typename bt>
std::string to_native(const takum<nbits, rbits, bt>& number, bool nibbleMarker = false) {
	return to_binary(number, nibbleMarker);
}

template<unsigned nbits, unsigned rbits, typename bt>
std::string to_native(const takum_log<nbits, rbits, bt>& number, bool nibbleMarker = false) {
	return to_binary(number, nibbleMarker);
}


// Moved out of the impl headers (#1334): these turn a takum into a std::string.
template<unsigned nbits, unsigned rbits, typename bt>
std::string to_binary(const takum<nbits, rbits, bt>& number, bool nibbleMarker = false) {
	using T = takum<nbits, rbits, bt>;
	std::stringstream s;
	bool negative = number.sign();
	uint64_t mag = number.magnitude_bits();

	s << "0b";
	s << (negative ? "1." : "0.");

	// Direction bit from magnitude
	bool D = static_cast<bool>((mag >> (nbits - 2)) & 1);
	s << (D ? "1." : "0.");

	// Regime field from magnitude (rbits bits)
	unsigned regime = static_cast<unsigned>((mag >> (nbits - T::overhead)) & T::r_mask);
	for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
		s << ((regime >> i) & 1 ? '1' : '0');
	}
	s << '.';

	// Characteristic and mantissa bits (geometry from the shared codec)
	auto g = T::Codec::layout_of(number.dr_field());
	unsigned p = g.p;
	unsigned c_stored = g.c_stored_bits;
	int bit = static_cast<int>(nbits) - static_cast<int>(T::overhead) - 1;

	for (unsigned i = 0; i < c_stored && bit >= 0; ++i) {
		s << ((mag >> bit) & 1 ? '1' : '0');
		--bit;
		if (i < c_stored - 1 && ((c_stored - 1 - i) % 4) == 0 && nibbleMarker) s << '\'';
	}
	s << '.';
	for (unsigned i = 0; i < p && bit >= 0; ++i) {
		s << ((mag >> bit) & 1 ? '1' : '0');
		if (bit > 0 && (bit % 4) == 0 && nibbleMarker) s << '\'';
		--bit;
	}

	return s.str();
}

template<unsigned nbits, unsigned rbits, typename bt>
std::string to_binary(const takum_log<nbits, rbits, bt>& number, bool nibbleMarker = false) {
	using T = takum_log<nbits, rbits, bt>;
	std::stringstream s;
	uint64_t mag = number.magnitude_bits();

	s << "0b";
	s << (number.sign() ? "1." : "0.");
	bool D = static_cast<bool>((mag >> (nbits - 2)) & 1);
	s << (D ? "1." : "0.");

	unsigned regime = static_cast<unsigned>((mag >> (nbits - T::overhead)) & T::r_mask);
	for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) s << ((regime >> i) & 1 ? '1' : '0');
	s << '.';

	auto g = T::Codec::layout_of(number.dr_field());
	int bit = static_cast<int>(nbits) - static_cast<int>(T::overhead) - 1;
	for (unsigned i = 0; i < g.c_stored_bits && bit >= 0; ++i) {
		s << ((mag >> bit) & 1 ? '1' : '0');
		--bit;
		if (i < g.c_stored_bits - 1 && ((g.c_stored_bits - 1 - i) % 4) == 0 && nibbleMarker) s << '\'';
	}
	s << '.';
	for (unsigned i = 0; i < g.p && bit >= 0; ++i) {
		s << ((mag >> bit) & 1 ? '1' : '0');
		if (bit > 0 && (bit % 4) == 0 && nibbleMarker) s << '\'';
		--bit;
	}
	return s.str();
}


	// Generate a type tag for this takum
	template<typename TakumType,
		std::enable_if_t< is_any_takum<TakumType>, bool> = true
	>
	inline std::string type_tag(const TakumType & = {}) {
		std::stringstream s;
		typename TakumType::BlockType bt{0};
		// name the variant: the two share an encoding but are different number systems
		s << (is_takum_log<TakumType> ? "takum_log<" : "takum<")
			<< std::setw(3) << TakumType::nbits << ", "
			<< std::setw(1) << TakumType::rbits << ", "
			<< type_tag(bt) << ">";
		return s.str();
	}

	template<typename TakumType,
		std::enable_if_t< is_any_takum<TakumType>, bool> = true
	>
	inline std::string range(const TakumType & = {}) {
		std::stringstream s;
		TakumType b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
		s << "[" << b << " ... " << c << ", 0, " << d << " ... " << e << "]\n";
		return s.str();
	}

	template<typename TakumType,
		std::enable_if_t< is_any_takum<TakumType>, bool> = true
	>
	inline bool isInRange(double v) {
		TakumType a{};
		bool inside = true;
		if (v > double(a.maxpos()) || v < double(a.maxneg())) inside = false;
		return inside;
	}

	// Generate a string representing the takum components
	template<typename TakumType,
		std::enable_if_t< is_any_takum<TakumType>, bool> = true
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
		std::enable_if_t< is_any_takum<TakumType>, bool> = true
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
		std::enable_if_t< is_any_takum<TakumType>, bool> = true
	>
	inline std::string hex_print(const TakumType& c) {
		constexpr unsigned nbits = TakumType::nbits;
		std::stringstream s;
		s << nbits << 'x' << to_hex(c) << 't';
		return s.str();
	}

	template<typename TakumType,
		std::enable_if_t< is_any_takum<TakumType>, bool> = true
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
		// characteristic and mantissa fields (geometry from the shared codec)
		auto g = TakumType::Codec::layout_of(number.dr_field());
		unsigned c_stored = g.c_stored_bits;
		unsigned p = g.p;
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
		std::enable_if_t< is_any_takum<TakumType>, bool> = true
	>
	inline std::string info_print(const TakumType& p, int printPrecision = 17) {
		return std::string("TBD");
	}

	template<typename TakumType,
		std::enable_if_t< is_any_takum<TakumType>, bool> = true
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
		// characteristic and mantissa fields (geometry from the shared codec)
		auto g = TakumType::Codec::layout_of(number.dr_field());
		unsigned c_stored = g.c_stored_bits;
		unsigned p = g.p;
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
