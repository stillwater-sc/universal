#pragma once
// ieee-754.hpp: manipulation functions for ieee-754 native type
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>
#include <iomanip>
#include <cmath>    // for frexpf/frexp/frexpl  float/double/long double fraction/exponent extraction

namespace sw {
namespace unum {

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


union float_decoder {
  float f;
  struct {
    uint32_t fraction : 23;
    uint32_t exponent :  8;
    uint32_t sign     :  1;
  } parts;
};

union double_decoder {
  double d;
  struct {
    uint64_t fraction : 52;
    uint64_t exponent : 11;
    uint64_t  sign    :  1;
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
inline std::string to_binary(const float& number) {
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
			mask >>= 1;
		}
	}

	ss << '.';

	// print fraction bits
	uint32_t mask = (uint32_t(1) << 22);
	for (int i = 22; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
		mask >>= 1;
	}

	return ss.str();
}

// return in triple form (+, scale, fraction)
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
	// Exponents range from �126 to +127 because exponents of �127 (all 0s) and +128 (all 1s) are reserved for special numbers.
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
inline std::string to_binary(const double& number) {
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
			mask >>= 1;
		}
	}

	ss << '.';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 51);
	for (int i = 51; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
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
	// (i.e. for 2^(e � 127) to be one, e must be 127). 
	// Exponents range from �126 to +127 because exponents of �127 (all 0s) and +128 (all 1s) are reserved for special numbers.
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
	and 64 bits for the significand.The exponent field is biased by 16383,
	meaning that 16383 has to be subtracted from the value in the
	exponent field to compute the actual power of 2.
	An exponent field value of 32767 (all fifteen bits 1) is reserved
	so as to enable the representation of special states such as
	infinity and Not a Number.If the exponent field is zero, the
	value is a denormal number and the exponent of 2 is �16382.
*/
#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */
union long_double_decoder {
	long double ld;
	struct {
		uint64_t fraction : 63;
		uint64_t bit63 : 1;
		uint64_t exponent : 15;
		uint64_t  sign : 1;
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
inline std::string to_binary(const long double& number) {
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
			mask >>= 1;
		}
	}

	ss << '.';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 62);
	for (int i = 62; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
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
	// (i.e. for 2^(e � 127) to be one, e must be 127). 
	// Exponents range from �126 to +127 because exponents of �127 (all 0s) and +128 (all 1s) are reserved for special numbers.
	if (decoder.parts.exponent == 0) {
		ss << "exp=0,";
	}
	else if (decoder.parts.exponent == 0xFF) {
		ss << "exp=1, ";
	}
	int scale = int(decoder.parts.exponent) - 16383;
	ss << scale << ',';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 62);
	for (int i = 62; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
		mask >>= 1;
	}

	ss << ')';
	return ss.str();
}

#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */
// generate a binary string for a native long double precision IEEE floating point
inline std::string to_hex(const long double& number) {
	return std::string("not-implemented");
}

// generate a binary string for a native double precision IEEE floating point
inline std::string to_binary(const long double& number) {
	return std::string("not-implemented");
}

// return in triple form (+, scale, fraction)
inline std::string to_triple(const long double& number) {
	return std::string("not-implemented");
}

#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */

// long double decoder
union long_double_decoder {
	long double ld;
	struct {
		uint64_t fraction : 63;
		uint64_t bit63 : 1;
		uint64_t exponent : 15;
		uint64_t  sign : 1;
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
inline std::string to_binary(const long double& number) {
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
			mask >>= 1;
		}
	}

	ss << '.';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 62);
	for (int i = 62; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
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
	// (i.e. for 2^(e � 127) to be one, e must be 127). 
	// Exponents range from �126 to +127 because exponents of �127 (all 0s) and +128 (all 1s) are reserved for special numbers.
	if (decoder.parts.exponent == 0) {
		ss << "exp=0,";
	}
	else if (decoder.parts.exponent == 0xFF) {
		ss << "exp=1, ";
	}
	int scale = int(decoder.parts.exponent) - 16383;
	ss << scale << ',';

	// print fraction bits
	uint64_t mask = (uint64_t(1) << 62);
	for (int i = 62; i >= 0; --i) {
		ss << ((decoder.parts.fraction & mask) ? '1' : '0');
		mask >>= 1;
	}

	ss << ')';
	return ss.str();
}

#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/C++. ---------------------------------- */

// generate a binary string for a native long double precision IEEE floating point
inline std::string to_hex(const long double& number) {
	return std::string("not-implemented");
}

// generate a binary string for a native double precision IEEE floating point
inline std::string to_binary(const long double& number) {
	return std::string("not-implemented");
}

// return in triple form (+, scale, fraction)
inline std::string to_triple(const long double& number) {
	return std::string("not-implemented");
}

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */
// generate a binary string for a native long double precision IEEE floating point
inline std::string to_hex(const long double& number) {
	return std::string("not-implemented");
}

// generate a binary string for a native double precision IEEE floating point
inline std::string to_binary(const long double& number) {
	return std::string("not-implemented");
}

// return in triple form (+, scale, fraction)
inline std::string to_triple(const long double& number) {
	return std::string("not-implemented");
}

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */
// Visual C++ compiler is 15.00.20706.01, the _MSC_FULL_VER will be 15002070601

// Visual C++ does not support long double, it is just an alias for double
union long_double_decoder {
	long double ld;
	struct {
		uint64_t fraction : 52;
		uint64_t exponent : 11;
		uint64_t  sign : 1;
	} parts;
};

// generate a binary string for a native long double precision IEEE floating point
inline std::string to_hex(const long double& number) {
	return to_hex(double(number));
}

// generate a binary string for a native double precision IEEE floating point
inline std::string to_binary(const long double& number) {
	return to_binary(double(number));
}

// return in triple form (+, scale, fraction)
inline std::string to_triple(const long double& number) {
	return to_triple(double(number));
}

#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */


#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

} // namespace unum
} // namespace sw