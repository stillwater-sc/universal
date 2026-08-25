#pragma once
// manipulators.hpp: text produced from a blocksignificand
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The <iomanip> half of blocksignificand's text layer (#1334): to_binary and to_hex.
// blocksignificand.hpp holds the arithmetic and needs none of it, which is what keeps
// <sstream> out of every graph that builds on it.
#include <cstdint>
#include <sstream>
#include <string>
#include <universal/internal/blocksignificand/blocksignificand.hpp>

namespace sw { namespace universal {

//////////////////////////////////////////////////////////////////////////////
// conversions to string representations

// create a binary representation of the blocksignificand: 00h.ffff
// by design, the radix point is at nbits-3
template<unsigned nbits, typename bt>
std::string to_binary(const blocksignificand<nbits, bt>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	for (int i = nbits - 1; i >= 0; --i) {
		s << (number.at(unsigned(i)) ? '1' : '0');
		if (i == number.radix()) {
			s << '.';
		}
		else {
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
		}
	}
	return s.str();
}

// local helper to display the contents of a byte array in hex format
template<unsigned nbits, typename bt>
std::string to_hex(const blocksignificand<nbits, bt>& number, bool nibbleMarker = true) {
	char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::stringstream ss;
	ss << "0x" << std::hex;
	int nrNibbles = int(1 + ((nbits - 1) >> 2));
	for (int n = nrNibbles - 1; n >= 0; --n) {
		uint8_t nibble = number.nibble(static_cast<unsigned>(n));
		ss << hexChar[nibble];
		if (nibbleMarker && n > 0 && (n % 4) == 0) ss << '\'';
	}
	return ss.str();
}

///////////////////////////////////////////////////////////////////////////////
// specialty binary operators

// unrounded division, returns a blocksignificand that is of size 2*nbits
template<unsigned nbits, unsigned roundingBits, typename bt>
blocksignificand<2 * nbits + roundingBits, bt> urdiv(const blocksignificand<nbits, bt>& a, const blocksignificand<nbits, bt>& b, blocksignificand<roundingBits, bt>& r) {
	if (b.iszero()) {
		// division by zero
		throw "urdiv divide by zero";
	}
	// generate the absolute values to do long division 
	// 2's complement special case -max requires an signed int that is 1 bit bigger to represent abs()
	bool a_sign = a.sign();
	bool b_sign = b.sign();
	bool result_negative = (a_sign ^ b_sign);

	// normalize both arguments to positive in new size
	blocksignificand<nbits + 1, bt> a_new(a); // TODO optimize: now create a, create _a.bb, copy, destroy _a.bb_copy
	blocksignificand<nbits + 1, bt> b_new(b);
	if (a_sign) a_new.twoscomplement();
	if (b_sign) b_new.twoscomplement();

	// initialize the long division
	blocksignificand<2 * nbits + roundingBits, bt> decimator(a_new);
	blocksignificand<2 * nbits + roundingBits, bt> subtractand(b_new); // prepare the subtractand
	blocksignificand<2 * nbits + roundingBits, bt> result;

	int msp = nbits + roundingBits - 1; // msp = most significant position
	decimator <<= msp; // scale the decimator to the largest possible positive value

	int msb_b = subtractand.msb();
	int msb_a = decimator.msb();
	int shift = msb_a - msb_b;
	// The quotient is produced in an oversized fixed-point workspace. `scale` recenters it so the returned
	// bits remain the unrounded quotient, while the truncated tail is left in `r` for the caller's rounding rule.
	int scale = shift - msp;   // scale of the result quotient
	subtractand <<= shift;

	// long division
	for (int i = msb_a; i >= 0; --i) {

		if (subtractand <= decimator) {
			decimator -= subtractand;
			result.set(static_cast<unsigned>(i));
		}
		else {
			result.reset(static_cast<unsigned>(i));
		}
		subtractand >>= 1;

	}
	result <<= scale;
	if (result_negative) result.twosComplement();
	r.assign(result); // low bits preserve rounding information for later rounding.
	return result;
}

}} // namespace sw::universal
