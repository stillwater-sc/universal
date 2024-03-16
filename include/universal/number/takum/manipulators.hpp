#pragma once
// manipulators.hpp: definitions of helper functions for takum number manipulation
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for this takum
	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline std::string type_tag(const TakumType & = {}) {
		std::stringstream s;
		typename TakumType::BlockType bt{0};
		s << "takum<"
			<< std::setw(3) << TakumType::nbits << ", "
			<< type_tag(bt) << ">";
		return s.str();
	}

	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline std::string range(const TakumType & = {}) {
		std::stringstream s;
		TakumType b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
		s << "[" << b << " ... " << c << ", 0, " << d << " ... " << e << "]\n";
		return s.str();
	}

	// report if a native floating-point value is within the dynamic range of the takum configuration
	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline bool isInRange(double v) {
		TakumType a{};

		bool inside = true;
		if (v > double(a.maxpos()) || v < double(a.maxneg())) inside = false;
		return inside;
	}

	template<typename TakumType,
		std::enable_if_t< is_takum<TakumType>, bool> = true
	>
	inline std::string color_print(const TakumType& number, bool nibbleMarker = false) {

		std::stringstream s;

		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color blue(ColorCode::FG_BLUE);
		Color green(ColorCode::FG_GREEN);
		Color magenta(ColorCode::FG_MAGENTA);
		Color cyan(ColorCode::FG_CYAN);
		Color white(ColorCode::FG_WHITE);
		Color def(ColorCode::FG_DEFAULT);
		s << red << (number.sign() ? '1' : '0');
		bool D = number.direct();
		s << green << (D ? '1' : '0');
		s << yellow;
		int bit = static_cast<int>(TakumType::nbits) - 3;
		for (int i = 0; (i < 3) && (bit >= 0); ++i) {
			s << (number.at(static_cast<unsigned>(bit--)) ? '1' : '0');
		}
		// exponent field
		s << cyan;
		unsigned regime = number.regime();
		int r = static_cast<int>(D ? regime : 7 - regime);
		for (int i = r - 1; i >= 0 && bit >= 0; --i) {
			s << (number.at(static_cast<unsigned>(bit--)) ? '1' : '0');
			if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
		}
		// fraction field
		s << blue;
		while (bit >= 0) {
			s << (number.at(static_cast<unsigned>(bit)) ? '1' : '0');
			if (bit > 0 && (bit % 4) == 0 && nibbleMarker) s << '\'';
			--bit;

		}
		s << def;
		return s.str();
	}

}} // namespace sw::universal
