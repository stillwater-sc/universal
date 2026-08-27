#pragma once
// triple_io.hpp: the text layer for the abstract scientific-notation triple
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// operator<<, operator>> and components() for triple<> (#1334). triple.hpp is included
// by six number-system impl headers -- lns, takum, dbns, rational, faithful and the
// one-parameter skeleton -- so leaving three text producers in it put <iomanip>,
// <sstream>, <ostream> and <istream> into every one of their arithmetic cores. Same
// split blockbinary, blocksignificand, blockfraction and blocktriple already had
// (#1394, #1401, #1403).
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <universal/internal/abstract/triple.hpp>

namespace sw { namespace universal {

////////////////////// VALUE operators
template<size_t nfbits, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const triple<nfbits,nbt>& v) {
	if (v._inf) {
		ostr << FP_INFINITE;
	}
	else {
		ostr << (long double)v;
	}
	return ostr;
}

template<size_t nfbits, typename nbt>
inline std::istream& operator>> (std::istream& istr, const triple<nfbits,nbt>& v) {
	istr >> v._fraction;
	return istr;
}
template<size_t fbits, typename BlockType>
inline std::string components(const triple<fbits,BlockType>& v) {
	std::stringstream s;
	if (v.iszero()) {
		s << "(+,0," << std::setw(fbits) << v.fraction() << ')';
		return s.str();
	}
	else if (v.isinf()) {
		s << "(inf," << std::setw(fbits) << v.fraction() << ')';
		return s.str();
	}
	s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ')';
	return s.str();
}

}} // namespace sw::universal
