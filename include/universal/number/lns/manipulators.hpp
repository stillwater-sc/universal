#pragma once
// manipulators.hpp: definitions of helper functions for logarithmic numbers manipulation
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

// Generate a type tag for this lns
template<size_t nbits, typename bt>
std::string type_tag(const lns<nbits, bt>& v) {
	std::stringstream s;
	s << "lns<"
		<< std::setw(3) << nbits << ", "
		<< typeid(bt).name() << '>';
	if (v.iszero()) s << ' ';
	return s.str();
}

/* TBD
// report dynamic range of a type, specialized for lns
template<size_t nbits, typename bt>
std::string dynamic_range(const lns<nbits, bt>& a) {
	std::stringstream s;
	lns<nbits, bt> b(SpecificValue::maxneg), c(SpecificValue::minneg), d(SpecificValue::minpos), e(SpecificValue::maxpos);
	s << type_tag(a) << ": ";
	s << "minpos scale " << std::setw(10) << d.scale() << "     ";
	s << "maxpos scale " << std::setw(10) << e.scale() << '\n';
	s << "[" << b << " ... " << c << ", -0, +0, " << d << " ... " << e << "]\n";
	s << "[" << to_binary(b) << " ... " << to_binary(c) << ", -0, +0, " << to_binary(d) << " ... " << to_binary(e) << "]\n";
	return s.str();
}
*/

}} // namespace sw::universal
