#pragma once
// manipulators.hpp: definitions of helper functions for unum Type I manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the unum headers (#1334, Phase 2 group 5c, #1467): the <iomanip> half --
// everything that turns a unum into a std::string, and parse(), which reads a decimal
// literal through an istringstream (the cfloat/bfloat16 placement). Self-contained.
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

#include <universal/utility/color_print.hpp>
#include <universal/number/unum/core.hpp>

namespace sw { namespace universal {

// parse a unum from a decimal floating-point string
template<unsigned esizesize, unsigned fsizesize, typename bt>
bool parse(const std::string& txt, unum<esizesize, fsizesize, bt>& v) {
	// Detect nan / inf / infinity tokens (case-insensitive, optional sign).
	// unum Type I has no Inf encoding -- inf tokens collapse to NaN, matching
	// the existing operator=(double) behavior which also maps +/-inf to NaN.
	{
		std::string t;
		t.reserve(txt.size());
		for (char c : txt) {
			t.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
		}
		std::string body = t;
		if (!body.empty() && (body.front() == '+' || body.front() == '-')) body.erase(0, 1);
		if (body == "nan" || body == "inf" || body == "infinity") {
			v.setnan();
			return true;
		}
	}
	std::istringstream ss(txt);
	double d;
	ss >> d;
	if (ss.fail()) return false;
	ss >> std::ws;
	if (!ss.eof()) return false;
	v = d;
	return true;
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
inline std::string components(const unum<esizesize, fsizesize, bt>& v) {
	std::stringstream s;
	if (v.iszero()) {
		s << "zero";
	}
	else if (v.isnan()) {
		s << "NaN";
	}
	else {
		s << (v.sign() ? "-" : "+")
		  << " esize:" << (v.esize() + 1u) << " fsize:" << v.fsize()
		  << " exp:" << v.exponent() << " frac:" << v.fraction()
		  << " ubit:" << v.ubit();
	}
	return s.str();
}


// Generate a type tag for this unum
template<unsigned esizesize, unsigned fsizesize, typename bt>
std::string type_tag(const unum<esizesize, fsizesize, bt>& = {}) {
	std::stringstream ss;
	ss << "sw::universal::unum<" << esizesize << ", " << fsizesize << ", "
	   << type_tag(bt{}) << '>';
	return ss.str();
}

// report the dynamic range of a unum configuration
template<unsigned esizesize, unsigned fsizesize, typename bt = std::uint8_t>
std::string unum_range() {
	using Unum = unum<esizesize, fsizesize, bt>;
	std::stringstream ss;
	ss << " unum<" << esizesize << ", " << fsizesize << "> ";
	ss << "max exponent bits " << Unum::maxesize << "     ";
	ss << "max fraction bits " << Unum::maxfsize << "     ";
	ss << "max word size " << Unum::maxbits << " bits";
	return ss.str();
}

// binary representation showing field decomposition:
// sign.exponent.fraction.esize_field.fsize_field.ubit
template<unsigned esizesize, unsigned fsizesize, typename bt>
inline std::string to_binary(const unum<esizesize, fsizesize, bt>& u, bool nibbleMarker = false) {
	std::stringstream s;
	unsigned es = u.esize();
	unsigned fs = u.fsize();

	// sign bit
	s << "0b" << (u.sign() ? '1' : '0') << '.';

	// exponent field (esize+1 bits, MSB first)
	unsigned exp_bits = es + 1u;
	uint64_t exp = u.exponent();
	for (unsigned i = 0; i < exp_bits; ++i) {
		unsigned bit_pos = exp_bits - 1u - i;
		s << ((exp >> bit_pos) & 1u ? '1' : '0');
		if (nibbleMarker && ((bit_pos % 4) == 0) && bit_pos != 0) s << '\'';
	}
	s << '.';

	// fraction field (fsize bits, MSB first)
	uint64_t frac = u.fraction();
	if (fs == 0) {
		s << '~';
	}
	else {
		for (unsigned i = 0; i < fs; ++i) {
			unsigned bit_pos = fs - 1u - i;
			s << ((frac >> bit_pos) & 1u ? '1' : '0');
			if (nibbleMarker && ((bit_pos % 4) == 0) && bit_pos != 0) s << '\'';
		}
	}
	s << '.';

	// esize field (esizesize bits, MSB first)
	unsigned es_val = es;
	for (unsigned i = 0; i < esizesize; ++i) {
		unsigned bit_pos = esizesize - 1u - i;
		s << ((es_val >> bit_pos) & 1u ? '1' : '0');
	}
	s << '.';

	// fsize field (fsizesize bits, MSB first)
	unsigned fs_val = fs;
	for (unsigned i = 0; i < fsizesize; ++i) {
		unsigned bit_pos = fsizesize - 1u - i;
		s << ((fs_val >> bit_pos) & 1u ? '1' : '0');
	}
	s << '.';

	// ubit
	s << (u.ubit() ? '1' : '0');

	return s.str();
}

// color_print: fields in distinct colors
// sign(red), exponent(cyan), fraction(magenta), esize(yellow), fsize(yellow), ubit(green)
template<unsigned esizesize, unsigned fsizesize, typename bt>
std::string color_print(const unum<esizesize, fsizesize, bt>& u) {
	std::stringstream s;
	unsigned es = u.esize();
	unsigned fs = u.fsize();

	Color red(ColorCode::FG_RED);
	Color cyan(ColorCode::FG_CYAN);
	Color magenta(ColorCode::FG_MAGENTA);
	Color yellow(ColorCode::FG_YELLOW);
	Color green(ColorCode::FG_GREEN);
	Color def(ColorCode::FG_DEFAULT);

	// sign
	s << red << (u.sign() ? '1' : '0');

	// exponent (esize+1 bits, MSB first)
	uint64_t exp = u.exponent();
	for (unsigned i = 0; i < es + 1u; ++i) {
		unsigned bit_pos = es - i;
		s << cyan << ((exp >> bit_pos) & 1u ? '1' : '0');
	}

	// fraction (fsize bits, MSB first)
	uint64_t frac = u.fraction();
	for (unsigned i = 0; i < fs; ++i) {
		unsigned bit_pos = fs - 1u - i;
		s << magenta << ((frac >> bit_pos) & 1u ? '1' : '0');
	}

	// esize field (esizesize bits, MSB first)
	for (unsigned i = 0; i < esizesize; ++i) {
		unsigned bit_pos = esizesize - 1u - i;
		s << yellow << ((es >> bit_pos) & 1u ? '1' : '0');
	}

	// fsize field (fsizesize bits, MSB first)
	for (unsigned i = 0; i < fsizesize; ++i) {
		unsigned bit_pos = fsizesize - 1u - i;
		s << yellow << ((fs >> bit_pos) & 1u ? '1' : '0');
	}

	// ubit
	s << green << (u.ubit() ? '1' : '0');

	s << def;
	return s.str();
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
std::string pretty_print(const unum<esizesize, fsizesize, bt>& u) {
	std::stringstream s;
	s << "s" << (u.sign() ? '1' : '0')
	  << " e(" << (u.esize() + 1u) << "):" << u.exponent()
	  << " f(" << u.fsize() << "):" << u.fraction()
	  << " u" << u.ubit();
	return s.str();
}

template<unsigned esizesize, unsigned fsizesize, typename bt>
std::string info_print(const unum<esizesize, fsizesize, bt>& u) {
	std::stringstream s;
	s << "unum<" << esizesize << "," << fsizesize << "> "
	  << "bits_used:" << u.nbits_used()
	  << " esize:" << (u.esize() + 1u)
	  << " fsize:" << u.fsize()
	  << " value:" << u.to_double()
	  << (u.ubit() ? " (inexact)" : " (exact)");
	return s.str();
}

}} // namespace sw::universal
