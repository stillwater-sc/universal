#pragma once
// attributes.hpp: information functions for hfloat type and value attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Part of the text layer (#1334): hfloat_range() streams hfloat values through a
// stringstream, so this header sits above iostream.hpp and is not included by core.hpp.
// Self-contained: <string>, <sstream> and <iomanip> were all used but never named.
#include <string>
#include <sstream>
#include <iomanip>   // std::setw
#include <universal/number/hfloat/core.hpp>
#include <universal/number/hfloat/manipulators.hpp>   // type_tag
#include <universal/number/hfloat/iostream.hpp>       // operator<< on hfloat, used by hfloat_range

namespace sw { namespace universal {

// generate the maxneg through maxpos value range of a hfloat configuration
template<typename HfloatConfiguration>
std::string hfloat_range() {
	constexpr unsigned nd = HfloatConfiguration::ndigits;
	constexpr unsigned e  = HfloatConfiguration::es;
	using BlockType = typename HfloatConfiguration::BlockType;

	using Hfloat = hfloat<nd, e, BlockType>;
	Hfloat v;
	std::stringstream s;
	s << std::setw(80) << type_tag(v) << " : [ "
		<< v.maxneg() << " ... "
		<< v.minneg() << " "
		<< "0 "
		<< v.minpos() << " ... "
		<< v.maxpos() << " ]";
	return s.str();
}

// report dynamic range of a type, specialized for hfloat
template<unsigned ndigits, unsigned es, typename bt>
std::string dynamic_range(const hfloat<ndigits, es, bt>& a) {
	std::stringstream s;
	hfloat<ndigits, es, bt> b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
	s << type_tag(a) << ": ";
	s << "minpos scale " << std::setw(10) << d.scale() << "     ";
	s << "maxpos scale " << std::setw(10) << e.scale() << '\n';
	s << "[" << b << " ... " << c << ", 0, " << d << " ... " << e << "]\n";
	s << "[" << to_binary(b) << " ... " << to_binary(c) << ", 0, " << to_binary(d) << " ... " << to_binary(e) << "]\n";
	return s.str();
}

template<unsigned ndigits, unsigned es, typename bt>
int minpos_scale(const hfloat<ndigits, es, bt>& b) {
	hfloat<ndigits, es, bt> c(b);
	return c.minpos().scale();
}

template<unsigned ndigits, unsigned es, typename bt>
int maxpos_scale(const hfloat<ndigits, es, bt>& b) {
	hfloat<ndigits, es, bt> c(b);
	return c.maxpos().scale();
}

template<unsigned ndigits, unsigned es, typename bt>
int max_negative_scale(const hfloat<ndigits, es, bt>& b) {
	hfloat<ndigits, es, bt> c(b);
	return c.maxneg().scale();
}

// free function for scale
template<unsigned ndigits, unsigned es, typename bt>
int scale(const hfloat<ndigits, es, bt>& a) {
	return a.scale();
}

}} // namespace sw::universal
