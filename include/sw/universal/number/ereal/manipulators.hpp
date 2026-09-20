#pragma once
// manipulators.hpp: definitions of helper functions for ereal type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the ereal headers (#1334, Phase 2 group 5b, #1455): the <iomanip> half
// -- everything that turns an ereal into a std::string by way of a stringstream.
// Self-contained.
//
// The dependency runs manipulators -> iostream here, as it does for dd and qd:
// to_triple() streams the fraction it gets from frexp(), so it needs operator<<.
// iostream.hpp does not include this header back.
#include <cstddef>   // std::size_t
#include <iomanip>   // std::setw
#include <sstream>
#include <string>
#include <type_traits>
#include <universal/number/ereal/core.hpp>                  // frexp(ereal), for to_triple(), comes with it
#include <universal/number/ereal/iostream.hpp>              // operator<<, for to_triple()
#include <universal/native/ieee754.hpp>                     // to_binary(double), for to_binary()
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

// This file contains functions that manipulate a cfloat type
// using cfloat number system knowledge.

namespace sw { namespace universal {

// Generate a type tag
// ereal<8> for the default double limbs, as always; the limb type is named otherwise:
// ereal<5, float>, ereal<24, long double>
template<unsigned nlimbs, typename FpType>
std::string type_tag(const ereal<nlimbs, FpType>& = {}) {
	std::string limb;
	if constexpr (std::is_same_v<FpType, float>) limb = ", float";
	else if constexpr (std::is_same_v<FpType, long double>) limb = ", long double";
	return std::string("ereal<") + std::to_string(nlimbs) + limb + std::string(">");
}

// Generate a string representing the ereal components: sign, exponent, faction and value
template<typename ErealType,
	std::enable_if_t< is_ereal<ErealType>, bool> = true
>
inline std::string to_components(const ErealType& v) {
	std::stringstream s;
	s << "( ";
	for (std::size_t i = 0; i < v.limbs().size(); ++i) {
		s << std::setw(17) << v.limbs()[i];
		if (i < v.limbs().size() - 1) s << ", ";
	}
	s << " )";
	return s.str();
}

template<typename ErealType,
	std::enable_if_t< is_ereal<ErealType>, bool> = true
>
inline std::string to_triple(const ErealType& v) {
	std::stringstream s;
	s << (v.isneg() ? "(-, " : "(+, ");
	s << v.scale() << ", ";
	int e{ 0 };
	ErealType f = frexp(v, &e);
	s << f;
	s << ')';
	return s.str();
}

template<typename ErealType,
         std::enable_if_t<is_ereal<ErealType>, bool> = true>
inline std::string to_binary(const ErealType& v, bool nibbleMarker = false) {
	std::stringstream s;
	// present first and last limb in binary
	std::size_t firstLimb = 0;
	std::size_t lastLimb  = v.limbs().size() - 1;
	if (firstLimb == lastLimb) {
		// only one limb
		s << to_binary(v.limbs()[firstLimb], nibbleMarker);
	} else {
		s << to_binary(v.limbs()[firstLimb], nibbleMarker) << " ... " << to_binary(v.limbs()[lastLimb], nibbleMarker);
	}
	return s.str();
}

         // generate a hex string for ereal
template<typename ErealType,
	std::enable_if_t< is_ereal<ErealType>, bool> = true
>
inline std::string to_hex(const ErealType& v, bool nibbleMarker = false, bool hexPrefix = true) {
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

// generate a ereal format ASCII hex format nbits.esxNN...NNa
template<typename ErealType,
	std::enable_if_t< is_ereal<ErealType>, bool> = true
>
inline std::string hex_print(const ErealType& c) {
	std::stringstream s;
	// s << nbits << '.' << es << 'x' << to_hex(c) << 'c';
	s << "tbd";
	return s.str();
}

template<typename ErealType,
	std::enable_if_t< is_ereal<ErealType>, bool> = true
>
inline std::string pretty_print(const ErealType& r) {
	std::stringstream s;
	s << "tbd";
	return s.str();
}

// Report the state, the limb layout and the value of an ereal.
//
// It returned the literal "tbd" and ignored both of its arguments (#1556). The shape is
// posit's -- "raw: <bits> <fields> : value <v>" (posit/iostream.hpp) -- but an ereal is a
// multi-component expansion: there is no fixed sign/exponent/fraction bit layout to
// decode, so the fields are the ones that do vary, the limb count and the binary scale,
// and "raw" is the leading and trailing limb that to_binary() already reports.
//
// Unlike the other manipulator layers this one already depends on iostream.hpp -- as dd
// and qd do, for to_triple() -- so the value field can be streamed directly.
template<typename ErealType,
	std::enable_if_t< is_ereal<ErealType>, bool> = true
>
inline std::string info_print(const ErealType& p, int printPrecision = 17) {
	std::stringstream s;
	s << "raw: " << to_binary(p) << ' ' << ((p.sign() == -1) ? "s1" : "s0");
	if (p.isnan())       s << " nan";
	else if (p.isinf())  s << " inf";
	else if (p.iszero()) s << " zero";
	else s << " limbs " << p.limbs().size() << " scale " << p.scale();
	s << " : value " << std::setprecision(printPrecision) << p;
	return s.str();
}

// generate a binary, color-coded representation of the ereal
template<typename ErealType,
	std::enable_if_t< is_ereal<ErealType>, bool> = true
>
inline std::string color_print(const ErealType& r, bool nibbleMarker = false) {
	std::stringstream s;
	s << "tbd";
	return s.str();
}

}} // namespace sw::universal
