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
	s << "fixpnt<"
		<< nbits << ", "
		<< rbits << ", "
		<< (arithmetic ? "Modulo, " : "Saturating, ")
		<< typeid(bt).name() << "> = " << v;
	return s.str();
}

// Generate a type tag for this fixpnt
template<typename FixedPoint>
std::string type_tag() {
	constexpr size_t nbits = FixedPoint::nbits;
	constexpr size_t rbits = FixedPoint::rbits;
	using bt = typename FixedPoint::BlockType;
	constexpr bool arithmetic = FixedPoint::arithmetic;
	std::stringstream s;
	s << "fixpnt<"
		<< nbits << ", "
		<< rbits << ", "
		<< (arithmetic ? "Modulo, " : "Saturating, ")
		<< typeid(bt).name() << '>';
	return s.str();
}

} // namespace sw::universal
