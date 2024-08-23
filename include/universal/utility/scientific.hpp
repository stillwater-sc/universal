#pragma once
// scientific.hpp: transform a value into a scientific format string
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <sstream>
#include <iomanip>

namespace sw { namespace universal {

template<typename Ty>
std::string to_scientific(Ty value) {
	const char* scales[] = { "", "K", "M", "G", "T", "P", "E", "Z" };
	Ty lower_bound = Ty(1);
	Ty scale_factor = 1.0;
	size_t scale = 0;
	for (size_t i = 0; i < sizeof(scales); ++i) {
		if (value >= lower_bound && value < 1000 * lower_bound) {
			scale = i;
			break;
		}
		lower_bound *= 1000;
		scale_factor *= 1000.0;
	}
	int integer_value = int(value / scale_factor);
	std::stringstream ostr;
	ostr << std::setw(3) << std::right << integer_value << ' ' << scales[scale];
	return ostr.str();
}


}} // namespace sw::universal


