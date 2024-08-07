// manipulators.hpp: definitions of helper functions for doubledouble type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <iomanip>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/number/dd/dd_fwd.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for a doubledouble
	std::string type_tag(const dd& = {}) {
		return std::string("doubledouble");
	}

	// generate a binary, color-coded representation of the doubledouble
	std::string color_print(const dd& r, bool nibbleMarker = false) {
		constexpr unsigned es = 11;
		constexpr unsigned fbits = 106;
		std::stringstream s;
		bool sign{ false };
		blockbinary<es> e{ 0 };
		blockbinary<fbits + 1, uint32_t, BinaryNumberType::Unsigned> f{ 0 };
		sign = r.sign();
		e = r.exponent();
		uint128_t raw = r.fraction();
		f.setbits(raw.limb[0]);

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
			if ((i - es) > 0 && ((i - es) % 4) == 0 && nibbleMarker) s << yellow << '\'';
		}

		// fraction bits
		for (int i = int(fbits) - 1; i >= 0; --i) {
			s << magenta << (f.test(static_cast<size_t>(i)) ? '1' : '0');
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << yellow << '\'';
		}

		s << def;
		return s.str();
	}

}} // namespace sw::universal
