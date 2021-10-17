#pragma once
// manipulators.hpp: definition of manipulation functions for integer types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <exception>

#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */


#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */


#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */


#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

namespace sw::universal {

// return in triple form (sign, scale, fraction)
template<size_t nbits, typename bt>
inline std::string to_triple(const integer<nbits, bt>& number) {
	std::stringstream ss;

	// print sign bit
	ss << '(' << (number < 0 ? '-' : '+') << ',';

	// scale
	ss << scale(number) << ',';

	// print fraction bits
	long msb = findMsb(number);
	if (msb < 0) {
		ss << '-';
	}
	else {
		for (int i = msb-1; i >= 0; --i) {
			ss << (number.at(static_cast<size_t>(i)) ? '1' : '0');
		}
	}

	ss << ')';
	return ss.str();
}

} // namespace sw::universal
