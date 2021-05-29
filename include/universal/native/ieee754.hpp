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

#include <universal/utility/color_print.hpp>

namespace sw::universal {

////////////////////////////////////////////////////////////////////////
// numerical helpers

// return the Unit in the Last Position
template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
>
inline Real ulp(const Real& a) {
	return std::nextafter(a, a + a/2.0f) - a;
}

// check if the floating-point number is zero
template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
>
inline bool iszero(const Real& a) {
	return (std::fpclassify(a) == FP_ZERO);
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

// generate a binary string for a native single precision IEEE floating point
inline std::string to_hex(const float& number) {
	std::stringstream s;
	float_decoder decoder;
	decoder.f = number;
	s << (decoder.parts.sign ? '1' : '0') << '.' << std::hex << int(decoder.parts.exponent) << '.' << decoder.parts.fraction;
	return s.str();
}

// generate a binary string for a native single precision IEEE floating point
inline std::string to_binary(const float& number, bool bNibbleMarker = false) {
	std::stringstream s;
	float_decoder decoder;
	decoder.f = number;

	s << 'b';
	// print sign bit
	s << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint8_t mask = 0x80;
		for (int i = 7; i >= 0; --i) {
			s << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
			mask >>= 1;
		}
	}

	s << '.';

	// print fraction bits
	uint32_t mask = (uint32_t(1) << 22);
	for (int i = 22; i >= 0; --i) {
		s << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
		mask >>= 1;
	}

	return s.str();
}

// return in triple form (sign, scale, fraction)
inline std::string to_triple(const float& number) {
	std::stringstream s;
	float_decoder decoder;
	decoder.f = number;

	// print sign bit
	s << '(' << (decoder.parts.sign ? '-' : '+') << ',';

	// exponent 
	// the exponent value used in the arithmetic is the exponent shifted by a bias 
	// for the IEEE 754 binary32 case, an exponent value of 127 represents the actual zero 
	// (i.e. for 2^(e - 127) to be one, e must be 127). 
	// Exponents range from ¿126 to +127 because exponents of ¿127 (all 0s) and +128 (all 1s) are reserved for special numbers.
	if (decoder.parts.exponent == 0) {
		s << "exp=0,";
	}
	else if (decoder.parts.exponent == 0xFF) {
		s << "exp=1, ";
	}
	int scale = int(decoder.parts.exponent) - 127;
	s << scale << ',';

	// print fraction bits
	uint32_t mask = (uint32_t(1) << 22);
	for (int i = 22; i >= 0; --i) {
		s << ((decoder.parts.fraction & mask) ? '1' : '0');
		mask >>= 1;
	}

	s << ')';
	return s.str();
}

// specialization for IEEE single precision floats
inline std::string to_base2_scientific(const float& number) {
	std::stringstream s;
	float_decoder decoder;
	decoder.f = number;
	s << (decoder.parts.sign == 1 ? "-" : "+") << "1.";
	uint32_t mask = (uint32_t(1) << 22);
	for (int i = 22; i >= 0; --i) {
		s << ((decoder.parts.fraction & mask) ? '1' : '0');
		mask >>= 1;
	}
	s << "e2^" << std::showpos << (decoder.parts.exponent - 127);
/* deprecated
	bool s;
	int base2Exp;
	float _fr;
	unsigned int mantissa;
	extract_fp_components(number, s, base2Exp, _fr, mantissa);
	s << (s ? "-" : "+") << "1." << std::bitset<23>(mantissa) << "e2^" << std::showpos << base2Exp - 1;
*/
	return s.str();
}

// generate a color coded binary string for a native single precision IEEE floating point
inline std::string color_print(const float& number) {
	std::stringstream s;
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
	s << red << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint8_t mask = 0x80;
		for (int i = 7; i >= 0; --i) {
			s << cyan << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (i > 0 && i % 4 == 0) s << cyan << '\'';
			mask >>= 1;
		}
	}

	s << '.';

	// print fraction bits
	uint32_t mask = (uint32_t(1) << 22);
	for (int i = 22; i >= 0; --i) {
		s << magenta << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (i > 0 && i % 4 == 0) s << magenta << '\'';
		mask >>= 1;
	}
	
	s << def;
	return s.str();
}

// generate a color coded binary string for a native double precision IEEE floating point
inline std::string color_print(const double& number) {
	std::stringstream s;
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
	s << red << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint64_t mask = 0x800;
		for (int i = 11; i >= 0; --i) {
			s << cyan << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (i > 0 && i % 4 == 0) s << cyan << '\'';
			mask >>= 1;
		}
	}

	s << '.';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 52);
	for (int i = 52; i >= 0; --i) {
		s << magenta << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (i > 0 && i % 4 == 0) s << magenta << '\'';
		mask >>= 1;
	}

	s << def;
	return s.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// native double precision IEEE floating point

// generate a binary string for a native double precision IEEE floating point
inline std::string to_hex(const double& number) {
	std::stringstream s;
	double_decoder decoder;
	decoder.d = number;
	s << (decoder.parts.sign ? '1' : '0') << '.' << std::hex << int(decoder.parts.exponent) << '.' << decoder.parts.fraction;
	return s.str();
}

// generate a binary string for a native double precision IEEE floating point
inline std::string to_binary(const double& number, bool bNibbleMarker = false) {
	std::stringstream s;
	double_decoder decoder;
	decoder.d = number;

	s << 'b';
	// print sign bit
	s << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint64_t mask = 0x400;
		for (int i = 10; i >= 0; --i) {
			s << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
			mask >>= 1;
		}
	}

	s << '.';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 51);
	for (int i = 51; i >= 0; --i) {
		s << ((decoder.parts.fraction & mask) ? '1' : '0');
		if (bNibbleMarker && i != 0 && (i % 4) == 0) s << '\'';
		mask >>= 1;
	}

	return s.str();
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

/// <summary>
/// return the binary scale ( = 2^scale ) of a float
/// </summary>
/// <param name="v">single precision value</param>
/// <returns>binary scale</returns>
inline int scale(float v) {
	int exponent{ 0 };
	float frac = frexpf(v, &exponent);
	if (frac == 0.0f) exponent = 0;
	return exponent;
}
/// <summary>
/// return the binary scale ( = 2^scale ) of a double
/// </summary>
/// <param name="v">double precision value</param>
/// <returns>binary scale</returns>
inline int scale(double v) {
	int exponent{ 0 };
	double frac = frexp(v, &exponent);
	if (frac == 0.0) exponent = 0;
	return exponent;
}
/// <summary>
/// return the binary scale ( = 2^scale ) of a long double
/// </summary>
/// <param name="v">quad precision value</param>
/// <returns>binary scale</returns>
inline int scale(long double v) {
	int exponent{ 0 };
	long double frac = frexpl(v, &exponent);
	if (frac == 0.0l) exponent = 0;
	return exponent;
}

} // namespace sw::universal

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
#include <universal/native/ieee754_msvc.hpp>
#include <universal/native/ieee754_clang.hpp>
#include <universal/native/ieee754_gcc.hpp>
#include <universal/native/ieee754_intelicc.hpp>
#include <universal/native/ieee754_riscv.hpp>
#include <universal/native/ieee754_ibmxlc.hpp>
#include <universal/native/ieee754_hpcc.hpp>
#include <universal/native/ieee754_pgi.hpp>
#include <universal/native/ieee754_sunpro.hpp>

