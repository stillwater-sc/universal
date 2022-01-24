#pragma once
// ibmxlc_long_double.hpp: nonconstexpr implementation of IEEE-754 long double manipulators
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

namespace sw { namespace universal {

////////////////////////////////////////////////////////////////////////
// numerical helpers


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// compiler specific long double IEEE floating point

// generate a binary string for a native long double precision IEEE floating point
inline std::string to_hex(const long double& number) {
	return std::string("to_hex() not implemented for IBM compiler");
}

// generate a binary string for a native long double precision IEEE floating point
inline std::string to_binary(const long double& number, bool bNibbleMarker = false) {
	return std::string("to_binary() not implemented for IBM compiler");
}

// return in triple form (+, scale, fraction)
inline std::string to_triple(const long double& number) {
	return std::string("to_triple() not implemented for IBM compiler");
}

inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, float& _fr, uint32_t& _fraction) {
	std::cerr << "extract_fp_components not implemented for IBM compiler");
}


}} // namespace sw::universal

#endif // IBM XL C/C++.
