#pragma once
// attributes.hpp: information functions for classic floating-point type and value attributes
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath> // for std:pow()
//#include <universal/internal/bitblock/bitblock.hpp>

namespace sw { namespace universal {

// functions to provide details about
// the properties of a cfloat configuration
// in terms of native types.

// calculate exponential scale of maxpos
template<size_t nbits, size_t es, typename BlockType, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
int scale_maxpos_cfloat() {
	constexpr cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> maxpos(SpecificValue::maxpos);
	return maxpos.scale();
}

// calculate exponential scale of minpos
template<size_t nbits, size_t es, typename BlockType, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
int scale_minpos_cfloat() {
	constexpr cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> minpos(SpecificValue::minpos);
	return minpos.scale();
}

// generate the maxneg through maxpos value range of a cfloat configuration
// TODO: needs SFINAE
template<typename CfloatConfiguration>
void report_range(std::ostream& ostr) {
	constexpr size_t nbits = CfloatConfiguration::nbits;
	constexpr size_t es = CfloatConfiguration::es;
	using BlockType = typename CfloatConfiguration::BlockType;
	constexpr bool hasSubnormals = CfloatConfiguration::hasSubnormals;
	constexpr bool hasSupernormals = CfloatConfiguration::hasSupernormals;
	constexpr bool isSaturating = CfloatConfiguration::isSaturating;
	using Cfloat = cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat c{};
	ostr << std::setw(40) << type_tag(c) << " : [ "
		<< c.maxneg() << " ... "
		<< c.minneg() << " "
		<< "0 "
		<< c.minpos() << " ... "
		<< c.maxpos() << " ]\n";
}

}} // namespace sw::universal
