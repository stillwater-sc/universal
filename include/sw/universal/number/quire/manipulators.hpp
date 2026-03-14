#pragma once
// manipulators.hpp: definitions of helper functions for manipulation of generalized quire types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <iomanip>
#include <cmath>  // for frexp/frexpf
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for this generalized quire
	template<typename NumberType, unsigned capacity, typename LimbType>
	std::string type_tag(const quire<NumberType, capacity, LimbType>& = {}) {
		std::stringstream str;
		str << "quire<"
			<< type_tag(NumberType{}) << ", "
			<< capacity << ", "
			<< type_tag(LimbType{}) << '>';
		return str.str();
	}

	// report the dynamic range of a quire
	template<typename NumberType, unsigned capacity, typename LimbType>
	std::string quire_range() {
		std::stringstream str;
	    str << type_tag(quire<NumberType, capacity, LimbType>{}) << " range: ";
		str << "minimum " << std::setw(12) << std::numeric_limits<sw::universal::quire<NumberType, capacity, LimbType>>::min() << "     ";
		str << "maximum " << std::setw(12) << std::numeric_limits<sw::universal::quire<NumberType, capacity, LimbType>>::max() ;
		return str.str();
	}


	template<typename NumberType, unsigned capacity, typename LimbType>
	std::string color_print(const quire<NumberType, capacity, LimbType>& q) {
		using Traits = quire_traits<NumberType>;
		constexpr unsigned qbits = Traits::range + capacity;
		constexpr unsigned rp    = Traits::radix_point;

		std::stringstream str;
		Color red(ColorCode::FG_RED);
		Color cyan(ColorCode::FG_CYAN);
		Color magenta(ColorCode::FG_MAGENTA);
		Color def(ColorCode::FG_DEFAULT);

		// sign bit
		str << red << (q.isneg() ? "1" : "0");
		// upper bits (above radix point)
		str << cyan;
		for (int i = static_cast<int>(qbits) - 1; i >= static_cast<int>(rp); --i) {
			str << (q[static_cast<unsigned>(i)] ? '1' : '0');
		}
		str << '.';
		// lower bits (below radix point)
		str << magenta;
		for (int i = static_cast<int>(rp) - 1; i >= 0; --i) {
			str << (q[static_cast<unsigned>(i)] ? '1' : '0');
		}
		str << def;
		return str.str();
	}

}} // namespace sw::universal
