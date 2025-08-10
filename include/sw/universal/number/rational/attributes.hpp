#pragma once
// attributes.hpp: information functions for rational arithmetic type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath> // for std:pow()

namespace sw { namespace universal {

// functions to provide details about
// the properties of the number system configuration
// in terms of native types.
// Since many number system configurations cannot be represented by 
// native types, these are all convenience functions.
// They should not be used for the core algorithms.

	// free function sign
	template<typename RationalType, std::enable_if_t<is_rational<RationalType>, bool> = true>
bool sign(const RationalType& v) {
	return v.sign();
}

template<typename RationalType, std::enable_if_t<is_rational<RationalType>,bool> = true>
int scale(const RationalType& r) {
	return r.scale();
}

// generate the maxneg through maxpos value range of a logarithmic number system configuration
// the type of arithmetic, Modulo or Saturating, does not affect the range
template<typename RationalType, std::enable_if_t<is_rational<RationalType>, bool> = true>
std::string rational_range(const RationalType& v) {
	RationalType r{ v };
	std::stringstream s;
	s << std::setw(45) << type_tag(r) << " : [ "
		<< r.maxneg() << " ... "
		<< r.minneg() << " "
		<< "0 "
		<< r.minpos() << " ... "
		<< r.maxpos() << " ]";
	return s.str();
}

}} // namespace sw::universal
