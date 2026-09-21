#pragma once
//  test_formats.hpp : functions for test type formats reporting
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

namespace sw { namespace universal {

	// Report every rendering a number system offers for a value.
	//
	// This used to call type_tag, type_field, to_binary, to_hex, to_triple, info_print,
	// pretty_print and color_print unconditionally, which meant it only compiled for a
	// type that provided all eight. Surveying include/sw/universal/number/, exactly five
	// of the thirty-nine number systems do: cfloat, dbns, fixpnt, integer and lns. Even
	// posit is missing type_field and to_hex, and type_field alone is absent from thirty
	// of them (#1582).
	//
	// So this template did not compile for the overwhelming majority of the library's own
	// types, and nothing in the tree instantiated it -- which is why that never surfaced.
	//
	// Filling the surface would mean adding well over a hundred functions across the
	// number systems. Detecting each one instead costs nothing, makes ReportFormats work
	// for every type today, and lets a renderer appear later without touching this file.
	// A type gets whatever it offers, and a missing renderer is reported as missing rather
	// than failing the build.
	template<typename Scalar>
	void ReportFormats(const Scalar& a) {
		std::cout << type_tag(a) << '\n';

		if constexpr (requires { type_field(a); }) std::cout << type_field(a) << '\n';

		if constexpr (requires { to_binary(a); }) {
			std::cout << "binary formats : " << to_binary(a);
			if constexpr (requires { to_binary(a, true); }) std::cout << " : " << to_binary(a, true);
			std::cout << " : " << a << '\n';
		}

		if constexpr (requires { to_hex(a); }) {
			std::cout << "hex formats    : " << to_hex(a);
			if constexpr (requires { to_hex(a, false, false); }) {
				std::cout << " : " << to_hex(a, false, false) << " : " << to_hex(a, true);
			}
			std::cout << " : " << a << '\n';
		}

		if constexpr (requires { to_triple(a); })   std::cout << "triple format  : " << to_triple(a)   << " : " << a << '\n';
		if constexpr (requires { info_print(a); })  std::cout << "info_print     : " << info_print(a)  << " : " << a << '\n';
		if constexpr (requires { pretty_print(a); })std::cout << "pretty_print   : " << pretty_print(a)<< " : " << a << '\n';
		if constexpr (requires { color_print(a); }) std::cout << "color_print    : " << color_print(a) << " : " << a << '\n';
	}

	// Which of the eight renderings a number system actually offers, as a one-line
	// summary. Useful when deciding where to spend effort filling the surface (#1582).
	template<typename Scalar>
	std::string ReportFormatSurface(const Scalar& a = {}) {
		std::stringstream s;
		s << (requires { type_tag(a); }     ? "type_tag "     : "")
		  << (requires { type_field(a); }   ? "type_field "   : "")
		  << (requires { to_binary(a); }    ? "to_binary "    : "")
		  << (requires { to_hex(a); }       ? "to_hex "       : "")
		  << (requires { to_triple(a); }    ? "to_triple "    : "")
		  << (requires { info_print(a); }   ? "info_print "   : "")
		  << (requires { pretty_print(a); } ? "pretty_print " : "")
		  << (requires { color_print(a); }  ? "color_print"   : "");
		return s.str();
	}

}} // namespace sw::universal
