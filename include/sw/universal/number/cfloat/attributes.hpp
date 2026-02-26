#pragma once
// attributes.hpp: information functions for classic floating-point type and value attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// functions to provide details about
// the properties of a cfloat configuration
// in terms of native types.

// generate the maxneg through maxpos value range of a cfloat configuration
// TODO: needs SFINAE
template<typename CfloatConfiguration>
std::string cfloat_range() {
	constexpr unsigned nbits = CfloatConfiguration::nbits;
	constexpr unsigned es = CfloatConfiguration::es;
	using BlockType = typename CfloatConfiguration::BlockType;
	constexpr bool hasSubnormals = CfloatConfiguration::hasSubnormals;
	constexpr bool hasMaxExpValues = CfloatConfiguration::hasMaxExpValues;
	constexpr bool isSaturating = CfloatConfiguration::isSaturating;

	using Cfloat = cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating>;
	Cfloat v{ 0 };
	std::stringstream s;
	s << std::setw(80) << type_tag(v) << " : [ "
		<< v.maxneg() << " ... "
		<< v.minneg() << " "
		<< "0 "
		<< v.minpos() << " ... "
		<< v.maxpos() << " ]";
	return s.str();
}


// report dynamic range of a type, specialized for a cfloat
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
std::string dynamic_range(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& a) {
	std::stringstream s;
	cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
	s << type_tag(a) << ": ";
	s << "minpos scale " << std::setw(10) << d.scale() << "     ";
	s << "maxpos scale " << std::setw(10) << e.scale() << '\n';
	s << "[" << b << " ... " << c << ", -0, +0, " << d << " ... " << e << "]\n";
	s << "[" << to_binary(b) << " ... " << to_binary(c) << ", -0, +0, " << to_binary(d) << " ... " << to_binary(e) << "]\n";
	cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> ninf(SpecificValue::infneg), pinf(SpecificValue::infpos);
	s << "inclusive range = (" << to_binary(ninf) << ", " << to_binary(pinf) << ")\n";
	s << "inclusive range = (" << ninf << ", " << pinf << ")\n";
	return s.str();
}


template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
int minpos_scale(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& b) {
	cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> c(b);
	return c.minpos().scale();
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
int maxpos_scale(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& b) {
	cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> c(b);
	return c.maxpos().scale();
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
int max_negative_scale(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& b) {
	cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> c(b);
	return c.maxneg().scale();
}

}} // namespace sw::universal
