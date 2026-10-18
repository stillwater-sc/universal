#pragma once
// attributes.hpp: information functions for lindex numbers and value attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <universal/number/lindex/manipulators.hpp>

namespace sw { namespace universal {

// free function sign
template<unsigned nbits, unsigned lbits, typename bt>
bool sign(const lindex<nbits, lbits, bt>& v) {
	return v.sign();
}

// free function scale: floor(log2 |v|), saturated to int64_t
template<unsigned nbits, unsigned lbits, typename bt>
int64_t scale(const lindex<nbits, lbits, bt>& v) {
	return v.scale();
}

// generate the maxneg through maxpos value range
template<unsigned nbits, unsigned lbits, typename bt>
std::string lindex_range(const lindex<nbits, lbits, bt>& v = {}) {
	using L = lindex<nbits, lbits, bt>;
	L maxneg(SpecificValue::maxneg), minneg(SpecificValue::minneg), minpos(SpecificValue::minpos), maxpos(SpecificValue::maxpos);
	std::stringstream s;
	s << std::setw(30) << type_tag(v) << " : [ "
	  << to_value_string(maxneg) << " ... "
	  << to_value_string(minneg) << " 0 "
	  << to_value_string(minpos) << " ... "
	  << to_value_string(maxpos) << " ]";
	return s.str();
}

}} // namespace sw::universal
