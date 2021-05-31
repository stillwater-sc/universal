#pragma once
// ieee754.hpp: manipulation functions for IEEE-754 native types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#if defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */
// Visual C++ compiler is 15.00.20706.01, the _MSC_FULL_VER will be 15002070601

namespace sw::universal {

	// MSVC long double = double precision
// IEEE-754 parameter constexpressions for long double
template<>
class ieee754_parameter<long double> {
public:
	static constexpr uint64_t smask   = 0x8000'0000'0000'0000ull;
	static constexpr int      ebits   = 11;
	static constexpr int      bias    = 1023;
	static constexpr uint64_t emask   = 0x7FF0'0000'0000'0000ull;
	static constexpr uint64_t eallset = 0x7FF;
	static constexpr int      fbits   = 52;
	static constexpr uint64_t fmask   = 0x000F'FFFF'FFFF'FFFFull;
	static constexpr uint64_t fmsb    = 0x0008'0000'0000'0000ull;
};

////////////////////////////////////////////////////////////////////////
// numerical helpers


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// compiler specific long double IEEE floating point

// Visual C++ does not support long double, it is just an alias for double
/*
union long_double_decoder {
	long double ld;
	struct {
		uint64_t fraction : 52;
		uint64_t exponent : 11;
		uint64_t  sign : 1;
	} parts;
};
*/

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

// floating point component extractions
inline void extract_fp_components(float fp, bool& _sign, int& _exponent, float& _fr, uint32_t& _fraction) {
	static_assert(sizeof(float) == 4, "This function only works when float is 32 bit.");
	_sign = fp < 0.0 ? true : false;
	_fr = frexpf(fp, &_exponent);
	_fraction = uint32_t(0x007FFFFFul) & reinterpret_cast<uint32_t&>(_fr);
}
inline void extract_fp_components(double fp, bool& _sign, int& _exponent, double& _fr, uint64_t& _fraction) {
	static_assert(sizeof(double) == 8, "This function only works when double is 64 bit.");
	_sign = fp < 0.0 ? true : false;
	_fr = frexp(fp, &_exponent);
	_fraction = uint64_t(0x000FFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr);
}

#ifdef CPLUSPLUS_17
inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, uint64_t& _fraction) {
	static_assert(std::numeric_limits<long double>::digits <= 64, "This function only works when long double significant is <= 64 bit.");
	if constexpr (sizeof(long double) == 8) { // it is just a double
		_sign = fp < 0.0 ? true : false;
		_fr = frexp(double(fp), &_exponent);
		_fraction = uint64_t(0x000FFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr);
	}
	else if constexpr (sizeof(long double) == 16 && std::numeric_limits<long double>::digits <= 64) {
		_sign = fp < 0.0 ? true : false;
		_fr = frexpl(fp, &_exponent);
		_fraction = uint64_t(0x7FFFFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr); // 80bit extended format only has 63bits of fraction
	}
}
#else
#ifdef _MSC_VER
#pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#endif

inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, uint64_t& _fraction) {
	static_assert(std::numeric_limits<long double>::digits <= 64, "This function only works when long double significant is <= 64 bit.");
	if (sizeof(long double) == 8) { // check if (long double) is aliased to be just a double
		_sign = fp < 0.0 ? true : false;
		_fr = frexp(double(fp), &_exponent);
		_fraction = uint64_t(0x000FFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr);
	}
	else if (sizeof(long double) == 16 && std::numeric_limits<long double>::digits <= 64) {
		_sign = fp < 0.0 ? true : false;
		_fr = frexpl(fp, &_exponent);
		_fraction = uint64_t(0x7FFFFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr); // 80bit extended format only has 63bits of fraction
	}
}
#endif

} // namespace sw::universal

#endif // MSVC 

