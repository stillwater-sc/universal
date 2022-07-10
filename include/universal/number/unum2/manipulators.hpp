#pragma once
// manipulators.hpp: definitions of helper functions for unum2 type manipulation
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <cmath>  // for frexp/frexpf
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

// This file contains functions that use the unum2 type.
// If you have helper functions that the unum2 type could use, but dofsize not depend on 
// the unum2 type, you can add them to the file unum_helpers.hpp.

namespace sw { namespace universal {

// DEBUG/REPORTING HELPERS

template<size_t essize, size_t fsize>
std::string unum2_range() {
	std::stringstream ss;
	ss << " unum2<" << std::setw(3) << essize << "," << fsize << "> ";
	//ss << "minpos scale " << std::setw(10) << minpos_scale<essize, fsize>() << "     ";
	//ss << "maxpos scale " << std::setw(10) << maxpos_scale<essize, fsize>() << "     ";
	ss << "minimum " << std::setw(12) << std::numeric_limits<sw::universal::unum2<essize, fsize>>::min() << "     ";
	ss << "maximum " << std::setw(12) << std::numeric_limits<sw::universal::unum2<essize, fsize>>::max() ;
	return ss.str();
}

// Generate a type tag for this unum, for example, unum<8,1>
template<size_t essize, size_t fsize>
std::string type_tag(const unum2<essize, fsize>& = {}) {
	std::stringstream ss;
	ss << "unum2<" << essize << "," << fsize << ">";
	return ss.str();
}

// generate a unum2 format ASCII format essize.fsizexNN...NNp
template<size_t essize, size_t fsize>
inline std::string hex_print(const unum2<essize, fsize>& p) {
	std::stringstream ss;
	ss << essize << '.' << fsize << 'x' << to_hex(p.get()) << 'p';
	return ss.str();
}

template<size_t essize, size_t fsize>
std::string pretty_print(const unum2<essize, fsize>& p, int printPrecision = std::numeric_limits<double>::max_digits10) {
	std::stringstream ss;
	return ss.str();
}

template<size_t essize, size_t fsize>
std::string info_print(const unum2<essize, fsize>& p, int printPrecision = 17) {
	std::stringstream ss;
	return ss.str();
}

template<size_t essize, size_t fsize>
std::string color_print(const unum2<essize, fsize>& p) {
	std::stringstream ss;
	return ss.str();
}

}}  // namespace sw::universal

