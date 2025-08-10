#pragma once
// arithmetic_traits.hpp: functions to report on number system properties
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <limits>
#include <universal/native/integer_type_tag.hpp>
#include <universal/native/ieee754_type_tag.hpp>

namespace sw { namespace universal {

	// cfloat is the largest tag
	constexpr unsigned WIDTH_TYPE_TAG = 80;

	// report the minimum and maximum of a type
	template<typename Ty>
	std::string minmax_range(Ty = {}) {
		std::stringstream str;
		str << std::left << std::setw(WIDTH_TYPE_TAG) << type_tag(Ty());
		str << " : ";
		if constexpr (std::numeric_limits<Ty>::has_denorm == std::float_denorm_style::denorm_absent) {
			str << "min " << std::setw(13) << std::numeric_limits<Ty>::min() << "     ";
		}
		else {
			str << "min " << std::setw(13) << std::numeric_limits<Ty>::denorm_min() << "     ";
		}

		str << "max " << std::setw(13) << std::numeric_limits<Ty>::max() << "     ";
		return str.str();
	}

	// report the negative bounds, zero, and positive bounds of the number system
	template<typename Ty>
	std::string symmetry_range(Ty = {}) {
		std::stringstream str;
		str << std::left << std::setw(WIDTH_TYPE_TAG) << type_tag(Ty()) << " : ";
		str << "[ "
			<< std::numeric_limits<Ty>::lowest()
			<< " ... "
			<< -std::numeric_limits<Ty>::denorm_min()
			<< "  0  "
			<< std::numeric_limits<Ty>::denorm_min()
			<< " ... "
			<< std::numeric_limits<Ty>::max() << ']';
		return str.str();
	}

	// report the dynamic range of a number system type
	template<typename Ty>
	std::string dynamic_range(Ty = {}) {
		std::stringstream str;
		str << std::left << std::setw(WIDTH_TYPE_TAG) << type_tag(Ty());
		str << " : ";
		str << "minexp scale " << std::setw(10) << std::numeric_limits<Ty>::min_exponent << "     ";
		str << "maxexp scale " << std::setw(10) << std::numeric_limits<Ty>::max_exponent << "     ";
		str << "minimum " << std::setw(12) << std::numeric_limits<Ty>::min() << "     ";
		str << "maximum " << std::setw(12) << std::numeric_limits<Ty>::max() << "     ";
		return str.str();
	}

}} // namespace sw::universal

