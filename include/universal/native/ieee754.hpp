#pragma once
// ieee754.hpp: manipulation functions for IEEE-754 native types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>
#include <iomanip>
#include <cmath>    // for frexpf/frexp/frexpl  float/double/long double fraction/exponent extraction
#include <limits>
#include <tuple>

#ifdef _MSC_VER
#pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#endif
#include <universal/utility/color_print.hpp>

namespace sw::universal {

////////////////////////////////////////////////////////////////////////
// numerical helpers

// return the Unit in the Last Position
template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
>
inline Real ulp(const Real& a) {
	return std::nextafter(a, a + 1.0f) - a;
}

template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >:: type
>
inline bool isdenorm(const Real& a) {
	return (std::fpclassify(a) == FP_SUBNORMAL);
}

// IEEE double precision constants
static constexpr unsigned IEEE_FLOAT_FRACTION_BITS = 23;
static constexpr unsigned IEEE_FLOAT_EXPONENT_BITS = 8;
static constexpr unsigned IEEE_FLOAT_SIGN_BITS = 1;
// IEEE double precision constants
static constexpr unsigned IEEE_DOUBLE_FRACTION_BITS = 52;
static constexpr unsigned IEEE_DOUBLE_EXPONENT_BITS = 11;
static constexpr unsigned IEEE_DOUBLE_SIGN_BITS = 1;
// IEEE long double precision constants are compiler dependent

// TODO: completely replace this with <bit> library bit_cast<>
union float_decoder {
  float_decoder() : f{0.0f} {}
  float_decoder(float _f) : f{_f} {}
  float f;
  struct {
    uint32_t fraction : 23;
    uint32_t exponent :  8;
    uint32_t sign     :  1;
  } parts;
};

union double_decoder {
  double_decoder() : d{0.0} {}
  double_decoder(double _d) : d{_d} {}
  double d;
  struct {
    uint64_t fraction : 52;
    uint64_t exponent : 11;
    uint64_t sign     :  1;
  } parts;
};

////////////////// string operators

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// native single precision IEEE floating point

// generate a binary string for a native single precision IEEE floating point
inline std::string to_hex(const float& number) {
	std::stringstream ss;
	float_decoder decoder;
	decoder.f = number;
	ss << (decoder.parts.sign ? '1' : '0') << '.' << std::hex << int(decoder.parts.exponent) << '.' << decoder.parts.fraction;
	return ss.str();
}

// generate a binary string for a native single precision IEEE floating point
inline std::string to_binary(const float& number, bool bNibbleMarker = false) {
	std::stringstream ss;
	float_decoder decoder;
	decoder.f = number;

	// print sign bit
	ss << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint8_t mask = 0x80;
		for (int i = 7; i >= 0; --i) {
			ss << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (bNibbleMarker && i != 0 && (i % 4) == 0) ss << '\'';
			mask >>= 1;
		}
	}

	ss << '.';

	// print fraction bits
	uint32_t mask = (uint32_t(1) << 22);
	for (int i = 22; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (bNibbleMarker && i != 0 && (i % 4) == 0) ss << '\'';
		mask >>= 1;
	}

	return ss.str();
}

// return in triple form (sign, scale, fraction)
inline std::string to_triple(const float& number) {
	std::stringstream ss;
	float_decoder decoder;
	decoder.f = number;

	// print sign bit
	ss << '(' << (decoder.parts.sign ? '-' : '+') << ',';

	// exponent 
	// the exponent value used in the arithmetic is the exponent shifted by a bias 
	// for the IEEE 754 binary32 case, an exponent value of 127 represents the actual zero 
	// (i.e. for 2^(e - 127) to be one, e must be 127). 
	// Exponents range from ¿126 to +127 because exponents of ¿127 (all 0s) and +128 (all 1s) are reserved for special numbers.
	if (decoder.parts.exponent == 0) {
		ss << "exp=0,";
	}
	else if (decoder.parts.exponent == 0xFF) {
		ss << "exp=1, ";
	}
	int scale = int(decoder.parts.exponent) - 127;
	ss << scale << ',';

	// print fraction bits
	uint32_t mask = (uint32_t(1) << 22);
	for (int i = 22; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
		mask >>= 1;
	}

	ss << ')';
	return ss.str();
}

// specialization for IEEE single precision floats
inline std::string to_base2_scientific(const float& number) {
	std::stringstream ss;
	float_decoder decoder;
	decoder.f = number;
	ss << (decoder.parts.sign == 1 ? "-" : "+") << "1.";
	uint32_t mask = (uint32_t(1) << 22);
	for (int i = 22; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
		mask >>= 1;
	}
	ss << "e2^" << std::showpos << (decoder.parts.exponent - 127);
/* deprecated
	bool s;
	int base2Exp;
	float _fr;
	unsigned int mantissa;
	extract_fp_components(number, s, base2Exp, _fr, mantissa);
	ss << (s ? "-" : "+") << "1." << std::bitset<23>(mantissa) << "e2^" << std::showpos << base2Exp - 1;
*/
	return ss.str();
}

// generate a color coded binary string for a native single precision IEEE floating point
inline std::string color_print(const float& number) {
	std::stringstream ss;
	float_decoder decoder;
	decoder.f = number;

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color blue(ColorCode::FG_BLUE);
	Color magenta(ColorCode::FG_MAGENTA);
	Color cyan(ColorCode::FG_CYAN);
	Color white(ColorCode::FG_WHITE);
	Color def(ColorCode::FG_DEFAULT);

	// print sign bit
	ss << red << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint8_t mask = 0x80;
		for (int i = 7; i >= 0; --i) {
			ss << cyan << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (i > 0 && i % 4 == 0) ss << cyan << '\'';
			mask >>= 1;
		}
	}

	ss << '.';

	// print fraction bits
	uint32_t mask = (uint32_t(1) << 22);
	for (int i = 22; i >= 0; --i) {
		ss << magenta << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (i > 0 && i % 4 == 0) ss << magenta << '\'';
		mask >>= 1;
	}
	
	ss << def;
	return ss.str();
}

// generate a color coded binary string for a native double precision IEEE floating point
inline std::string color_print(const double& number) {
	std::stringstream ss;
	double_decoder decoder;
	decoder.d = number;

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color blue(ColorCode::FG_BLUE);
	Color magenta(ColorCode::FG_MAGENTA);
	Color cyan(ColorCode::FG_CYAN);
	Color white(ColorCode::FG_WHITE);
	Color def(ColorCode::FG_DEFAULT);

	// print sign bit
	ss << red << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint64_t mask = 0x800;
		for (int i = 11; i >= 0; --i) {
			ss << cyan << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (i > 0 && i % 4 == 0) ss << cyan << '\'';
			mask >>= 1;
		}
	}

	ss << '.';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 52);
	for (int i = 52; i >= 0; --i) {
		ss << magenta << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (i > 0 && i % 4 == 0) ss << magenta << '\'';
		mask >>= 1;
	}

	ss << def;
	return ss.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// native double precision IEEE floating point

// generate a binary string for a native double precision IEEE floating point
inline std::string to_hex(const double& number) {
	std::stringstream ss;
	double_decoder decoder;
	decoder.d = number;
	ss << (decoder.parts.sign ? '1' : '0') << '.' << std::hex << int(decoder.parts.exponent) << '.' << decoder.parts.fraction;
	return ss.str();
}

// generate a binary string for a native double precision IEEE floating point
inline std::string to_binary(const double& number, bool bNibbleMarker = false) {
	std::stringstream ss;
	double_decoder decoder;
	decoder.d = number;

	// print sign bit
	ss << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint64_t mask = 0x400;
		for (int i = 10; i >= 0; --i) {
			ss << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (bNibbleMarker && i != 0 && (i % 4) == 0) ss << '\'';
			mask >>= 1;
		}
	}

	ss << '.';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 51);
	for (int i = 51; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (bNibbleMarker && i != 0 && (i % 4) == 0) ss << '\'';
		mask >>= 1;
	}

	return ss.str();
}

// return in triple form (+, scale, fraction)
inline std::string to_triple(const double& number) {
	std::stringstream ss;
	double_decoder decoder;
	decoder.d = number;

	// print sign bit
	ss << '(' << (decoder.parts.sign ? '-' : '+') << ',';

	// exponent 
	// the exponent value used in the arithmetic is the exponent shifted by a bias 
	// for the IEEE 754 binary32 case, an exponent value of 127 represents the actual zero 
	// (i.e. for 2^(e - 127) to be one, e must be 127). 
	// Exponents range from -126 to +127 because exponents of -127 (all 0s) and +128 (all 1s) are reserved for special numbers.
	if (decoder.parts.exponent == 0) {
		ss << "exp=0,";
	}
	else if (decoder.parts.exponent == 0xFF) {
		ss << "exp=1, ";
	}
	int scale = int(decoder.parts.exponent) - 1023;
	ss << scale << ',';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 51);
	for (int i = 51; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
		mask >>= 1;
	}

	ss << ')';
	return ss.str();
}

// specialization for IEEE double precision floats
inline std::string to_base2_scientific(const double& number) {
	std::stringstream ss;
	double_decoder decoder;
	decoder.d = number;
	ss << (decoder.parts.sign == 1 ? "-" : "+") << "1.";
	uint64_t mask = (uint64_t(1) << 52);
	for (int i = 52; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
		mask >>= 1;
	} 
	ss << "e2^" << std::showpos << (decoder.parts.exponent - 1023);
	return ss.str();
}

/// Returns a tuple of sign, exponent, and fraction.
inline std::tuple<bool, int32_t, uint32_t> ieee_components(float fp)
{
	static_assert(std::numeric_limits<float>::is_iec559,
		"This function only works when float complies with IEC 559 (IEEE 754)");
	static_assert(sizeof(float) == 4, "This function only works when float is 32 bit.");

	float_decoder fd{ fp }; // initializes the first member of the union
	// Reading inactive union parts is forbidden in constexpr :-(
	return std::make_tuple<bool, int32_t, uint32_t>(
		static_cast<bool>(fd.parts.sign), 
		static_cast<int32_t>(fd.parts.exponent),
		static_cast<uint32_t>(fd.parts.fraction) 
	);

#if 0 // reinterpret_cast forbidden in constexpr :-(
	uint32_t& as_int = reinterpret_cast<uint32_t&>(fp);
	uint32_t exp = static_cast<int32_t>(as_int >> 23);
	if (exp & 0x80)
		exp |= 0xffffff00l; // turn on leading bits for negativ exponent
	return { fp < 0.0, exp, as_int & uint32_t{0x007FFFFFul} };
#endif
}

/// Returns a tuple of sign, exponent, and fraction.
inline std::tuple<bool, int64_t, uint64_t> ieee_components(double fp)
{
	static_assert(std::numeric_limits<double>::is_iec559,
		"This function only works when double complies with IEC 559 (IEEE 754)");
	static_assert(sizeof(double) == 8, "This function only works when double is 64 bit.");

	double_decoder dd{ fp }; // initializes the first member of the union
	// Reading inactive union parts is forbidden in constexpr :-(
	return std::make_tuple<bool, int64_t, uint64_t>(
		static_cast<bool>(dd.parts.sign), 
		static_cast<int64_t>(dd.parts.exponent),
		static_cast<uint64_t>(dd.parts.fraction) 
	);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// compiler specific long double IEEE floating point

/*
	Long double is not consistently implemented across different compilers.
	The following section organizes the implementation details of each
	of the compilers supported.

	The x86 extended precision format is an 80-bit format first
	implemented in the Intel 8087 math coprocessor and is supported
	by all processors that are based on the x86 design that incorporate
	a floating-point unit(FPU).This 80 - bit format uses one bit for
	the sign of the significand, 15 bits for the exponent field
	(i.e. the same range as the 128 - bit quadruple precision IEEE 754 format)
	and 64 bits for the significand. The exponent field is biased by 16383,
	meaning that 16383 has to be subtracted from the value in the
	exponent field to compute the actual power of 2.
	An exponent field value of 32767 (all fifteen bits 1) is reserved
	so as to enable the representation of special states such as
	infinity and Not a Number.If the exponent field is zero, the
	value is a denormal number and the exponent of 2 is ¿16382.
*/
#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
/* Clang/LLVM. ---------------------------------------------- */
/* GNU GCC/G++. --------------------------------------------- */
union long_double_decoder {
	long_double_decoder() : ld{0.0l} {}
	long_double_decoder(long double _ld) : ld{_ld} {}
	long double ld;
	struct {
		uint64_t fraction : 63;
		uint64_t bit63 : 1;
		uint64_t exponent : 15;
		uint64_t sign : 1;
	} parts;
};

// generate a binary string for a native double precision IEEE floating point
inline std::string to_hex(const long double& number) {
	std::stringstream ss;
	long_double_decoder decoder;
	decoder.ld = number;
	ss << (decoder.parts.sign ? '1' : '0') << '.' << std::hex << int(decoder.parts.exponent) << '.' << decoder.parts.fraction;
	return ss.str();
}

// generate a binary string for a native double precision IEEE floating point
inline std::string to_binary(const long double& number, bool bNibbleMarker = false) {
	std::stringstream ss;
	long_double_decoder decoder;
	decoder.ld = number;

	// print sign bit
	ss << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint64_t mask = 0x4000;
		for (int i = 14; i >= 0; --i) {
			ss << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (bNibbleMarker && i != 0 && (i % 4) == 0) ss << '\'';
			mask >>= 1;
		}
	}

	ss << '.';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 62);
	for (int i = 62; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (bNibbleMarker && i != 0 && (i % 4) == 0) ss << '\'';
		mask >>= 1;
	}

	return ss.str();
}

// return in triple form (+, scale, fraction)
inline std::string to_triple(const long double& number) {
	std::stringstream ss;
	long_double_decoder decoder;
	decoder.ld = number;

	// print sign bit
	ss << '(' << (decoder.parts.sign ? '-' : '+') << ',';

	// exponent 
	// the exponent value used in the arithmetic is the exponent shifted by a bias 
	// for the IEEE 754 binary32 case, an exponent value of 127 represents the actual zero 
	// (i.e. for 2^(e ¿ 127) to be one, e must be 127). 
	// Exponents range from ¿126 to +127 because exponents of ¿127 (all 0s) and +128 (all 1s) are reserved for special numbers.
	if (decoder.parts.exponent == 0) {
		ss << "exp=0,";
	}
	else if (decoder.parts.exponent == 0xFF) {
		ss << "exp=1, ";
	}
	int scale = int(decoder.parts.exponent) - 16383;
	ss << scale << ',';

	// print fraction bits
	ss << (decoder.parts.bit63 ? '1' : '0');
	uint64_t mask = (uint64_t(1) << 62);
	for (int i = 62; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
		mask >>= 1;
	}

	ss << ')';
	return ss.str();
}

// generate a color coded binary string for a native double precision IEEE floating point
inline std::string color_print(const long double& number) {
	std::stringstream ss;
	long_double_decoder decoder;
	decoder.ld = number;

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color blue(ColorCode::FG_BLUE);
	Color magenta(ColorCode::FG_MAGENTA);
	Color cyan(ColorCode::FG_CYAN);
	Color white(ColorCode::FG_WHITE);
	Color def(ColorCode::FG_DEFAULT);

	// print sign bit
	ss << red << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint64_t mask = 0x8000;
		for (int i = 15; i >= 0; --i) {
			ss << cyan << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (i > 0 && i % 4 == 0) ss << cyan << '\'';
			mask >>= 1;
		}
	}

	ss << '.';

	// print fraction bits
	ss << magenta << (decoder.parts.bit63 ? '1' : '0');
	uint64_t mask = (uint64_t(1) << 62);
	for (int i = 62; i >= 0; --i) {
		ss << magenta << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (i > 0 && i % 4 == 0) ss << magenta << '\'';
		mask >>= 1;
	}

	ss << def;
	return ss.str();
}

// floating point component extractions
inline void extract_fp_components(float fp, bool& _sign, int& _exponent, float& _fr, unsigned int& _fraction) {
	static_assert(sizeof(float) == 4, "This function only works when float is 32 bit.");
	_sign = fp < 0.0 ? true : false;
	_fr = frexpf(fp, &_exponent);
	_fraction = uint32_t(0x007FFFFFul) & reinterpret_cast<uint32_t&>(_fr);
}
inline void extract_fp_components(double fp, bool& _sign, int& _exponent, double& _fr, unsigned long long& _fraction) {
	static_assert(sizeof(double) == 8, "This function only works when double is 64 bit.");
	_sign = fp < 0.0 ? true : false;
	_fr = frexp(fp, &_exponent);
	_fraction = uint64_t(0x000FFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr);
}
#ifdef CPLUSPLUS_17
inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, unsigned long long& _fraction) {
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
inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, unsigned long long& _fraction) {
	static_assert(std::numeric_limits<long double>::digits <= 64, "This function only works when long double significant is <= 64 bit.");
	if (sizeof(long double) == 8) { // it is just a double
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

#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */
// generate a binary string for a native long double precision IEEE floating point
inline std::string to_hex(const long double& number) {
	return std::string("not-implemented");
}

// generate a binary string for a native double precision IEEE floating point
inline std::string to_binary(const long double& number, bool bNibbleMarker = false) {
	return std::string("not-implemented");
}

// return in triple form (+, scale, fraction)
inline std::string to_triple(const long double& number) {
	return std::string("not-implemented");
}

#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */

/*
 * In contrast to the single and double-precision formats, this format does not utilize an implicit/hidden bit. Rather, bit 63 contains the integer part of the significand and bits 62-0 hold the fractional part. Bit 63 will be 1 on all normalized numbers.
 */

// long double decoder
union long_double_decoder {
	long double ld;
	struct {
		uint64_t fraction : 63;
		uint64_t bit63 : 1;
		uint64_t exponent : 15;
		uint64_t sign : 1;
	} parts;
};

// generate a binary string for a native double precision IEEE floating point
inline std::string to_hex(const long double& number) {
	std::stringstream ss;
	long_double_decoder decoder;
	decoder.ld = number;
	ss << (decoder.parts.sign ? '1' : '0') << '.' << std::hex << int(decoder.parts.exponent) << '.' << decoder.parts.fraction;
	return ss.str();
}

// generate a binary string for a native double precision IEEE floating point
inline std::string to_binary(const long double& number, bool bNibbleMarker = false) {
	std::stringstream ss;
	long_double_decoder decoder;
	decoder.ld = number;

	// print sign bit
	ss << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint64_t mask = 0x4000;
		for (int i = 14; i >= 0; --i) {
			ss << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (bNibbleMarker && i != 0 && (i % 4) == 0) ss << '\'';
			mask >>= 1;
		}
	}

	ss << '.';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 62);
	ss << (decoder.parts.bit63 ? '1' : '0');
	for (int i = 62; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (bNibbleMarker && i != 0 && (i % 4) == 0) ss << '\'';
		mask >>= 1;
	}

	return ss.str();
}

// return in triple form (+, scale, fraction)
inline std::string to_triple(const long double& number) {
	std::stringstream ss;
	long_double_decoder decoder;
	decoder.ld = number;

	// print sign bit
	ss << '(' << (decoder.parts.sign ? '-' : '+') << ',';

	// exponent 
	// the exponent value used in the arithmetic is the exponent shifted by a bias 
	// for the IEEE 754 binary32 case, an exponent value of 127 represents the actual zero 
	// (i.e. for 2^(e ¿ 127) to be one, e must be 127). 
	// Exponents range from ¿126 to +127 because exponents of ¿127 (all 0s) and +128 (all 1s) are reserved for special numbers.
	if (decoder.parts.exponent == 0) {
		ss << "exp=0,";
	}
	else if (decoder.parts.exponent == 0xFF) {
		ss << "exp=1, ";
	}
	int scale = int(decoder.parts.exponent) - 16383;
	ss << scale << ',';

	// print fraction bits
	ss << (decoder.parts.bit63 ? '1' : '0');
	uint64_t mask = (uint64_t(1) << 62);
	for (int i = 62; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
		mask >>= 1;
	}

	ss << ')';
	return ss.str();
}

// generate a color coded binary string for a native double precision IEEE floating point
inline std::string color_print(const long double& number) {
	std::stringstream ss;
	long_double_decoder decoder;
	decoder.ld = number;

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color blue(ColorCode::FG_BLUE);
	Color magenta(ColorCode::FG_MAGENTA);
	Color cyan(ColorCode::FG_CYAN);
	Color white(ColorCode::FG_WHITE);
	Color def(ColorCode::FG_DEFAULT);

	// print sign bit
	ss << red << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint64_t mask = 0x8000;
		for (int i = 15; i >= 0; --i) {
			ss << cyan << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (i > 0 && i % 4 == 0) ss << cyan << '\'';
			mask >>= 1;
		}
	}

	ss << '.';

	// print fraction bits
	ss << magenta << (decoder.parts.bit63 ? '1' : '0');
	uint64_t mask = (uint64_t(1) << 62);
	for (int i = 62; i >= 0; --i) {
		ss << magenta << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (i > 0 && i % 4 == 0) ss << magenta << '\'';
		mask >>= 1;
	}

	ss << def;
	return ss.str();
}

// floating point component extractions
inline void extract_fp_components(float fp, bool& _sign, int& _exponent, float& _fr, unsigned int& _fraction) {
	static_assert(sizeof(float) == 4, "This function only works when float is 32 bit.");
	_sign = fp < 0.0 ? true : false;
	_fr = frexpf(fp, &_exponent);
	_fraction = uint32_t(0x007FFFFFul) & reinterpret_cast<uint32_t&>(_fr);
}
inline void extract_fp_components(double fp, bool& _sign, int& _exponent, double& _fr, unsigned long long& _fraction) {
	static_assert(sizeof(double) == 8, "This function only works when double is 64 bit.");
	_sign = fp < 0.0 ? true : false;
	_fr = frexp(fp, &_exponent);
	_fraction = uint64_t(0x000FFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr);
}
#ifdef CPLUSPLUS_17
inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, unsigned long long& _fraction) {
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
inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, unsigned long long& _fraction) {
	static_assert(std::numeric_limits<long double>::digits <= 64, "This function only works when long double significant is <= 64 bit.");
	if (sizeof(long double) == 8) { // it is just a double
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

#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/C++. ---------------------------------- */

// generate a binary string for a native long double precision IEEE floating point
inline std::string to_hex(const long double& number) {
	return std::string("to_hex() not implemented for HP compiler");
}

// generate a binary string for a native long double precision IEEE floating point
inline std::string to_binary(const long double& number, bool bNibbleMarker = false) {
	return std::string("to_binary() not implemented for HP compiler");
}

// return in triple form (+, scale, fraction)
inline std::string to_triple(const long double& number) {
	return std::string("to_triple() not implemented for HP compiler");
}

inline void extract_fp_components(float fp, bool& _sign, int& _exponent, float& _fr, uint32_t& _fraction) {
	std::cerr << "extract_fp_components not implemented for HP compiler");
}
inline void extract_fp_components(double fp, bool& _sign, int& _exponent, float& _fr, uint32_t& _fraction) {
	std::cerr << "extract_fp_components not implemented for HP compiler");
}
inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, float& _fr, uint32_t& _fraction) {
	std::cerr << "extract_fp_components not implemented for HP compiler");
}

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */
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

inline void extract_fp_components(float fp, bool& _sign, int& _exponent, float& _fr, uint32_t& _fraction) {
	std::cerr << "extract_fp_components not implemented for IBM compiler");
}
inline void extract_fp_components(double fp, bool& _sign, int& _exponent, float& _fr, uint32_t& _fraction) {
	std::cerr << "extract_fp_components not implemented for IBM compiler");
}
inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, float& _fr, uint32_t& _fraction) {
	std::cerr << "extract_fp_components not implemented for IBM compiler");
}

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */
// Visual C++ compiler is 15.00.20706.01, the _MSC_FULL_VER will be 15002070601

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
inline std::string to_hex(const long double& number) {
	return to_hex(double(number));
}

// generate a binary string for a native long double precision IEEE floating point
inline std::string to_binary(const long double& number, bool bNibbleMarker = false) {
	return to_binary(double(number), bNibbleMarker);
}

// return in triple form (+, scale, fraction)
inline std::string to_triple(const long double& number) {
	return to_triple(double(number));
}

// generate a color coded binary string for a native double precision IEEE floating point
inline std::string color_print(const long double& number) {
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

#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#elif defined(__riscv)
/* RISC-V G++ tool chain */

// floating point component extractions
inline void extract_fp_components(float fp, bool& _sign, int& _exponent, float& _fr, uint32_t& _fraction) {
	static_assert(sizeof(float) == 4, "This function only works when float is 32 bits.");
	_sign = fp < 0.0 ? true : false;
	_fr = frexpf(fp, &_exponent);
	_fraction = uint32_t(0x007FFFFFul) & reinterpret_cast<uint32_t&>(_fr);
}
inline void extract_fp_components(double fp, bool& _sign, int& _exponent, double& _fr, uint64_t& _fraction) {
	static_assert(sizeof(double) == 8, "This function only works when double is 64 bits.");
	_sign = fp < 0.0 ? true : false;
	_fr = frexp(fp, &_exponent);
	_fraction = uint64_t(0x000FFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr);
}
inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, uint64_t& _fraction) {
	// RISC-V ABI defines long double as a 128-bit quadprecision floating point
	static_assert(sizeof(double) == 16, "This function only works when long double is 128 bits.");
	_sign = fp < 0.0 ? true : false;
	_fr = frexpl(fp, &_exponent);
	_fraction = uint64_t(0x7FFFFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr);
}

#endif

/// <summary>
/// return the binary scale ( = 2^scale ) of a float
/// </summary>
/// <param name="v">single precision value</param>
/// <returns>binary scale</returns>
inline int scale(float v) {
	int exponent{ 0 };
	float f = frexpf(v, &exponent);
	if (f == 0.0f) exponent = 0;
	return exponent;
}
/// <summary>
/// return the binary scale ( = 2^scale ) of a double
/// </summary>
/// <param name="v">double precision value</param>
/// <returns>binary scale</returns>
inline int scale(double v) {
	int exponent{ 0 };
	frexp(v, &exponent); // C6031: return value ignored
	return exponent;
}
/// <summary>
/// return the binary scale ( = 2^scale ) of a long double
/// </summary>
/// <param name="v">quad precision value</param>
/// <returns>binary scale</returns>
inline int scale(long double v) {
	int exponent{ 0 };
	frexpl(v, &exponent);
	return exponent;
}

} // namespace sw::universal

