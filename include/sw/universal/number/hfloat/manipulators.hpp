#pragma once
// manipulators.hpp: definitions of helper functions for hfloat type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the hfloat headers (#1334): the <iomanip> half -- everything that turns an
// hfloat into a std::string through a stringstream. iostream.hpp is the <iostream> half.
// Self-contained: it names every header it uses rather than relying on the umbrella's
// include order.
#include <string>        // std::string
#include <sstream>       // std::stringstream
#include <iomanip>       // std::setw
// pull in type_tag overloads for native integer block types
#include <universal/native/integer_type_tag.hpp>
#include <universal/number/hfloat/core.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {


	// Generate a type tag for this hfloat
	template<unsigned ndigits, unsigned es, typename bt>
	std::string type_tag(const hfloat<ndigits, es, bt>& = {}) {
		std::stringstream s;
		if constexpr (ndigits == 6 && es == 7) {
			s << "hfp32 (IBM HFP 32-bit)";
		}
		else if constexpr (ndigits == 14 && es == 7) {
			s << "hfp64 (IBM HFP 64-bit)";
		}
		else if constexpr (ndigits == 28 && es == 7) {
			s << "hfp128 (IBM HFP 128-bit)";
		}
		else {
			s << "hfloat<"
				<< std::setw(3) << ndigits << ", "
				<< std::setw(3) << es << ", "
				<< type_tag(bt{}) << '>';
		}
		return s.str();
	}

	// type field descriptor
	template<unsigned ndigits, unsigned es, typename bt>
	std::string type_field(const hfloat<ndigits, es, bt>& = {}) {
		using Hfloat = hfloat<ndigits, es, bt>;
		std::stringstream s;
		s << "fields(s:1|exp:" << es << "|frac:" << Hfloat::fbits << ')';
		return s.str();
	}

	// color print: show sign, exponent, fraction with ANSI colors
	template<unsigned ndigits, unsigned es, typename bt>
	std::string color_print(const hfloat<ndigits, es, bt>& number, bool nibbleMarker = false) {
		using Hfloat = hfloat<ndigits, es, bt>;
		std::stringstream s;

		Color red(ColorCode::FG_RED);
	    Color yellow(ColorCode::FG_YELLOW);
	    Color magenta(ColorCode::FG_MAGENTA);
	    Color cyan(ColorCode::FG_CYAN);
	    Color def(ColorCode::FG_DEFAULT);

		// sign bit
		s << red << (number.sign() ? '1' : '0') << def;

		// exponent bits
		unsigned expStart = Hfloat::nbits - 2;
		for (unsigned i = 0; i < es; ++i) {
			s << cyan << (number.getbit(expStart - i) ? '1' : '0');
		}

		// fraction bits
		for (int i = static_cast<int>(Hfloat::fbits) - 1; i >= 0; --i) {
			s << magenta << (number.getbit(static_cast<unsigned>(i)) ? '1' : '0');
			if (nibbleMarker && i > 0 && (i % 4 == 0)) s << yellow << '\'';
		}
	    s << def;

		return s.str();
	}

	// components: show unpacked sign, exponent, fraction in hex
	template<unsigned ndigits, unsigned es, typename bt>
	std::string components(const hfloat<ndigits, es, bt>& number) {
		std::stringstream s;
		bool sign = number.sign();
		s << "sign: " << (sign ? '-' : '+');
		if (number.iszero()) {
			s << ", zero";
		}
		else {
			// Read sign + exponent the same way unpack() does, but iterate
			// fraction hex digits directly from storage so wide configs
			// (hfp128 at ndigits=28) print all 28 digits instead of the
			// uint64_t-limited first 16. Use a left-fold to assemble
			// exp_field MSB-first; this is safe even if es approaches the
			// width of unsigned (the legacy `1u << (es - 1 - i)` is UB once
			// es exceeds the width of unsigned).
			using Hfloat = hfloat<ndigits, es, bt>;
			unsigned exp_field = 0;
			unsigned expStart = Hfloat::nbits - 2;
			for (unsigned i = 0; i < es; ++i) {
				exp_field = (exp_field << 1) | (number.getbit(expStart - i) ? 1u : 0u);
			}
			int exp = static_cast<int>(exp_field) - Hfloat::bias;
			s << ", hex scale: " << exp << ", hex fraction: 0x0.";
			for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
				unsigned hex_digit = 0;
				for (unsigned b = 0; b < 4u; ++b) {
					if (number.getbit(static_cast<unsigned>(i) * 4u + b)) {
						hex_digit |= (1u << b);
					}
				}
				s << "0123456789ABCDEF"[hex_digit];
			}
		}
		return s.str();
	}


// Moved out of hfloat_impl.hpp (#1334): these format an hfloat through a stringstream,
// which is what keeps them out of the core.

template<unsigned ndigits, unsigned es, typename BlockType>
inline std::string to_binary(const hfloat<ndigits, es, BlockType>& number, bool nibbleMarker = false) {
	using Hfloat = hfloat<ndigits, es, BlockType>;
	std::stringstream s;

	// sign bit
	s << "0b" << (number.sign() ? '1' : '0') << '.';

	// exponent field (es bits)
	unsigned expStart = Hfloat::nbits - 2;
	for (unsigned i = 0; i < es; ++i) {
		s << (number.getbit(expStart - i) ? '1' : '0');
	}
	s << '.';

	// fraction field (fbits bits, show in hex-digit groups)
	for (int i = static_cast<int>(Hfloat::fbits) - 1; i >= 0; --i) {
		s << (number.getbit(static_cast<unsigned>(i)) ? '1' : '0');
		if (nibbleMarker && i > 0 && (i % 4 == 0)) s << '\'';
	}

	return s.str();
}

template<unsigned ndigits, unsigned es, typename BlockType>
inline std::string to_hex(const hfloat<ndigits, es, BlockType>& number) {
	std::stringstream s;

	s << (number.sign() ? '-' : '+');
	s << "0x0.";

	// Read exponent and fraction directly from bit storage. unpack() returns
	// the fraction as uint64_t, which loses bits for wide configs
	// (hfloat_extended has fbits=112); the legacy `frac_val >> (i*4)` loop
	// below also had UB for i*4 >= 64, producing wrapped/duplicated digits.
	// MSB-first left-fold to assemble the exponent field; safe for any es.
	using Hfloat = hfloat<ndigits, es, BlockType>;
	unsigned exp_field = 0;
	unsigned expStart  = Hfloat::nbits - 2;
	for (unsigned i = 0; i < es; ++i) {
		exp_field = (exp_field << 1) | (number.getbit(expStart - i) ? 1u : 0u);
	}
	int exp_val = static_cast<int>(exp_field) - Hfloat::bias;

	for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
		unsigned hex_digit = 0;
		for (unsigned b = 0; b < 4u; ++b) {
			if (number.getbit(static_cast<unsigned>(i) * 4u + b)) {
				hex_digit |= (1u << b);
			}
		}
		s << "0123456789ABCDEF"[hex_digit];
	}
	s << " * 16^" << exp_val;

	return s.str();
}


// native semantic representation: radix-16, delegates to to_hex
template<unsigned ndigits, unsigned es, typename BlockType>
inline std::string to_native(const hfloat<ndigits, es, BlockType>& v, bool = false) {
	return to_hex(v);
}

}} // namespace sw::universal
