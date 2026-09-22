#pragma once
// manipulators.hpp: definitions of helper functions for double base number system value manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the dbns headers (#1334): the <iomanip> half -- everything that turns a
// dbns into a std::string through a stringstream. iostream.hpp is the <iostream> half.
// This header does NOT include iostream.hpp: dbns's pretty_print/info_print build from
// the bit pattern rather than streaming the value, so the dependency runs one way only.
// Self-contained.
#include <string>        // std::string
#include <sstream>       // std::stringstream
#include <iomanip>
#include <typeinfo>

#include <universal/number/dbns/core.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for this double base number system
	template<typename DbnsType,
		std::enable_if_t< is_dbns<DbnsType>, bool> = true
	>
	inline std::string type_tag(const DbnsType& = {}) {
		std::stringstream s;
		s << "dbns<"
			<< std::setw(3) << DbnsType::nbits << ", "
			<< std::setw(3) << DbnsType::fbbits << ", "
			<< typeid(typename DbnsType::BlockType).name() << ", "
			<< std::setw(10) << type_tag(DbnsType::behavior) << '>';
		return s.str();
	}

	// Generate a type field descriptor for this lns
	template<typename DbnsType,
		std::enable_if_t< is_dbns<DbnsType>, bool> = true
	>
	inline std::string type_field(const DbnsType & = {}) {
		std::stringstream s;
		unsigned fbbits = DbnsType::fbbits;  // first exponent bits
		unsigned sbbits = DbnsType::sbbits;  // second exponent bits
		s << "fields(s:1|e1:" << fbbits << "|e2:" << sbbits << ')';
		return s.str();
	}

	template<typename DbnsType,
		std::enable_if_t< is_dbns<DbnsType>, bool> = true
	>
	inline std::string range(const DbnsType & = {}) {
		std::stringstream s;
		DbnsType b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
		// stream the double conversions rather than the dbns values themselves: this
		// layer must not depend on iostream.hpp (#1334), and operator<<(ostream, dbns)
		// lives there. It is `ostr << double(r)`, so this is the same text -- and the
		// same text is what the differential in the PR checks.
		s << "[" << double(b) << " ... " << double(c) << ", 0, " << double(d) << " ... " << double(e) << "]\n";
		return s.str();
	}

	// report if a native floating-point value is within the dynamic range of the dbns configuration
	template<typename DbnsType,
		std::enable_if_t< is_dbns<DbnsType>, bool> = true
	>
	inline constexpr bool isInRange(double v) noexcept {
		DbnsType a{};

		bool inside = true;
		if (v > double(a.maxpos()) || v < double(a.maxneg())) inside = false;
		return inside;
	}

	// (sign, scale, base exponents) -- the third field used to be fraction(), which is
	// hard-coded to return 0 for every dbns value, so this reported a constant 0 as though
	// it were a decoded field (#1582). A dbns has no fraction: what it actually carries is
	// the pair of base exponents, so that is what is reported.
	template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
	std::string to_triple(const dbns<nbits, fbbits, bt, xtra...>& v, [[maybe_unused]] bool nibbleMarker = false) {
		std::stringstream s;
		s << (v.sign() ? "(-, " : "(+, ");
		s << v.scale() << ", ";
		s << "e0=" << v.extractExponent(0) << " e1=" << v.extractExponent(1) << ')';
		return s.str();
	}

	template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
	std::string components(const dbns<nbits, fbbits, bt, xtra...>& v) {
		std::stringstream s;
		if (v.iszero()) {
			s << " zero " << to_binary(v);
			return s.str();
		}
		else if (v.isinf()) {
			s << " infinite " << to_binary(v);
			return s.str();
		}
		s << "(" << (v.sign() ? "-" : "+")
		  << "," << v.scale()
		  << ",e0=" << v.extractExponent(0)
		  << ",e1=" << v.extractExponent(1) << ")";
		return s.str();
	}

	template<typename DbnsType,
		std::enable_if_t< is_dbns<DbnsType>, bool> = true
	>
	inline std::string to_hex(const DbnsType& v, bool nibbleMarker = false, bool hexPrefix = true) {
		char hexChar[16] = {
			'0', '1', '2', '3', '4', '5', '6', '7',
			'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
		};
		std::stringstream s;
		if (hexPrefix) s << "0x" << std::hex;
		int nrNibbles = int(1ull + ((DbnsType::nbits - 1ull) >> 2ull));
		for (int n = nrNibbles - 1; n >= 0; --n) {
			uint8_t nibble = v.nibble(unsigned(n));
			s << hexChar[nibble];
			if (nibbleMarker && n > 0 && (n % 4) == 0) s << '\'';
		}
		return s.str();
	}

	template<typename DbnsType,
		std::enable_if_t< is_dbns<DbnsType>, bool> = true
	>
	inline std::string pretty_print(const DbnsType& l, bool nibbleMarker = false) {
		std::stringstream s;

		bool sign = l.sign();

		// sign bit
		s << (sign ? "1:" : "0:");

		// integer bits
		for (int i = static_cast<int>(DbnsType::nbits) - 2; i >= static_cast<int>(DbnsType::sbbits); --i) {
			s << (l.at(static_cast<unsigned>(i)) ? '1' : '0');
			if (nibbleMarker && (i - DbnsType::sbbits) > 0 && ((i - DbnsType::sbbits) % 4) == 0) s << '\'';
		}

		// fraction bits
		if constexpr (DbnsType::sbbits > 0) {
			s << ':';
			for (int i = static_cast<int>(DbnsType::sbbits) - 1; i >= 0; --i) {
				s << (l.at(static_cast<unsigned>(i)) ? '1' : '0');
				if (nibbleMarker && i > 0 && (i % 4) == 0) s << '\'';
			}
		}

		return s.str();
	}

	// Report the raw encoding, the decoded fields and the value of a dbns.
	//
	// It returned the literal "TBD" and ignored both of its arguments (#1556). The shape is
	// posit's -- "raw: <bits> <fields> : value <v>" (posit/iostream.hpp) -- built from the
	// bit pattern rather than by streaming the dbns: this layer must not depend on
	// iostream.hpp (#1334), and operator<< is `ostr << double(r)`, so the value field is
	// the same text.
	//
	// The two base exponents are what a double-base encoding is, so they are reported
	// with their bases: the value is (-1)^s * base0^e0 * base1^e1. This deliberately does
	// not go through fraction(), which is hard-coded to return 0 for every dbns value
	// (dbns_impl.hpp) and which components() reports as if it meant something.
	template<typename DbnsType,
		std::enable_if_t< is_dbns<DbnsType>, bool> = true
	>
	inline std::string info_print(const DbnsType& l, int printPrecision = 17) {
		std::stringstream s;
		s << "raw: " << to_binary(l) << ' ' << (l.sign() ? "s1" : "s0");
		if (l.isnan()) {
			s << " nan : value nan";
			return s.str();
		}
		if (l.iszero()) {
			s << " zero : value 0";
			return s.str();
		}
		// scale() is reported again now that it is correct: #1556 left it out because it
		// computed e0 + e1*log2(3), dropping the sign of the base-0.5 exponent, and
		// answered 2 for a value of 1.5 whose scale is 0 (fixed in #1582).
		s << " e0(" << DbnsType::base0 << ")=" << l.extractExponent(0)
		  << " e1(" << DbnsType::base1 << ")=" << l.extractExponent(1)
		  << " scale " << l.scale()
		  << " : value " << std::setprecision(printPrecision) << double(l);
		return s.str();
	}

	template<typename DbnsType,
		std::enable_if_t< is_dbns<DbnsType>, bool> = true
	>
	inline std::string color_print(const DbnsType& l, bool nibbleMarker = false) {

		std::stringstream s;

		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color blue(ColorCode::FG_BLUE);
		Color magenta(ColorCode::FG_MAGENTA);
		Color cyan(ColorCode::FG_CYAN);
		Color white(ColorCode::FG_WHITE);
		Color def(ColorCode::FG_DEFAULT);
		s << red << (l.sign() ? "1" : "0");
	
		// first base exponent bits
		constexpr int lsbFirstBase = static_cast<int>(DbnsType::nbits - DbnsType::fbbits - 1);
		for (int i = static_cast<int>(DbnsType::nbits) - 2; i >= lsbFirstBase; --i) {
			s << cyan << (l.at(static_cast<unsigned>(i)) ? '1' : '0');
			if ((i - DbnsType::fbbits) > 0 && ((i - DbnsType::fbbits) % 4) == 0 && nibbleMarker) s << yellow << '\'';
		}

		// second base exponent bits
		if constexpr (lsbFirstBase > 0) {
			for (int i = lsbFirstBase - 1; i >= 0; --i) {
				s << magenta << (l.at(static_cast<unsigned>(i)) ? '1' : '0');
				if (i > 0 && (i % 4) == 0 && nibbleMarker) s << yellow << '\'';
			}
		}
		s << def;
		return s.str();
	}


// Moved out of dbns_impl.hpp (#1334): formats a dbns through a stringstream, which is
// what keeps it out of the core.
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
std::string to_binary(const dbns<nbits, fbbits, bt, xtra...>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	s << (number.sign() ? "1." : "0.");
	// first base exponent bits
	constexpr int lsbFirstBase = static_cast<int>(nbits - fbbits - 1);
	if constexpr (nbits - 2 >= fbbits) {
		for (int i = static_cast<int>(nbits) - 2; i >= lsbFirstBase; --i) {
			s << (number.at(static_cast<unsigned>(i)) ? '1' : '0');
			if ((i - fbbits) > 0 && ((i - fbbits) % 4) == 0 && nibbleMarker) s << '\'';
		}
	}
	if constexpr (lsbFirstBase > 0) {
		s << '.';
		for (int i = lsbFirstBase - 1; i >= 0; --i) {
			s << (number.at(static_cast<unsigned>(i)) ? '1' : '0');
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
		}
	}
	return s.str();
}

}} // namespace sw::universal
