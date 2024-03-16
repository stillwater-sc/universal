#pragma once
//  test_formats.cpp : functions for test type formats reporting
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>

namespace sw { namespace universal {

	template<typename Scalar>
	void ReportFormats(const Scalar& a) {
		std::cout << type_tag(a) << '\n';
		std::cout << type_field(a) << '\n';
		std::cout << "binary formats : " << to_binary(a) << " : " << to_binary(a, true) << " : " << a << '\n';
		std::cout << "hex formats    : " << to_hex(a) << " : " << to_hex(a, false, false) << " : " << to_hex(a, true) << " : " << a << '\n';
		std::cout << "triple format  : " << to_triple(a) << " : " << a << '\n';
		std::cout << "info_print     : " << info_print(a) << " : " << a << '\n';
		std::cout << "pretty_print   : " << pretty_print(a) << " : " << a << '\n';
		std::cout << "color_print    : " << color_print(a) << " : " << a << '\n';
	}

}} // namespace sw::universal
