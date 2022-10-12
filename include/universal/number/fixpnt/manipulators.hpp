#pragma once
// manipulators.hpp: definition of manipulation functions for fixed-point types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>

namespace sw { namespace universal {

// Generate a type tag for general fixpnt
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
std::string type_tag(const fixpnt<nbits, rbits, arithmetic, bt>& = {}) {
	std::stringstream s;
	s << "fixpnt<"
		<< std::setw(3) << nbits << ", "
		<< std::setw(3) << rbits << ", "
		<< (arithmetic ? "    Modulo, " : 
			             "Saturating, ")
		<< typeid(bt).name() << '>';
	return s.str();
}

}} // namespace sw::universal
