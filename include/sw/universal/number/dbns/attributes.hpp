#pragma once
// attributes.hpp: information functions for double base number system value attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath> // for std:pow()

namespace sw { namespace universal {

// functions to provide details about
// the properties of a logarithmic number system configuration
// in terms of native types.
// Since many logarithmic number system configurations cannot be represented by native types,
// these are all convenience functions.
// They should not be used for the core algorithms.

	// free function sign
template<unsigned nbits, unsigned fbbits, typename bt, auto ...xtra>
bool sign(const dbns<nbits, fbbits, bt, xtra...>& v) {
	return v.sign();
}

// generate the maxneg through maxpos value range of a logarithmic number system configuration
// the type of arithmetic, Modulo or Saturating, does not affect the range
template<unsigned nbits, unsigned fbbits, typename bt, auto ...xtra>
std::string dbns_range(const dbns<nbits, fbbits, bt, xtra...>& v) {
	using DBNS = dbns<nbits, fbbits, bt, xtra...>;
	std::stringstream s;
	DBNS l;
	s << std::setw(45) << type_tag(v) << " : [ "
		<< l.maxneg() << " ... "
		<< l.minneg() << " "
		<< "0 "
		<< l.minpos() << " ... "
		<< l.maxpos() << " ]";
	return s.str();
}

}} // namespace sw::universal
