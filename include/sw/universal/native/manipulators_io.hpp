#pragma once
// manipulators_io.hpp: printing helpers for the native IEEE-754 types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// native/manipulators.hpp produces STRINGS (to_binary, to_triple, to_hex, ...) and
// needs no <iostream>. This header holds the helpers that PRINT them, so a
// translation unit that only formats does not pay for the standard streams.
// See #1334.
#include <iostream>
#include <universal/native/manipulators.hpp>

namespace sw { namespace universal {

	// print representations of an IEEE-754 floating-point
	template<typename Real>
    void valueRepresentations(Real value, bool showhex = false) {
		using namespace sw::universal;
		std::cout << "IEEE-754 type : " << type_tag<Real>() << '\n';

		std::cout << "binary : " << to_binary(value) << '\n';
		std::cout << "triple : " << to_triple(value) << '\n';
		std::cout << "base2  : " << to_base2_scientific(value) << '\n';
		std::cout << "base10 : " << value << '\n';
		std::cout << "color  : " << color_print(value) << '\n';
	    if (showhex) std::cout << "hex    : " << to_hex(value) << '\n';
	}


}} // namespace sw::universal
