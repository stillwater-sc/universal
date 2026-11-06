#pragma once
// attributes.hpp: information functions for faithful number and value attributes
//
// Copyright (C) 2023-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Part of the text layer (#1334): sign() and scale() are value queries, but faithful_range()
// streams faithful values through a stringstream, so this header sits above iostream.hpp and is
// not included by core.hpp. Until #1949 faithful_range() was another type's, written for nbits and
// a block type, and the umbrella left this header out because it did not compile.
#include <iomanip>       // std::setw
#include <sstream>       // std::stringstream
#include <string>        // std::string
#include <universal/number/faithful/core.hpp>
#include <universal/number/faithful/manipulators.hpp>   // type_tag
#include <universal/number/faithful/iostream.hpp>       // operator<< on faithful

namespace sw { namespace universal {

// the sign and the binary scale of the value a faithful represents, value + error
template<typename FloatingPointType>
inline bool sign(const faithful<FloatingPointType>& v) {
	return v.sign();
}

template<typename FloatingPointType>
inline int scale(const faithful<FloatingPointType>& v) {
	return v.scale();
}

// the maxneg through maxpos value range of a faithful configuration
template<typename FloatingPointType>
inline std::string faithful_range(const faithful<FloatingPointType>& v = {}) {
	using Faithful = faithful<FloatingPointType>;
	Faithful lmaxneg, lminneg, lminpos, lmaxpos;
	std::stringstream s;
	s << std::setw(45) << type_tag(v) << " : [ "
	  << maxneg(lmaxneg) << " ... "
	  << minneg(lminneg) << " "
	  << "0 "
	  << minpos(lminpos) << " ... "
	  << maxpos(lmaxpos) << " ]";
	return s.str();
}

}} // namespace sw::universal
