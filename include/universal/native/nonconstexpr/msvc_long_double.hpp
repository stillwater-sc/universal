#pragma once
// msvc_long_double.hpp: nonconstexpr implementation of IEEE-754 long double manipulators
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
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

// specialization for IEEE long double precision floats
inline std::string to_base2_scientific(long double number) {
	return to_base2_scientific(double(number));
}

// generate a binary string for a native long double precision IEEE floating point
inline std::string to_hex(long double number) {
	return to_hex(double(number));
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

#ifdef CPLUSPLUS_17
inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, std::uint64_t& _fraction) {
	static_assert(std::numeric_limits<long double>::digits <= 64, "This function only works when long double significant is <= 64 bit.");
	if constexpr (sizeof(long double) == 8) { // it is just a double
		_sign = fp < 0.0 ? true : false;
		_fr = std::frexp(double(fp), &_exponent);
		_fraction = uint64_t(0x000FFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr);
	}
	else if constexpr (sizeof(long double) == 16 && std::numeric_limits<long double>::digits <= 64) {
		_sign = fp < 0.0 ? true : false;
		_fr = std::frexpl(fp, &_exponent);
		_fraction = uint64_t(0x7FFFFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr); // 80bit extended format only has 63bits of fraction
	}
}
#else
#ifdef _MSC_VER
#pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#endif

inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, std::uint64_t& _fraction) {
	static_assert(std::numeric_limits<long double>::digits <= 64, "This function only works when long double significant is <= 64 bit.");
	if constexpr (sizeof(long double) == 8) { // check if (long double) is aliased to be just a double
		_sign = fp < 0.0 ? true : false;
		_fr = std::frexp(double(fp), &_exponent);
		_fraction = uint64_t(0x000FFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr);
	}
	else if constexpr (sizeof(long double) == 16 && std::numeric_limits<long double>::digits <= 64) {
		_sign = fp < 0.0 ? true : false;
		_fr = std::frexpl(fp, &_exponent);
		_fraction = uint64_t(0x7FFFFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr); // 80bit extended format only has 63bits of fraction
	}
}
#endif

}} // namespace sw::universal

#endif // MSVC 

