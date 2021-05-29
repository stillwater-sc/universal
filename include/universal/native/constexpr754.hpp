#pragma once
// constexpr754.hpp: constexpr manipulation functions for IEEE-754 native types using C++20 <bit>
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>
#include <iomanip>
#include <bit>    // bit_cast

#include <universal/utility/color_print.hpp>

namespace sw::universal {

template<typename Real>
class ieee754_parameters {
public:
	static constexpr int ebits = 0;
	static constexpr int bias = 0;
	static constexpr uint64_t emask = 0;
	static constexpr uint64_t eallset = 0;
	static constexpr int fbits = 0;
	static constexpr uint64_t fmask = 0;
	static constexpr uint64_t fmsb = 0;
};
template<>
class ieee754_parameters<float> {
public:
	static constexpr uint64_t smask   = 0x8000'0000ull;
	static constexpr int      ebits   = 8;
	static constexpr int      bias    = 127;
	static constexpr uint64_t emask   = 0x7F80'0000ull;
	static constexpr uint64_t eallset = 0xFFull;
	static constexpr int      fbits   = 23;
	static constexpr uint64_t fmask   = 0x007F'FFFFull;
	static constexpr uint64_t fmsb    = 0x0040'0000ull;
};
template<>
class ieee754_parameters<double> {
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

template<typename Real>
inline constexpr void extractFields(Real value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits) noexcept {
	// empty general case
}
template<>
inline constexpr void extractFields(float value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits) noexcept {
	uint64_t bc = std::bit_cast<uint32_t, float>(value);
	s = (ieee754_parameters<float>::smask & bc);
	rawExponentBits = (ieee754_parameters<float>::emask & bc) >> ieee754_parameters<float>::fbits;
	rawFractionBits = (ieee754_parameters<float>::fmask & bc);
}
inline constexpr void extractFields(double value, bool& s, uint64_t& rawExponentBits, uint64_t& rawFractionBits) noexcept {
	uint64_t bc = std::bit_cast<uint64_t, double>(value);
	s = (ieee754_parameters<double>::smask & bc);
	rawExponentBits = (ieee754_parameters<double>::emask & bc) >> ieee754_parameters<double>::fbits;
	rawFractionBits = (ieee754_parameters<double>::fmask & bc);
}

// generate a hex formatted string for a native IEEE floating point
template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
>
inline std::string to_hex(Real number) {
	std::stringstream s;
	bool sign{ false };
	uint64_t rawExponent{ 0 };
	uint64_t rawFraction{ 0 };
	extractFields(number, sign, rawExponent, rawFraction);
	s << (sign ? '1' : '0') << '.' << std::hex << int(rawExponent) << '.' << rawFraction;
	return s.str();
}

// generate a binary string for a native IEEE floating point
template<typename Real>
inline std::string to_binary(Real number, bool bNibbleMarker = false) {
	std::stringstream s;

	bool sign{ false };
	uint64_t rawExponent{ 0 };
	uint64_t rawFraction{ 0 };
	extractFields(number, sign, rawExponent, rawFraction);

	s << 'b';
	// print sign bit
	s << (sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint32_t mask = (uint32_t(1) << (ieee754_parameters<Real>::ebits-1));
		for (int i = ieee754_parameters<Real>::ebits - 1; i >= 0; --i) {
			s << ((rawExponent & mask) ? '1' : '0');
			if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
			mask >>= 1;
		}
	}

	s << '.';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << (ieee754_parameters<Real>::fbits - 1));
	for (int i = ieee754_parameters<Real>::fbits - 1; i >= 0; --i) {
		s << ((rawFraction & mask) ? '1' : '0');
		if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
		mask >>= 1;
	}

	return s.str();
}

// return in triple form (sign, scale, fraction)
template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
>
inline std::string to_triple(Real number) {
	std::stringstream s;

	bool sign{ false };
	uint64_t rawExponent{ 0 };
	uint64_t rawFraction{ 0 };
	extractFields(number, sign, rawExponent, rawFraction);

	// print sign bit
	s << '(' << (sign ? '-' : '+') << ',';

	// exponent 
	// the exponent value used in the arithmetic is the exponent shifted by a bias 
	// for the IEEE 754 binary32 case, an exponent value of 127 represents the actual zero 
	// (i.e. for 2^(e - 127) to be one, e must be 127). 
	// Exponents range from -126 to +127 because exponents of -127 (all 0s) and 128 (all 1s) are reserved for special numbers.
	if (rawExponent == 0) {
		s << "exp=0,";
	}
	else if (rawExponent == ieee754_parameters<Real>::eallset) {
		s << "exp=1, ";
	}
	int scale = static_cast<int>(rawExponent) - ieee754_parameters<Real>::bias;
	s << scale << ',';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << (ieee754_parameters<Real>::fbits - 1));
	for (int i = (ieee754_parameters<Real>::fbits - 1); i >= 0; --i) {
		s << ((rawFraction & mask) ? '1' : '0');
		mask >>= 1;
	}

	s << ')';
	return s.str();
}

template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
>
inline std::string to_base2_scientific(Real number) {
	std::stringstream s;

	bool sign{ false };
	uint64_t rawExponent{ 0 };
	uint64_t rawFraction{ 0 };
	extractFields(number, sign, rawExponent, rawFraction);

	s << (sign == 1 ? "-" : "+") << "1.";
	uint64_t mask = (uint64_t(1) << (ieee754_parameters<Real>::fbits - 1));
	for (int i = (ieee754_parameters<Real>::fbits - 1); i >= 0; --i) {
		s << ((rawFraction & mask) ? '1' : '0');
		mask >>= 1;
	}
	s << "e2^" << std::showpos << (rawExponent - ieee754_parameters<Real>::bias);

	return s.str();
}


// generate a color coded binary string for a native single precision IEEE floating point
template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
>
inline std::string color_print(Real number) {
	std::stringstream s;

	bool sign{ false };
	uint64_t rawExponent{ 0 };
	uint64_t rawFraction{ 0 };
	extractFields(number, sign, rawExponent, rawFraction);

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color blue(ColorCode::FG_BLUE);
	Color magenta(ColorCode::FG_MAGENTA);
	Color cyan(ColorCode::FG_CYAN);
	Color white(ColorCode::FG_WHITE);
	Color def(ColorCode::FG_DEFAULT);

	// print sign bit
	s << red << (sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint64_t mask = (1 << (ieee754_parameters<Real>::ebits - 1));
		for (int i = (ieee754_parameters<Real>::ebits - 1); i >= 0; --i) {
			s << cyan << ((rawExponent & mask) ? '1' : '0');
			if (i > 0 && i % 4 == 0) s << cyan << '\'';
			mask >>= 1;
		}
	}

	s << '.';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << (ieee754_parameters<Real>::fbits - 1));
	for (int i = (ieee754_parameters<Real>::fbits - 1); i >= 0; --i) {
		s << magenta << ((rawFraction & mask) ? '1' : '0');
		if (i > 0 && i % 4 == 0) s << magenta << '\'';
		mask >>= 1;
	}

	s << def;
	return s.str();
}

} // namespace sw::universal

