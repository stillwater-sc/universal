#pragma once
// attributes.hpp: information functions for positional integer type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// free function sign
template<unsigned ndigits, unsigned radix>
bool sign(const positional<ndigits, radix>& v) {
	return v.sign();
}

template<unsigned ndigits, unsigned radix>
int scale(const positional<ndigits, radix>& v) {
	return v.scale();
}

// generate the maxneg through maxpos value range of a positional integer configuration
template<unsigned ndigits, unsigned radix>
std::string positional_range(const positional<ndigits, radix>& v) {
	positional<ndigits, radix> p{ v };
	std::stringstream s;
	s << std::setw(45) << type_tag(p) << " : [ "
		<< p.maxneg() << " ... "
		<< p.minneg() << " "
		<< "0 "
		<< p.minpos() << " ... "
		<< p.maxpos() << " ]";
	return s.str();
}

}} // namespace sw::universal
