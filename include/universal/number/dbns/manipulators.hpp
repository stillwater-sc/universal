#pragma once
// manipulators.hpp: definitions of helper functions for double base number system value manipulation
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

	// Generate a type tag for this double base number system
	template<typename DbnsType,
		std::enable_if_t< is_dbns<DbnsType>, bool> = true
	>
	inline std::string type_tag(const DbnsType& = {}) {
		std::stringstream s;
		s << "dbns<"
			<< std::setw(3) << DbnsType::nbits << ", "
			<< std::setw(3) << DbnsType::fbbits << ", "
			<< typeid(typename DbnsType::BlockType).name() << ", "
			<< std::setw(10) << type_tag(DbnsType::behavior) << '>';
		return s.str();
	}

#ifdef DEPRECATED
	template<typename DbnsType,
		std::enable_if_t< is_dbns<DbnsType>, bool> = true
	>
	inline std::string range(const DbnsType & = {}) {
		std::stringstream s;
		DbnsType b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
		s << "[" << b << " ... " << c << ", 0, " << d << " ... " << e << ']';
		return s.str();
	}
#endif

	// report if a native floating-point value is within the dynamic range of the dbns configuration
	template<typename DbnsType,
		std::enable_if_t< is_dbns<DbnsType>, bool> = true
	>
	inline bool isInRange(double v) {
		DbnsType a{};

		bool inside = true;
		if (v > double(a.maxpos()) || v < double(a.maxneg())) inside = false;
		return inside;
	}

	template<typename DbnsType,
		std::enable_if_t< is_dbns<DbnsType>, bool> = true
	>
	inline std::string color_print(const DbnsType& l, bool nibbleMarker = false) {

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
		constexpr int lsbFirstBase = static_cast<int>(DbnsType::nbits - DbnsType::fbbits - 1);
		for (int i = static_cast<int>(DbnsType::nbits) - 2; i >= lsbFirstBase; --i) {
			s << cyan << (l.at(static_cast<unsigned>(i)) ? '1' : '0');
			if ((i - DbnsType::fbbits) > 0 && ((i - DbnsType::fbbits) % 4) == 0 && nibbleMarker) s << yellow << '\'';
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
