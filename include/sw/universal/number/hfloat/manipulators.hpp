#pragma once
// manipulators.hpp: definitions of helper functions for hfloat type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// pull in type_tag overloads for native integer block types
#include <universal/native/integer_type_tag.hpp>

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

}} // namespace sw::universal
