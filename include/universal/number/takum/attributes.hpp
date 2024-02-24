#pragma once
// attributes.hpp: information functions for two-parameter number and value attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
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
template<unsigned nbits, typename bt>
bool sign(const takum<nbits, bt>& v) {
	return v.sign();
}

// generate the maxneg through maxpos value range of a takum number system configuration
template<unsigned nbits, typename bt>
std::string takum_range(const takum<nbits, bt>& v = {}) {
	std::stringstream s;
	s << std::setw(45) << type_tag(v) << " : [ "
		<< v.maxneg() << " ... "
		<< v.minneg() << " "
		<< "0 "
		<< v.minpos() << " ... "
		<< v.maxpos() << " ]";
	return s.str();
}

}} // namespace sw::universal
