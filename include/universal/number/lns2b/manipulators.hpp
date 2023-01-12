#pragma once
// manipulators.hpp: definitions of helper functions for 2-base logarithmic numbers manipulation
//
// Copyright (C) 2022-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for this 2-base lns
	template<typename Lns2bType,
		std::enable_if_t< is_lns2b<Lns2bType>, bool> = true
	>
	inline std::string type_tag(const Lns2bType& = {}) {
		std::stringstream s;
		s << "lns2b<"
			<< std::setw(3) << Lns2bType::nbits << ", "
			<< std::setw(3) << Lns2bType::fbbits << ", "
			<< typeid(typename Lns2bType::BlockType).name() << ", "
			<< std::setw(10) << type_tag(Lns2bType::behavior) << '>';
		return s.str();
	}

#ifdef DEPRECATED
	template<typename Lns2bType,
		std::enable_if_t< is_lns2b<Lns2bType>, bool> = true
	>
	inline std::string range(const Lns2bType & = {}) {
		std::stringstream s;
		Lns2bType b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
		s << "[" << b << " ... " << c << ", 0, " << d << " ... " << e << ']';
		return s.str();
	}
#endif

	// report if a native floating-point value is within the dynamic range of the lns2b configuration
	template<typename Lns2bType,
		std::enable_if_t< is_lns2b<Lns2bType>, bool> = true
	>
	inline bool isInRange(double v) {
		Lns2bType a{};

		bool inside = true;
		if (v > double(a.maxpos()) || v < double(a.maxneg())) inside = false;
		return inside;
	}

	template<typename Lns2bType,
		std::enable_if_t< is_lns2b<Lns2bType>, bool> = true
	>
	inline std::string color_print(const Lns2bType& l, bool nibbleMarker = false) {

		std::stringstream s;

		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color blue(ColorCode::FG_BLUE);
		Color magenta(ColorCode::FG_MAGENTA);
		Color cyan(ColorCode::FG_CYAN);
		Color white(ColorCode::FG_WHITE);
		Color def(ColorCode::FG_DEFAULT);
		s << red << (l.sign() ? "1" : "0");
	
		// first base exponent bits
		constexpr int lsbFirstBase = static_cast<int>(Lns2bType::nbits - Lns2bType::fbbits - 1);
		for (int i = static_cast<int>(Lns2bType::nbits) - 2; i >= lsbFirstBase; --i) {
			s << cyan << (l.at(static_cast<unsigned>(i)) ? '1' : '0');
			if ((i - Lns2bType::fbbits) > 0 && ((i - Lns2bType::fbbits) % 4) == 0 && nibbleMarker) s << yellow << '\'';
		}

		// second base exponent bits
		if constexpr (lsbFirstBase > 0) {
			for (int i = lsbFirstBase - 1; i >= 0; --i) {
				s << magenta << (l.at(static_cast<unsigned>(i)) ? '1' : '0');
				if (i > 0 && (i % 4) == 0 && nibbleMarker) s << yellow << '\'';
			}
		}
		s << def;
		return s.str();
	}

}} // namespace sw::universal
