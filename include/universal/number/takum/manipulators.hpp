#pragma once
// manipulators.hpp: definitions of helper functions for takum numbers manipulation
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
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
	inline std::string color_print(const TakumType& l, bool nibbleMarker = false) {

		std::stringstream s;

		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color blue(ColorCode::FG_BLUE);
		Color magenta(ColorCode::FG_MAGENTA);
		Color cyan(ColorCode::FG_CYAN);
		Color white(ColorCode::FG_WHITE);
		Color def(ColorCode::FG_DEFAULT);
		s << red << (l.sign() ? "1" : "0");
	
		s << def;
		return s.str();
	}

}} // namespace sw::universal
