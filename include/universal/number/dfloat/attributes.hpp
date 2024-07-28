#pragma once
// attributes.hpp: information functions for decimal floating-point type and value attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// functions to provide details about
// the properties of a dfloat configuration
// in terms of native types.

// generate the maxneg through maxpos value range of a dfloat configuration
// TODO: needs SFINAE
template<typename DfloatConfiguration>
std::string dfloat_range() {
	constexpr unsigned ndigits = DfloatConfiguration::nbits;
	constexpr unsigned es = DfloatConfiguration::es;
	using BlockType = typename DfloatConfiguration::BlockType;

	using Dfloat = dfloat<ndigits, es, BlockType>;
	Dfloat v;
	std::stringstream s;
	s << std::setw(80) << type_tag(v) << " : [ "
		<< v.maxneg() << " ... "
		<< v.minneg() << " "
		<< "0 "
		<< v.minpos() << " ... "
		<< v.maxpos() << " ]";
	return s.str();
}


// report dynamic range of a type, specialized for a dfloat
template<unsigned ndigits, unsigned es, typename bt>
std::string dynamic_range(const dfloat<ndigits, es, bt>& a) {
	std::stringstream s;
	dfloat<ndigits, es, bt> b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
	s << type_tag(a) << ": ";
	s << "minpos scale " << std::setw(10) << d.scale() << "     ";
	s << "maxpos scale " << std::setw(10) << e.scale() << '\n';
	s << "[" << b << " ... " << c << ", -0, +0, " << d << " ... " << e << "]\n";
	s << "[" << to_binary(b) << " ... " << to_binary(c) << ", -0, +0, " << to_binary(d) << " ... " << to_binary(e) << "]\n";
	dfloat<ndigits, es, bt> ninf(SpecificValue::infneg), pinf(SpecificValue::infpos);
	s << "inclusive range = (" << to_binary(ninf) << ", " << to_binary(pinf) << ")\n";
	s << "inclusive range = (" << ninf << ", " << pinf << ")\n";
	return s.str();
}


template<unsigned ndigits, unsigned es, typename bt>
int minpos_scale(const dfloat<ndigits, es, bt>& b) {
	dfloat<ndigits, es, bt> c(b);
	return c.minpos().scale();
}

template<unsigned ndigits, unsigned es, typename bt>
int maxpos_scale(const dfloat<ndigits, es, bt>& b) {
	dfloat<ndigits, es, bt> c(b);
	return c.maxpos().scale();
}

template<unsigned ndigits, unsigned es, typename bt>
int max_negative_scale(const dfloat<ndigits, es, bt>& b) {
	dfloat<ndigits, es, bt> c(b);
	return c.maxneg().scale();
}

}} // namespace sw::universal
