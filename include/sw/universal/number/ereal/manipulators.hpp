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
	// An ereal may hold NO limbs at all: renormalize_expansion prunes components that
	// cancel, and expansion_sum_normalized returns its result without the canonical-zero
	// push that expansion_product does, so the empty vector reaches _limb. ereal's own
	// to_string guards it in as many words ("renormalize_expansion can prune all
	// components to empty"), as do iszero, isinf, isnan, sign, scale and significant --
	// this was the one accessor that did not, and size() - 1 on an empty vector is an
	// unsigned wrap into an out-of-bounds read. info_print is its first caller (#1556).
	if (v.limbs().empty()) {
		s << to_binary(typename ErealType::limb_type(0), nibbleMarker);
		return s.str();
	}
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

// Generate a hex string for an ereal: the hex of each limb, most significant first.
//
// It returned the literal "tbd" over a commented-out body that indexed a nibble(n) and an
// nbits that an ereal does not have (#1582). An ereal is a multi-component expansion, so
// it is rendered the way dd, qd and dd_cascade render theirs: delegate to the limbs' own
// native rendering and join them (dd/manipulators.hpp, dd_cascade/manipulators.hpp). The
// limbs ARE floating-point values, so unlike efloat there is a native to_hex to hand
// them to.
template<typename ErealType,
	std::enable_if_t< is_ereal<ErealType>, bool> = true
>
inline std::string to_hex(const ErealType& v, bool nibbleMarker = false, bool hexPrefix = true) {
	std::stringstream s;
	s << "ereal[";
	const auto& limbs = v.limbs();
	if (limbs.empty()) {
		// an ereal with no limbs is semantically zero: renormalize_expansion prunes
		// components that cancel, and nothing canonicalises the empty result (#1556)
		s << to_hex(typename ErealType::limb_type(0), nibbleMarker, hexPrefix);
	}
	else {
		for (std::size_t i = 0; i < limbs.size(); ++i) {
			if (i > 0) s << ", ";
			s << to_hex(limbs[i], nibbleMarker, hexPrefix);
		}
	}
	s << ']';
	return s.str();
}

// Generate an ereal format ASCII hex format, the analogue of cfloat's nbits.esxNN...NNc.
//
// It returned the literal "tbd" (#1582). An ereal has no nbits or es to name, so the
// configuration field is the limb count. The type letter is uppercase 'R' because areal
// has already taken lowercase 'r'.
template<typename ErealType,
	std::enable_if_t< is_ereal<ErealType>, bool> = true
>
inline std::string hex_print(const ErealType& c) {
	std::stringstream s;
	s << c.limbs().size() << 'x' << to_hex(c) << 'R';
	return s.str();
}

// Render each limb's fields separated, the way the fixed-size types render theirs.
//
// It returned the literal "tbd" (#1582). pretty_print(native) gives each limb the
// sign:exponent:fraction form cfloat uses, so an ereal reads as the expansion of
// IEEE-754 values that it is.
template<typename ErealType,
	std::enable_if_t< is_ereal<ErealType>, bool> = true
>
inline std::string pretty_print(const ErealType& r) {
	std::stringstream s;
	s << "ereal[";
	const auto& limbs = r.limbs();
	if (limbs.empty()) {
		s << pretty_print(typename ErealType::limb_type(0));
	}
	else {
		for (std::size_t i = 0; i < limbs.size(); ++i) {
			if (i > 0) s << ", ";
			s << pretty_print(limbs[i]);
		}
	}
	s << ']';
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

// Generate a binary, color-coded representation of the ereal.
//
// It returned the literal "tbd" (#1582). dd does exactly this -- color_print<double>(high)
// then color_print<double>(low), comma-joined (dd/manipulators.hpp) -- and the ereal api
// walk-through already colour-prints an expansion by hand, one limb at a time
// (elastic/ereal/api/api.cpp). This is that, for a limb count known only at run time.
template<typename ErealType,
	std::enable_if_t< is_ereal<ErealType>, bool> = true
>
inline std::string color_print(const ErealType& r, bool nibbleMarker = false) {
	std::stringstream s;
	s << "ereal[";
	const auto& limbs = r.limbs();
	if (limbs.empty()) {
		s << color_print(typename ErealType::limb_type(0), nibbleMarker);
	}
	else {
		for (std::size_t i = 0; i < limbs.size(); ++i) {
			if (i > 0) s << ", ";
			s << color_print(limbs[i], nibbleMarker);
		}
	}
	s << ']';
	return s.str();
}

}} // namespace sw::universal
