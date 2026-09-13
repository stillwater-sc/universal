// show_representations.hpp: show output of different manipulators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries
#include <ostream>
#include <iomanip>

namespace sw {
    namespace universal {

// ShowRepresentations prints the different output formats for the Scalar type
template<typename Scalar>
void ShowRepresentations(std::ostream& ostr, Scalar f) {
	auto defaultPrecision = ostr.precision(); // save stream state

	constexpr int max_digits10 = std::numeric_limits<Scalar>::max_digits10; 	// floating-point attribute for printing scientific format

	Scalar v(f); // convert to target cfloat
	ostr << "scientific   : " << std::setprecision(max_digits10) << v << '\n';
	ostr << "triple form  : " << to_triple(v) << '\n';
	ostr << "binary form  : " << to_binary(v, true) << '\n';
	ostr << "color coded  : " << color_print(v) << '\n';

	ostr << std::setprecision(defaultPrecision);
}

}} // namespace sw::universal