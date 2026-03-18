#pragma once
// attributes.hpp: information functions for takum numbers and value attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// free function sign
template<unsigned nbits, unsigned rbits, typename bt>
bool sign(const takum<nbits, rbits, bt>& v) {
	return v.sign();
}

// generate the maxneg through maxpos value range
template<unsigned nbits, unsigned rbits, typename bt>
std::string takum_range(const takum<nbits, rbits, bt>& v = {}) {
	std::stringstream s;
	s << std::setw(45) << type_tag(v) << " : [ "
		<< v.maxneg() << " ... "
		<< v.minneg() << " "
		<< "0 "
		<< v.minpos() << " ... "
		<< v.maxpos() << " ]";
	return s.str();
}

// report dynamic range of a type, specialized for a takum
template<unsigned nbits, unsigned rbits, typename bt>
std::string dynamic_range(const takum<nbits, rbits, bt>& a) {
	std::stringstream s;
	takum<nbits, rbits, bt> b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
	s << type_tag(a) << ": ";
	s << "minpos scale " << std::setw(10) << d.scale() << "     ";
	s << "maxpos scale " << std::setw(10) << e.scale() << '\n';
	s << "[" << b << " ... " << c << ", -0, +0, " << d << " ... " << e << "]\n";
	s << "[" << to_binary(b) << " ... " << to_binary(c) << ", -0, +0, " << to_binary(d) << " ... " << to_binary(e) << "]\n";
	takum<nbits, rbits, bt> ninf(SpecificValue::infneg), pinf(SpecificValue::infpos);
	s << "inclusive range = (" << to_binary(ninf) << ", " << to_binary(pinf) << ")\n";
	s << "inclusive range = (" << ninf << ", " << pinf << ")\n";
	return s.str();
}

template<unsigned nbits, unsigned rbits, typename bt>
int minpos_scale(const takum<nbits, rbits, bt>& b) {
	takum<nbits, rbits, bt> c(b);
	return c.minpos().scale();
}

template<unsigned nbits, unsigned rbits, typename bt>
int maxpos_scale(const takum<nbits, rbits, bt>& b) {
	takum<nbits, rbits, bt> c(b);
	return c.maxpos().scale();
}

template<unsigned nbits, unsigned rbits, typename bt>
int max_negative_scale(const takum<nbits, rbits, bt>& b) {
	takum<nbits, rbits, bt> c(b);
	return c.maxneg().scale();
}

}} // namespace sw::universal
