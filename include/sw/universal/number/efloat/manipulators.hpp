#pragma once
// manipulators.hpp: definitions of helper functions for efloat type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the efloat headers (#1334, Phase 2 group 5b, #1455): the <iomanip> half
// -- everything that turns an efloat into a std::string by way of a stringstream.
// iostream.hpp is the <iostream> half. Self-contained.
#include <cstdint>
#include <iomanip>   // std::setw, std::setfill, std::hex, in to_binary()
#include <sstream>
#include <string>
#include <typeinfo>  // for typeid()
#include <universal/number/efloat/core.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

// This file contains functions that manipulate a efloat type
// using efloat number system knowledge.

namespace sw { namespace universal {

// to_binary formatter for efloat to support test reporters
template<unsigned nlimbs>
inline std::string to_binary(const efloat<nlimbs>& number, bool nibbleMarker = false) {
	std::stringstream ss;
	if (number.isnan()) {
		ss << "nan";
	} else if (number.isinf()) {
		ss << (number.sign() == -1 ? "-inf" : "+inf");
	} else if (number.iszero()) {
		ss << "0b0.0.0";
	} else {
		ss << "0b" << (number.sign() == -1 ? "1" : "0") << "."
		   << number.scale() << ".";
		auto limbs = number.bits();
		for (int i = limbs.size() - 1; i >= 0; --i) {
			ss << std::setw(8) << std::setfill('0') << std::hex << limbs[i];
			if (i > 0) ss << "'";
		}
	}
	return ss.str();
}

// Generate a type tag
template<unsigned nlimbs>
std::string type_tag(const efloat<nlimbs>& = {}) {
	return std::string("efloat");
}

// Generate a string representing the efloat components: sign, exponent, faction and value
template<typename EfloatType,
	std::enable_if_t< is_efloat<EfloatType>, bool> = true
>
inline std::string components(const EfloatType& v) {
	std::stringstream s;
	s << (v.sign() ? "(-, " : "(+, ");
	s << v.exponent() << ", ";
	s << "tbd)";
	return s.str();
}

template<typename EfloatType,
	std::enable_if_t< is_efloat<EfloatType>, bool> = true
>
inline std::string to_triple(const EfloatType& v) {
	std::stringstream s;
	s << (v.isneg() ? "(-, " : "(+, ");
	s << v.scale() << ", ";
	bool firstLimb{ true };
	for (auto l : v.bits()) {
		if (!firstLimb) s << '\'';
		uint64_t mask = (1ull << 31);
		for (int i = 31; i >= 0; --i) {
			s << ((l & mask) ? '1' : '0');
			if (firstLimb && i == 31) s << '.';
			if (i > 0 && i % 4 == 0) s << '\'';
			mask >>= 1;
		}
		firstLimb = false;
	}
	s << ')';
	return s.str();
}

// generate a binary string for efloat
template<typename EfloatType,
	std::enable_if_t< is_efloat<EfloatType>, bool> = true
>
inline std::string to_hex(const EfloatType& v, bool nibbleMarker = false, bool hexPrefix = true) {
	std::stringstream s;
	/*
	constexpr char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	if (hexPrefix) s << "0x" << std::hex;
	int nrNibbles = int(1ull + ((nbits - 1ull) >> 2ull));
	for (int n = nrNibbles - 1; n >= 0; --n) {
		uint8_t nibble = v.nibble(unsigned(n));
		s << hexChar[nibble];
		if (nibbleMarker && n > 0 && (n % 4) == 0) s << '\'';
	}
	*/
	s << "tbd";
	return s.str();
}

// generate a efloat format ASCII hex format nbits.esxNN...NNa
template<typename EfloatType,
	std::enable_if_t< is_efloat<EfloatType>, bool> = true
>
inline std::string hex_print(const EfloatType& c) {
	std::stringstream s;
	// s << nbits << '.' << es << 'x' << to_hex(c) << 'c';
	s << "tbd";
	return s.str();
}

template<typename EfloatType,
	std::enable_if_t< is_efloat<EfloatType>, bool> = true
>
inline std::string pretty_print(const EfloatType& r) {
	std::stringstream s;
	s << "tbd";
	return s.str();
}

template<typename EfloatType,
	std::enable_if_t< is_efloat<EfloatType>, bool> = true
>
inline std::string info_print(const EfloatType& p, int printPrecision = 17) {
	return std::string("TBD");
}

// generate a binary, color-coded representation of the efloat
template<typename EfloatType,
	std::enable_if_t< is_efloat<EfloatType>, bool> = true
>
inline std::string color_print(const EfloatType& r, bool nibbleMarker = false) {
	std::stringstream s;
	s << "tbd";
	return s.str();
}


}} // namespace sw::universal
