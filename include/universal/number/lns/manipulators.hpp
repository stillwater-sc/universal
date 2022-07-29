#pragma once
// manipulators.hpp: definitions of helper functions for logarithmic numbers manipulation
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for this lns
	template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename BlockType>
	inline std::string type_tag(const lns<nbits, rbits, behavior, BlockType>& = {}) {
		std::stringstream s;
		s << "lns<"
			<< std::setw(3) << nbits << ", "
			<< std::setw(3) << rbits << ", "
			<< typeid(BlockType).name() << '>';
		return s.str();
	}

	// report dynamic range of a type, specialized for lns
	template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
	inline std::string dynamic_range(const lns<nbits, rbits, behavior, bt>& a) {
		std::stringstream s;
		lns<nbits, rbits, behavior, bt> b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
		s << type_tag(a) << ": ";
		s << "minpos scale " << std::setw(10) << d.scale() << "     ";
		s << "maxpos scale " << std::setw(10) << e.scale() << '\n';
		s << "[" << b << " ... " << c << ", 0, " << d << " ... " << e << "]\n";
		s << "[" << to_binary(b) << " ... " << to_binary(c) << ", 0, " << to_binary(d) << " ... " << to_binary(e) << "]\n";
		return s.str();
	}

	template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
	inline std::string range() {
		std::stringstream s;
		lns<nbits, rbits, behavior, bt> b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
		s << "[" << b << " ... " << c << ", 0, " << d << " ... " << e << "]\n";
		return s.str();
	}

	// report if a native floating-point value is within the dynamic range of the lns configuration
	template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
	inline bool isInRange(double v) {
		using LNS = lns<nbits, rbits, behavior, bt>;
		LNS a;

		bool inRange = true;
		if (v > double(a.maxpos()) || v < double(a.maxneg())) inRange = false;
		return inRange;
	}

	template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename BlockType>
	inline std::string color_print(const lns<nbits, rbits, behavior, BlockType>& l, bool nibbleMarker = false) {

		std::stringstream s;

		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color blue(ColorCode::FG_BLUE);
		Color magenta(ColorCode::FG_MAGENTA);
		Color cyan(ColorCode::FG_CYAN);
		Color white(ColorCode::FG_WHITE);
		Color def(ColorCode::FG_DEFAULT);
		s << red << (l.sign() ? "1" : "0");
	
		// integer bits
		for (int i = static_cast<int>(nbits) - 2; i >= static_cast<int>(rbits); --i) {
			s << cyan << (l.at(static_cast<size_t>(i)) ? '1' : '0');
			if ((i - rbits) > 0 && ((i - rbits) % 4) == 0 && nibbleMarker) s << yellow << '\'';
		}

		// fraction bits
		if constexpr (rbits > 0) {
			s << magenta << '.';
			for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
				s << magenta << (l.at(static_cast<size_t>(i)) ? '1' : '0');
				if (i > 0 && (i % 4) == 0 && nibbleMarker) s << yellow << '\'';
			}
		}
		s << def;
		return s.str();
	}

}} // namespace sw::universal
