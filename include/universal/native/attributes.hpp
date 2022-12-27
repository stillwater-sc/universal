#pragma once
// attributes.hpp: definition of attribute functions for native real and integer types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>
#include <string>

namespace sw { namespace universal {

	/*
	 * we are doing this through std::numeric_limits<Scalar> in arithmetic_traits.hpp
	 * 
	 * If you include these functions here then you'll trigger the ambiguous call compilation error

	template<typename IntegralType, 
		std::enable_if_t< std::is_integral<IntegralType>::value, bool> = true
	>
	std::string dynamic_range(IntegralType = {}) {
		return type_string;
	}

	template<typename RealType,
		std::enable_if_t< std::is_floating_point<RealType>::value, bool> = true
	>
	std::string dynamic_range(RealType = {}) {
		std::stringstream s;
		s << "[ " << std::numeric_limits<RealType>::min() << " ... " << std::numeric_limits<RealType>::max() << " ]";
		return s.str();
	}
	*/

	// TODO: how to filter/specialize signed/unsigned
	template<typename IntegralType,
		std::enable_if_t< std::is_integral<IntegralType>::value, bool> = true
	>
	std::string integer_range() {
		std::stringstream s;
		s << std::setw(80) << type_tag(IntegralType()) << " : [ "
			<< std::numeric_limits<IntegralType>::lowest() << " ... "
			<< -std::numeric_limits<IntegralType>::min() << " "
			<< "0 "
			<< std::numeric_limits<IntegralType>::min() << " ... "
			<< std::numeric_limits<IntegralType>::max() << " ]";
		return s.str();
	}

	template<typename RealType,
		unsigned tagWidth = 80,
		std::enable_if_t< std::is_floating_point<RealType>::value, bool> = true
	>
	std::string ieee754_range() {
		std::stringstream s;
		s << std::setw(tagWidth) << type_tag(RealType()) << " : [ "
			<< std::numeric_limits<RealType>::lowest() << " ... "
			<< -std::numeric_limits<RealType>::denorm_min() << " "
			<< "0 "
			<< std::numeric_limits<RealType>::denorm_min() << " ... "
			<< std::numeric_limits<RealType>::max() << " ]";
		return s.str();
	}

	inline std::string float_range() {
		return ieee754_range<float, 15>();
	}
	inline std::string double_range() {
		return ieee754_range<double, 15>();
	}
	inline std::string longdouble_range() {
		return ieee754_range<long double, 15>();
	}

}} // namespace sw::universal
