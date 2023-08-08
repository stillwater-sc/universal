#pragma once
// manipulators.hpp: definitions of helper functions for faithful numbers manipulation
//
// Copyright (C) 2023-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for this faithful
	template<typename FloatingPointType,
		std::enable_if_t< is_faithful<FloatingPointType>, bool> = true
	>
	inline std::string type_tag(const FloatingPointType & = {}) {
		std::stringstream s;
		typename FloatingPointType::BlockType bt{0};
		s << "faithful<"
			<< std::setw(3) << FloatingPointType::nbits << ", "
			<< std::setw(3) << FloatingPointType::rbits << ", "
			<< type_tag(bt) << ", "
		<< std::setw(10) << type_tag(Behavior{ FloatingPointType::behavior }) << '>';
		return s.str();
	}

	template<typename FloatingPointType,
		std::enable_if_t< is_faithful<FloatingPointType>, bool> = true
	>
	inline std::string range(const FloatingPointType & = {}) {
		std::stringstream s;
		FloatingPointType b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
		s << "[" << b << " ... " << c << ", 0, " << d << " ... " << e << "]\n";
		return s.str();
	}

	// report if a native floating-point value is within the dynamic range of the faithful configuration
	template<typename FloatingPointType,
		std::enable_if_t< is_faithful<FloatingPointType>, bool> = true
	>
	inline bool isInRange(double v) {
		FloatingPointType a{};

		bool inside = true;
		if (v > double(a.maxpos()) || v < double(a.maxneg())) inside = false;
		return inside;
	}

	template<typename FloatingPointType,
		std::enable_if_t< is_faithful<FloatingPointType>, bool> = true
	>
	inline std::string color_print(const FloatingPointType& l, bool nibbleMarker = false) {

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
		for (int i = static_cast<int>(FloatingPointType::nbits) - 2; i >= static_cast<int>(FloatingPointType::rbits); --i) {
			s << cyan << (l.at(static_cast<unsigned>(i)) ? '1' : '0');
			if ((i - FloatingPointType::rbits) > 0 && ((i - FloatingPointType::rbits) % 4) == 0 && nibbleMarker) s << yellow << '\'';
		}

		// fraction bits
		if constexpr (FloatingPointType::rbits > 0) {
			s << magenta << '.';
			for (int i = static_cast<int>(FloatingPointType::rbits) - 1; i >= 0; --i) {
				s << magenta << (l.at(static_cast<unsigned>(i)) ? '1' : '0');
				if (i > 0 && (i % 4) == 0 && nibbleMarker) s << yellow << '\'';
			}
		}
		s << def;
		return s.str();
	}

}} // namespace sw::universal
