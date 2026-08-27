#pragma once
// manipulators.hpp: definition of manipulation functions for rational types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the rational headers (#1334): the string-producing half. These build a
// std::string and hand it back, so they need <sstream> but not <iostream>; the stream
// insertion and extraction operators are in iostream.hpp.
//
// This header had NO includes at all while using std::string, std::stringstream and the
// stream base-format manipulators -- it compiled only behind a prior include.
#include <string>
#include <sstream>
#include <ios>          // std::oct / std::dec / std::hex, used by to_binary(blockdigit)
#include <cstdint>      // int64_t, the to_binary(blockdigit) fallback
#include <type_traits>  // std::is_same_v, used by type_tag

#include <universal/native/integers.hpp>                   // to_binary(Integer, bool), the base-2 fallback
#include <universal/internal/blockbinary/manipulators.hpp>   // to_binary/to_hex on blockbinary,
                                                             // moved off the arithmetic core (#1334)
#include <universal/number/rational/core.hpp>

namespace sw { namespace universal {

	// Generate a type tag for rational type
	template<unsigned nbits, typename Base, typename bt>
	std::string type_tag(const rational<nbits,Base,bt>& = {}) {
		std::stringstream s;
		if constexpr (std::is_same_v<Base, base2>) {
			s << "rational<" << nbits << ", base2, " << type_tag(bt()) << '>';
		}
		else if constexpr (std::is_same_v<Base, base8>) {
			s << "rational<" << nbits << ", base8>";
		}
		else if constexpr (std::is_same_v<Base, base10>) {
			s << "rational<" << nbits << ", base10>";
		}
		else if constexpr (std::is_same_v<Base, base16>) {
			s << "rational<" << nbits << ", base16>";
		}
		else {
			s << "rational<" << nbits << ", unknown_base>";
		}
		return s.str();
	}

template<unsigned nbits, unsigned base, typename bt>
inline std::string to_binary(const blockdigit<nbits, base, bt>& v, bool nibbleMarker = true) {
	if constexpr (base == 8) {
		std::stringstream s;
		for (unsigned i = 0; i < blockdigit<nbits, 8, bt>::ndigits; ++i) {
			if (nibbleMarker && (i > 0) && (i % 2 == 0)) s << '\'';
			s << std::oct << static_cast<unsigned>(v.digit(i));
		}
		return s.str();
	} else if constexpr (base == 10) {
		std::stringstream s;
		for (unsigned i = 0; i < blockdigit<nbits, 10, bt>::ndigits; ++i) {
			if (nibbleMarker && (i > 0) && (i % 2 == 0)) s << '\'';
			s << std::dec << static_cast<unsigned>(v.digit(i));
		}
		return s.str();
	} else if constexpr (base == 16) {
		std::stringstream s;
		for (unsigned i = 0; i < blockdigit<nbits, 16, bt>::ndigits; ++i) {
			if (nibbleMarker && (i > 0) && (i % 2 == 0)) s << '\'';
			s << std::hex << static_cast<unsigned>(v.digit(i));
		}
		return s.str();
	}
	else {
		return to_binary(static_cast<int64_t>(v), nibbleMarker);
	}
}

template<unsigned nbits, typename Base, typename bt>
inline std::string to_binary(const rational<nbits,Base,bt>& v, bool nibbleMarker = true) {
	std::stringstream s;
	s << to_binary(v.numerator(), nibbleMarker)
		<< " / "
		<< to_binary(v.denominator(), nibbleMarker);
	return s.str();
}

// native semantic representation: rational as N/D in decimal
template<unsigned nbits, typename Base, typename bt>
inline std::string to_native(const rational<nbits,Base,bt>& v, bool = false) {
	std::stringstream s;
	s << v.numerator() << " / " << v.denominator();
	return s.str();
}

}} // namespace sw::universal
