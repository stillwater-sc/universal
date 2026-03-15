#pragma once
// manipulators.hpp: definitions of helper functions for unum Type I manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <iomanip>
#include <sstream>

#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

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
