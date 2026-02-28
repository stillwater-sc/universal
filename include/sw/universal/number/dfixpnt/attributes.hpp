#pragma once
// attributes.hpp: information functions for decimal fixed-point type and value attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>
#include <iomanip>

namespace sw { namespace universal {

// generate the maxneg through maxpos value range of a dfixpnt configuration
template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
std::string dfixpnt_range(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& = {}) {
	using Dfixpnt = dfixpnt<ndigits, radix, encoding, arithmetic, bt>;
	Dfixpnt v;
	std::stringstream s;
	s << std::setw(40) << type_tag(v) << " : [ "
		<< v.maxneg() << " ... "
		<< v.minneg() << " "
		<< "       0  "
		<< v.minpos() << " ... "
		<< v.maxpos() << " ]";
	return s.str();
}

// free function to get the sign of a dfixpnt
template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
bool sign(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& v) {
	return v.sign();
}

// free function to get the scale (power of 10 exponent) that approximates the value
template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
int scale(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& v) {
	// the scale is the base-10 exponent of the most significant non-zero digit minus radix
	for (int i = static_cast<int>(ndigits) - 1; i >= 0; --i) {
		if (v.digit(static_cast<unsigned>(i)) != 0)
			return i - static_cast<int>(radix);
	}
	return 0;
}

}} // namespace sw::universal
