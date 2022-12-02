#pragma once
// attributes.hpp: information functions for classic floating-point type and value attributes
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
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
	constexpr bool hasSupernormals = CfloatConfiguration::hasSupernormals;
	constexpr bool isSaturating = CfloatConfiguration::isSaturating;

	using Cfloat = cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat v;
	std::stringstream s;
	s << std::setw(80) << type_tag(v) << " : [ "
		<< v.maxneg() << " ... "
		<< v.minneg() << " "
		<< "0 "
		<< v.minpos() << " ... "
		<< v.maxpos() << " ]";
	return s.str();
}

}} // namespace sw::universal
