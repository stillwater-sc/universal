#pragma once
// number_system_properties.hpp: functions to report on number system properties
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <limits>

namespace sw::universal {

	// report the minimum and maximum of a type
	template<typename Ty>
	std::string minmax_range() {
		std::stringstream ss;
		Ty v(0);
		ss << std::setw(30) << typeid(v).name() << ' ';
		ss << "min " << std::setw(13) << std::numeric_limits<Ty>::min() << "     ";
		ss << "max " << std::setw(13) << std::numeric_limits<Ty>::max() << "     ";
		return ss.str();
	}

	// report the negative bounds, zero, and positive bounds of the number system
	template<typename Ty>
	std::string symmetry() {
		std::stringstream ss;
		Ty v(0);
		constexpr unsigned WIDTH = 20;
		ss << std::setw(30) << typeid(v).name() << ' ';
		ss << "[ "
			<< std::setw(WIDTH) << std::numeric_limits<Ty>::lowest()
			<< ", "
			<< std::setw(WIDTH) << -std::numeric_limits<Ty>::denorm_min()
			<< "] 0 [ "
			<< std::setw(WIDTH) << std::numeric_limits<Ty>::denorm_min()
			<< ", "
			<< std::setw(WIDTH) << std::numeric_limits<Ty>::max() << ']';
		return ss.str();
	}

	// report the dynamic range of a number system type
	template<typename Ty>
	std::string dynamic_range() {
		std::stringstream ss;
		Ty v(0);
		ss << std::setw(30) << typeid(v).name();
		ss << ' ';
		ss << "minexp scale " << std::setw(10) << std::numeric_limits<Ty>::min_exponent << "     ";
		ss << "maxexp scale " << std::setw(10) << std::numeric_limits<Ty>::max_exponent << "     ";
		ss << "minimum " << std::setw(12) << std::numeric_limits<Ty>::min() << "     ";
		ss << "maximum " << std::setw(12) << std::numeric_limits<Ty>::max() << "     ";
		return ss.str();
	}

	// report the dynamic range of the type associated with a value
	template<typename Ty>
	std::string dynamic_range(Ty v) {
		std::stringstream ss;
		ss << std::setw(30) << typeid(v).name();
		ss << ' ';
		ss << "minexp scale " << std::setw(10) << std::numeric_limits<Ty>::min_exponent << "     ";
		ss << "maxexp scale " << std::setw(10) << std::numeric_limits<Ty>::max_exponent << "     ";
		ss << "minimum " << std::setw(12) << std::numeric_limits<Ty>::min() << "     ";
		ss << "maximum " << std::setw(12) << std::numeric_limits<Ty>::max() << "     ";
		return ss.str();
	}



}  // namespace sw::universal

