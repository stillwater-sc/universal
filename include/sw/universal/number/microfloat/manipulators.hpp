#pragma once
// manipulators.hpp: definition of manipulation functions for microfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <iomanip>
#include <universal/number/microfloat/microfloat_fwd.hpp>
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for microfloat types
	template<unsigned nbits, unsigned es, bool hasInf, bool hasNaN, bool isSaturating>
	inline std::string type_tag(const microfloat<nbits, es, hasInf, hasNaN, isSaturating>& = {}) {
		// Return friendly names for standard MX aliases
		if constexpr (nbits == 4 && es == 2 && !hasInf && !hasNaN && isSaturating) return "e2m1";
		if constexpr (nbits == 6 && es == 2 && !hasInf && !hasNaN && isSaturating) return "e2m3";
		if constexpr (nbits == 6 && es == 3 && !hasInf && !hasNaN && isSaturating) return "e3m2";
		if constexpr (nbits == 8 && es == 4 && !hasInf && hasNaN && isSaturating) return "e4m3";
		if constexpr (nbits == 8 && es == 5 && hasInf && hasNaN && !isSaturating) return "e5m2";
		// Generic fallback
		std::stringstream s;
		s << "microfloat<" << nbits << "," << es;
		if (hasInf) s << ",inf";
		if (hasNaN) s << ",nan";
		if (isSaturating) s << ",sat";
		s << ">";
		return s.str();
	}

	// generate a hex string for microfloat
	template<unsigned nbits, unsigned es, bool hasInf, bool hasNaN, bool isSaturating>
	inline std::string to_hex(const microfloat<nbits, es, hasInf, hasNaN, isSaturating>& v, bool = false, bool hexPrefix = true) {
		char hexChar[16] = {
			'0', '1', '2', '3', '4', '5', '6', '7',
			'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
		};
		std::stringstream s;
		if (hexPrefix) s << "0x";
		int nrNibbles = static_cast<int>(1 + ((nbits - 1) >> 2));
		for (int nn = nrNibbles - 1; nn >= 0; --nn) {
			uint8_t nibble = v.nibble(static_cast<unsigned>(nn));
			s << hexChar[nibble];
		}
		return s.str();
	}

	// return in triple form (sign, scale, fraction)
	template<unsigned nbits, unsigned es, bool hasInf, bool hasNaN, bool isSaturating>
	inline std::string to_triple(const microfloat<nbits, es, hasInf, hasNaN, isSaturating>& number, bool = false) {
		constexpr unsigned fbits = nbits - 1u - es;
		constexpr int bias = (1 << (es - 1)) - 1;
		std::stringstream s;

		s << '(' << (number.sign() ? '-' : '+') << ',';

		unsigned exponent = number.exponent();
		int scale = static_cast<int>(exponent) - bias;
		s << scale << ",0b";

		unsigned fraction = number.fraction();
		unsigned mask = (1u << (fbits - 1));
		for (int j = static_cast<int>(fbits) - 1; j >= 0; --j) {
			s << ((fraction & mask) ? '1' : '0');
			mask >>= 1;
		}

		s << ')';
		return s.str();
	}

	// generate a binary, color-coded representation of the microfloat
	template<unsigned nbits, unsigned es, bool hasInf, bool hasNaN, bool isSaturating>
	inline std::string color_print(const microfloat<nbits, es, hasInf, hasNaN, isSaturating>& r, bool = false) {
		constexpr unsigned fbits = nbits - 1u - es;
		std::stringstream s;

		Color red(ColorCode::FG_RED);
		Color cyan(ColorCode::FG_CYAN);
		Color magenta(ColorCode::FG_MAGENTA);
		Color def(ColorCode::FG_DEFAULT);

		// sign bit
		s << red << (r.sign() ? '1' : '0');

		// exponent bits
		uint8_t exp = r.exponent();
		for (int j = static_cast<int>(es) - 1; j >= 0; --j) {
			s << cyan << ((exp & (1u << j)) ? '1' : '0');
		}

		// fraction bits
		uint8_t frac = r.fraction();
		for (int j = static_cast<int>(fbits) - 1; j >= 0; --j) {
			s << magenta << ((frac & (1u << j)) ? '1' : '0');
		}

		s << def;
		return s.str();
	}

}} // namespace sw::universal
