#pragma once
// manipulators.hpp: definitions of helper functions for hfloat type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Generate a type tag for this hfloat
	template<unsigned ndigits, unsigned es, typename bt>
	std::string type_tag(const hfloat<ndigits, es, bt>& = {}) {
		std::stringstream s;
		if constexpr (ndigits == 6 && es == 7) {
			s << "hfloat_short (IBM HFP 32-bit)";
		}
		else if constexpr (ndigits == 14 && es == 7) {
			s << "hfloat_long (IBM HFP 64-bit)";
		}
		else if constexpr (ndigits == 28 && es == 7) {
			s << "hfloat_extended (IBM HFP 128-bit)";
		}
		else {
			s << "hfloat<"
				<< std::setw(3) << ndigits << ", "
				<< std::setw(3) << es << ", "
				<< typeid(bt).name() << '>';
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

		// sign in red
		s << "\033[31m" << (number.sign() ? '1' : '0') << "\033[0m" << '.';

		// exponent in blue
		s << "\033[34m";
		unsigned expStart = Hfloat::nbits - 2;
		for (unsigned i = 0; i < es; ++i) {
			s << (number.getbit(expStart - i) ? '1' : '0');
		}
		s << "\033[0m" << '.';

		// fraction in default color (with hex-digit separators)
		for (int i = static_cast<int>(Hfloat::fbits) - 1; i >= 0; --i) {
			s << (number.getbit(static_cast<unsigned>(i)) ? '1' : '0');
			if (nibbleMarker && i > 0 && (i % 4 == 0)) s << '\'';
		}

		return s.str();
	}

	// components: show unpacked sign, exponent, fraction in hex
	template<unsigned ndigits, unsigned es, typename bt>
	std::string components(const hfloat<ndigits, es, bt>& number) {
		std::stringstream s;
		bool sign; int exp; uint64_t frac;
		number.unpack(sign, exp, frac);
		s << (sign ? "(-" : "(+") << "0x";
		// show hex digits
		for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
			unsigned hex_digit = (frac >> (i * 4)) & 0xF;
			s << "0123456789ABCDEF"[hex_digit];
		}
		s << " * 16^" << exp << ')';
		return s.str();
	}

}} // namespace sw::universal
