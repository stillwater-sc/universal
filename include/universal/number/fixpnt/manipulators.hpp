#pragma once
// manipulators.hpp: definition of manipulation functions for fixed-point types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <exception>

namespace sw::universal {

// Generate a type tag for this fixpnt
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
std::string type_tag(const fixpnt<nbits, rbits, arithmetic, bt>& v) {
	std::stringstream s;
	if (v.iszero()) s << ' '; // remove 'unreferenced formal parameter warning from compilation log
	s << "fixpnt<"
		<< std::setw(3) << nbits << ", "
		<< std::setw(3) << rbits << ", "
		<< (arithmetic ? "    Modulo, " : 
			             "Saturating, ")
		<< typeid(bt).name() << '>';
	return s.str();
}

// TODO: you need to guard this with a fixpnt type
// as right now this type_tag() design matches any type
// and is thus ambiguous

// Generate a type tag for this fixpnt
template<typename FixedPoint>
std::string type_tag() {
	constexpr size_t nbits = FixedPoint::nbits;
	constexpr size_t rbits = FixedPoint::rbits;
	constexpr bool arithmetic = FixedPoint::arithmetic;
	using bt = typename FixedPoint::BlockType;
	return type_tag(fixpnt<nbits, rbits, arithmetic, bt>());
}

} // namespace sw::universal
