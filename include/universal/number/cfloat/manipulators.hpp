#pragma once
// manipulators.hpp: definitions of helper functions for classic cfloat type manipulation
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

// This file contains functions that manipulate a cfloat type
// using cfloat number system knowledge.

namespace sw::universal {

// Generate a type tag for this cfloat, for example, cfloat<8,1, unsigned char, hasSubnormals, noSupernormals, notSaturating>
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
std::string type_tag(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& v) {
	std::stringstream s;
	s << "cfloat<"
		<< nbits << ", "
		<< es << ", "
		<< typeid(bt).name() << ", "
		<< (hasSubnormals ? "hasSubnormals, " : "noSubnormals, ")
		<< (hasSupernormals ? "hasSupernormals, " : "noSupernormals, ")
		<< (isSaturating ? "Saturating>" : "notSaturating>");
	if (v.iszero()) s << ' ';
	return s.str();
}

// generate and tabulate subnormals of the cfloat configuration
template<typename cfloatConfiguration>
void subnormals() {
	constexpr size_t nbits         = cfloatConfiguration::nbits;
	constexpr size_t es            = cfloatConfiguration::es;
	using bt                       = typename cfloatConfiguration::BlockType;
	constexpr bool hasSubnormals   = cfloatConfiguration::hasSubnormals;
	constexpr bool hasSupernormals = cfloatConfiguration::hasSupernormals;
	constexpr bool isSaturating    = cfloatConfiguration::isSaturating;
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a{ 0 };

	// generate the smallest subnormal with ULP set
	++a;
	if constexpr (hasSubnormals) {
		constexpr size_t fbits = cfloatConfiguration::fbits;
		std::cout << type_tag(a) << " subnormals\n";
		if constexpr (nbits < 65) {
			for (size_t i = 0; i < fbits; ++i) {
				std::cout << to_binary(a, true) << " : " << color_print(a) << " : " << a << '\n';
				uint64_t fraction = a.fraction_ull();
				fraction <<= 1;
				a.setfraction(fraction);
			}
		}
		else {
			blockbinary<fbits, bt> fraction{ 0 };
			for (size_t i = 0; i < fbits; ++i) {
				std::cout << to_binary(a, true) << " : " << color_print(a) << " : " << a << '\n';
				a.fraction(fraction);
				fraction <<= 1;
				a.setfraction(fraction);
			}
		}
	}
	else {
		std::cout << type_tag(a) << " has no subnormals\n";
	}
}

// report dynamic range of a type, specialized for a cfloat
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
std::string dynamic_range(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& b) {
	std::stringstream s;
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> c(b);
	s << type_tag(c) << ": ";
	s << "minpos scale " << std::setw(10) << c.minpos().scale() << "     ";
	s << "maxpos scale " << std::setw(10) << c.maxpos().scale() << '\n';
	s << "[" << c.maxneg() << " ... " << c.minneg() << ", -0, +0, " << c.minpos() << " ... " << c.maxpos() << "]\n";
	cfloat<nbits + 1, es, bt, hasSubnormals, hasSupernormals, isSaturating> d,e;
	d = double(c.maxneg());
	d--;
	e = double(c.maxpos());
	e++;
	s << "inclusive range = (" << to_binary(d) << ", " << to_binary(d) << ")\n";
	s << "inclusive range = (" << d << ", " << e << ")\n";
	return s.str();
}

// Generate a string representing the cfloat components: sign, exponent, faction and value
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
std::string components(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& v) {
	std::stringstream s;
	bool sign{ false };
	blockbinary<v.es, bt> e;
	blockbinary<v.fbits, bt> f;
	decode(v, sign, e, f);

	// TODO: hardcoded field width is governed by pretty printing cfloat tables, which by construction will always be small cfloats
	s << std::setw(14) << to_binary(v) 
		<< " Sign : " << std::setw(2) << sign
		<< " Exponent : " << std::setw(5) << e
		<< " Fraction : " << std::setw(8) << f
		<< " Value : " << std::setw(16) << v;

	return s.str();
}

// generate a binary string for cfloat
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline std::string to_hex(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& v) {
	constexpr size_t bitsInByte = 8;
	constexpr size_t bitsInBlock = sizeof(bt) * bitsInByte;
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::stringstream s;
	s << "0x" << std::hex;
	long nrNibbles = long(1ull + ((nbits - 1ull) >> 2ull));
	for (long n = nrNibbles - 1; n >= 0; --n) {
		uint8_t nibble = v.nibble(size_t(n));
		s << hexChar[nibble];
		if (n > 0 && ((n * 4ll) % bitsInBlock) == 0) s << '\'';
	}
	return s.str();
}

// generate a cfloat format ASCII hex format nbits.esxNN...NNa
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
inline std::string hex_print(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& c) {
	std::stringstream s;
	s << nbits << '.' << es << 'x' << to_hex(c) << 'c';
	return s.str();
}

template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
std::string pretty_print(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& r) {
	std::stringstream s;
	constexpr size_t fbits = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>::fbits;
	bool sign{ false };
	blockbinary<es, bt> e;
	blockbinary<fbits, bt> f;
	decode(r, sign, e, f);

	// sign bit
	s << (sign ? '1' : '0');

	// exponent bits
	s << '-';
	for (int i = int(es) - 1; i >= 0; --i) {
		s << (e.test(static_cast<size_t>(i)) ? '1' : '0');
	}

	// fraction bits
	s << '-';
	for (int i = int(r.fbits) - 1; i >= 0; --i) {
		s << (f.test(static_cast<size_t>(i)) ? '1' : '0');
	}

	return s.str();
}

template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
std::string info_print(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& p, int printPrecision = 17) {
	return "TBD";
}

// generate a binary, color-coded representation of the cfloat
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
std::string color_print(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& r) {
	using Real = cfloat<nbits, es, bt>;
	std::stringstream s;
	bool sign{ false };
	blockbinary<es,bt> e;
	blockbinary<Real::fbits,bt> f;
	decode(r, sign, e, f);

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
	}

	// fraction bits
	for (int i = int(r.fbits) - 1; i >= 0; --i) {
		s << magenta << (f.test(static_cast<size_t>(i)) ? '1' : '0');
	}

	s << def;
	return s.str();
}


}  // namespace sw::universal