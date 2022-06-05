#pragma once
// manipulators.hpp: definitions of helper functions for logarithmic numbers manipulation
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

namespace sw { namespace universal {

	// Generate a type tag for this lns
	template<size_t nbits, size_t rbits, typename BlockType>
	std::string type_tag(const lns<nbits, rbits, BlockType>& l) {
		std::stringstream s;
		s << "lns<"
			<< std::setw(3) << nbits << ", "
			<< std::setw(3) << rbits << ", "
			<< typeid(BlockType).name() << '>';
		if (l.iszero()) s << ' ';
		return s.str();
	}

	template<typename LnsType,
		std::enable_if_t<is_lns<LnsType>, LnsType> = 0
	>
	std::string type_tag() {
		LnsType l{ 1.0 };
		return type_tag(l);
	}

/* TBD
// report dynamic range of a type, specialized for lns
template<size_t nbits, size_t rbits, typename bt>
std::string dynamic_range(const lns<nbits, rbits, bt>& a) {
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
