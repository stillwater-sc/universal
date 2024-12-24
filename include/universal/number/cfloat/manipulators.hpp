#pragma once
// manipulators.hpp: definitions of helper functions for classic cfloat type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>  // for typeid()
#include <universal/number/cfloat/cfloat_fwd.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

// This file contains functions that manipulate a cfloat type
// using cfloat number system knowledge.

namespace sw { namespace universal {

// Generate a type tag for this cfloat, for example, cfloat<8,1, unsigned char, hasSubnormals, noSupernormals, notSaturating>
//template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
//std::string type_tag(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> & = {}) {}

// generate a type_tag for a cfloat
template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true>
inline std::string type_tag(CfloatType v = {}) {
	constexpr unsigned nbits = CfloatType::nbits;
	constexpr unsigned es = CfloatType::es;
	using bt = typename CfloatType::BlockType;
	constexpr bool hasSubnormals = CfloatType::hasSubnormals;
	constexpr bool hasSupernormals = CfloatType::hasSupernormals;
	constexpr bool isSaturating = CfloatType::isSaturating;
	std::stringstream s;
	if constexpr (nbits == 64 && es == 11 && hasSubnormals && !hasSupernormals && !isSaturating) {
		s << "fp64";
	}
	else if constexpr (nbits == 32 && es == 8 && hasSubnormals && !hasSupernormals && !isSaturating) {
		s << "fp32";
	}
	else if constexpr (nbits == 16 && es == 8 && hasSubnormals && !hasSupernormals && !isSaturating) {
		s << "bf16";
	}
	else if constexpr (nbits == 16 && es == 5 && hasSubnormals && !hasSupernormals && !isSaturating) {
		s << "fp16";
	}
	else if constexpr (nbits == 8 && es == 2 && hasSubnormals && !hasSupernormals && !isSaturating) {
		s << "fp8";
	}
	else {
		s << "cfloat<"
			<< std::setw(3) << nbits << ", "
			<< std::setw(3) << es << ", "
			<< type_tag(bt()) << ", "
			<< (hasSubnormals ? "hasSubnormals, " : " noSubnormals, ")
			<< (hasSupernormals ? "hasSupernormals, " : " noSupernormals, ")
			<< (isSaturating ? "   Saturating>" : "notSaturating>");
	}
	return s.str();
}

// Generate a type field descriptor for this cfloat
template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline std::string type_field(const CfloatType & = {}) {
	std::stringstream s;
//	typename CfloatType::BlockType bt{0};
//	unsigned nbits = CfloatType::nbits;  // total bits
	unsigned ebits = CfloatType::es;     // exponent bits
	unsigned fbits = CfloatType::fbits;  // integer bits
	s << "fields(s:1|e:" << ebits << "|m:" << fbits << ')';
	return s.str();
}

// generate and tabulate subnormals of a cfloat configuration
template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline void subnormals() {
	constexpr unsigned nbits       = CfloatType::nbits;
	constexpr unsigned es          = CfloatType::es;
	using bt                       = typename CfloatType::BlockType;
	constexpr bool hasSubnormals   = CfloatType::hasSubnormals;
	constexpr bool hasSupernormals = CfloatType::hasSupernormals;
	constexpr bool isSaturating    = CfloatType::isSaturating;
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a{ 0 };

	// generate the smallest subnormal with ULP set
	++a;
	if constexpr (hasSubnormals) {
		constexpr unsigned fbits = CfloatType::fbits;
		std::cout << type_tag(a) << " subnormals\n";
		if constexpr (nbits < 65u) {
			for (size_t i = 0; i < fbits; ++i) {
				std::cout << to_binary(a, true) << " : " << color_print(a) << " : " << a << '\n';
				uint64_t fraction = a.fraction_ull();
				fraction <<= 1;
				a.setfraction(fraction);
			}
		}
		else {
#ifdef DEPRECATED
			blockbinary<fbits, bt> fraction{ 0 };
			for (size_t i = 0; i < fbits; ++i) {
				std::cout << to_binary(a, true) << " : " << color_print(a) << " : " << a << '\n';
				a.fraction(fraction);
				fraction <<= 1;
				a.setfraction(fraction);
			}
#endif
			std::cerr << "big cfloat subnormals TBD\n";
		}
	}
	else {
		std::cout << type_tag(a) << " has no subnormals\n";
	}
}

// Generate a string representing the cfloat components: sign, exponent, faction and value
template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline std::string components(const CfloatType& v) {
	//constexpr unsigned nbits = CfloatType::nbits;
	constexpr unsigned es    = CfloatType::es;
	using bt = typename CfloatType::BlockType;
	constexpr unsigned fbits = CfloatType::fbits;
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
template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline std::string to_hex(const CfloatType& v, bool nibbleMarker = false, bool hexPrefix = true) {
	constexpr unsigned nbits = CfloatType::nbits;
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

// generate a cfloat format ASCII hex format nbits.esxNN...NNa
template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline std::string hex_print(const CfloatType& c) {
	constexpr unsigned nbits = CfloatType::nbits;
	constexpr unsigned es    = CfloatType::es;
	std::stringstream s;
	s << nbits << '.' << es << 'x' << to_hex(c) << 'c';
	return s.str();
}

template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline std::string pretty_print(const CfloatType& r) {
	constexpr unsigned es     = CfloatType::es;
	constexpr unsigned fhbits = CfloatType::fhbits;
	using bt = typename CfloatType::BlockType;
	bool sign{ false };
	blockbinary<es, bt> e;
	blockbinary<fhbits, bt> f;
	decode(r, sign, e, f);

	std::stringstream s;
	// sign bit
	s << (sign ? '1' : '0');

	// exponent bits
	s << ':';
	for (int i = int(es) - 1; i >= 0; --i) {
		s << (e.test(static_cast<size_t>(i)) ? '1' : '0');
	}

	// fraction bits
	s << ':';
	for (int i = int(r.fbits) - 1; i >= 0; --i) {
		s << (f.test(static_cast<size_t>(i)) ? '1' : '0');
	}

	return s.str();
}

template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline std::string info_print(const CfloatType& p, int printPrecision = 17) {
	return std::string("TBD");
}

// generate a binary, color-coded representation of the cfloat
template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline std::string color_print(const CfloatType& r, bool nibbleMarker = false) {
	constexpr unsigned es     = CfloatType::es;
	constexpr unsigned fhbits = CfloatType::fhbits;
	using bt = typename CfloatType::BlockType;
	bool sign{ false };
	blockbinary<es,bt> e;
	blockbinary<fhbits,bt> f;
	decode(r, sign, e, f);

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color blue(ColorCode::FG_BLUE);
	Color magenta(ColorCode::FG_MAGENTA);
	Color cyan(ColorCode::FG_CYAN);
	Color white(ColorCode::FG_WHITE);
	Color def(ColorCode::FG_DEFAULT);

	std::stringstream s;
	// sign bit
	s << red << (sign ? '1' : '0');

	// exponent bits
	for (int i = int(es) - 1; i >= 0; --i) {
		s << cyan << (e.test(static_cast<size_t>(i)) ? '1' : '0');
		if ((i - es) > 0 && ((i - es) % 4) == 0 && nibbleMarker) s << yellow << '\'';
	}

	// fraction bits
	for (int i = int(r.fbits) - 1; i >= 0; --i) {
		s << magenta << (f.test(static_cast<size_t>(i)) ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << yellow << '\'';
	}

	s << def;
	return s.str();
}


}} // namespace sw::universal
