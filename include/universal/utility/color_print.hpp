#pragma once
// color_print.hpp: base classes to print color to a shell
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>

namespace sw { namespace universal {

struct ColorCode {
	static constexpr int FG_DEFAULT = 39;
	static constexpr int  FG_BLACK = 30;
	static constexpr int  FG_RED = 31;
	static constexpr int  FG_GREEN = 32;
	static constexpr int  FG_YELLOW = 33;
	static constexpr int  FG_BLUE = 34;
	static constexpr int  FG_MAGENTA = 35;
	static constexpr int  FG_CYAN = 36;
	static constexpr int  FG_LIGHT_GRAY = 37;
	static constexpr int  FG_DARK_GRAY = 90;
	static constexpr int  FG_LIGHT_RED = 91;
	static constexpr int  FG_LIGHT_GREEN = 92;
	static constexpr int  FG_LIGHT_YELLOW = 93;
	static constexpr int  FG_LIGHT_BLUE = 94;
	static constexpr int  FG_LIGHT_MAGENTA = 95;
	static constexpr int  FG_LIGHT_CYAN = 96;
	static constexpr int  FG_WHITE = 97;

	static constexpr int  BG_DEFAULT = 49;
	static constexpr int  BG_BLACK = 40;
	static constexpr int  BG_RED = 41;
	static constexpr int  BG_GREEN = 42;
	static constexpr int  BG_YELLOW = 43;
	static constexpr int  BG_BLUE = 44;
	static constexpr int  BG_MAGENTA = 45;
	static constexpr int  BG_CYAN = 46;
	static constexpr int  BG_LIGHT_GRAY = 47;
	static constexpr int  BG_DARK_GRAY = 100;
	static constexpr int  BG_LIGHT_RED = 101;
	static constexpr int  BG_LIGHT_GREEN = 102;
	static constexpr int  BG_LIGHT_YELLOW = 103;
	static constexpr int  BG_LIGHT_BLUE = 104;
	static constexpr int  BG_LIGHT_MAGENTA = 105;
	static constexpr int  BG_LIGHT_CYAN = 106;
	static constexpr int  BG_WHITE = 107;
};

class Color {
	int code;
public:
	Color(int pCode) : code(pCode) {}
	friend std::ostream& operator<<(std::ostream& os, const Color& mod) {
		return os << "\033[" << mod.code << "m";
	}
};

inline void show_console_colors(std::ostream& os) {
	// enumerate all the colors
	Color def(ColorCode::FG_DEFAULT);
	for (int code = 0; code < 108; ++code) {
		Color c(code);
		os << c << " CODE " << std::setw(3) << code << def << '\n';
	}
}
	
}} // namespace sw::universal

