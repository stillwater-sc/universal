#pragma once
// color_print.hpp: base classes to print color to a shell
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <iomanip>
#include <cmath>  // for frexp/frexpf
#include <typeinfo>  // for typeid()

// This file contains functions that use the posit type.
// If you have helper functions that the posit type could use, but does not depend on 
// the posit type, you can add them to the file posit_helpers.hpp.

namespace sw { namespace unum {

enum ColorCode {
	FG_DEFAULT = 39,
	FG_BLACK = 30,
	FG_RED = 31,
	FG_GREEN = 32,
	FG_YELLOW = 33,
	FG_BLUE = 34,
	FG_MAGENTA = 35,
	FG_CYAN = 36,
	FG_LIGHT_GRAY = 37,
	FG_DARK_GRAY = 90,
	FG_LIGHT_RED = 91,
	FG_LIGHT_GREEN = 92,
	FG_LIGHT_YELLOW = 93,
	FG_LIGHT_BLUE = 94,
	FG_LIGHT_MAGENTA = 95,
	FG_LIGHT_CYAN = 96,
	FG_WHITE = 97,

	BG_DEFAULT = 49,
	BG_BLACK = 40,
	BG_RED = 41,
	BG_GREEN = 42,
	BG_YELLOW = 43,
	BG_BLUE = 44,
	BG_MAGENTA = 45,
	BG_CYAN = 46,
	BG_LIGHT_GRAY = 47,
	BG_DARK_GRAY = 100,
	BG_LIGHT_RED = 101,
	BG_LIGHT_GREEN = 102,
	BG_LIGHT_YELLOW = 103,
	BG_LIGHT_BLUE = 104,
	BG_LIGHT_MAGENTA = 105,
	BG_LIGHT_CYAN = 106,
	BG_WHITE = 107

};

class Color {
	ColorCode code;
public:
	Color(ColorCode pCode) : code(pCode) {}
	friend std::ostream& operator<<(std::ostream& os, const Color& mod) {
		return os << "\033[" << mod.code << "m";
	}
};

}}  // namespace sw::unum

