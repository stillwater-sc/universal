#pragma once
// manipulators.hpp: the functions that turn a faithful into a std::string
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the faithful headers (#1334): the <iomanip> half. iostream.hpp is the <iostream>
// half. Until #1936 this file was a copy of another type's manipulators, written for nbits, rbits
// and at(), which faithful does not have, and the umbrella left it out because it did not
// compile; components() lived in iostream.hpp instead and called a fraction() faithful lacks.
#include <iomanip>       // std::setprecision
#include <limits>        // std::numeric_limits<>::max_digits10
#include <sstream>       // std::stringstream
#include <string>        // std::string
#include <universal/native/ieee754.hpp>
#include <universal/native/ieee754_type_tag.hpp>
#include <universal/number/faithful/core.hpp>

namespace sw { namespace universal {

// the type tag: faithful<double>, faithful<float>
template<typename FloatingPointType>
inline std::string type_tag(const faithful<FloatingPointType>& = {}) {
	return std::string("faithful<") + type_tag(FloatingPointType{}) + '>';
}

// the two parts of a faithful: the sign and scale of the represented value, then the rounded value
// and the error it carries, each to enough digits to read back exactly
template<typename FloatingPointType>
inline std::string components(const faithful<FloatingPointType>& v) {
	std::stringstream s;
	if (v.isnan()) {
		s << "nan";
		return s.str();
	}
	if (v.iszero()) {
		s << "zero";
		return s.str();
	}
	if (v.isinf()) {
		s << (v.sign() ? "-inf" : "+inf");
		return s.str();
	}
	s << std::setprecision(std::numeric_limits<FloatingPointType>::max_digits10)
	  << "( value " << v.high() << ", error " << v.low() << ")";
	return s.str();
}

template<typename FloatingPointType>
inline std::string to_binary(const faithful<FloatingPointType>& v, bool nibbleMarker = false) {
	std::stringstream s;
	if (v.isnan()) {
		s << "( nan, 0)";
		return s.str();
	}
	if (v.iszero()) {
		s << "( zero, 0)";
		return s.str();
	}
	if (v.isinf()) {
		s << (v.sign() ? "(-inf, 0)" : "(+inf, 0)");
		return s.str();
	}
	FloatingPointType value = v.high();
	FloatingPointType error = v.low();
	s << std::setprecision(std::numeric_limits<FloatingPointType>::max_digits10) << "("
	  << to_binary(value, nibbleMarker) << ", " << to_binary(error, nibbleMarker) << ")";
	return s.str();
}

}} // namespace sw::universal
