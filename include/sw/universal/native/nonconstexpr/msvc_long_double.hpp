#pragma once
// msvc_long_double.hpp: nonconstexpr implementation of IEEE-754 long double manipulators
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#if defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */
// Visual C++ compiler is 15.00.20706.01, the _MSC_FULL_VER will be 15002070601

namespace sw { namespace universal {

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// compiler specific long double IEEE floating point

// Visual C++ does not support long double, it is just an alias for double
inline std::tuple<bool, int, std::uint64_t> ieee_components(long double fp) {
	return ieee_components(double(fp));
}

inline std::string to_hex(long double fp, bool nibbleMarker = false, bool hexPrefix = true) {
	return to_hex(double(fp), nibbleMarker, hexPrefix);
}

// specialization for IEEE long double precision floats
inline std::string to_base2_scientific(long double number) {
	return to_base2_scientific(double(number));
}

// generate a binary string for a native long double precision IEEE floating point
inline std::string to_binary(long double number, bool bNibbleMarker = false) {
	return to_binary(double(number), bNibbleMarker);
}

// return in triple form (+, scale, fraction)
inline std::string to_triple(long double number) {
	return to_triple(double(number));
}

// generate a color coded binary string for a native long double precision IEEE floating point
inline std::string color_print(long double number) {
	return color_print(double(number));
}

}} // namespace sw::universal

#endif // MSVC 

