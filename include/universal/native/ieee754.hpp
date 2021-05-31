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

namespace sw::universal {

// IEEE-754 parameter constexpressions
template<typename Real>
class ieee754_parameter {
public:
	static constexpr int ebits        = 0; // number of exponent bits
	static constexpr int bias         = 0; // exponent bias
	static constexpr uint64_t emask   = 0; // mask of the exponent field
	static constexpr uint64_t eallset = 0; // mask of exponent value
	static constexpr int fbits        = 0; // number of fraction bits
	static constexpr uint64_t hmask   = 0; // mask of the hidden bit
	static constexpr uint64_t fmask   = 0; // mask of the fraction field
	static constexpr uint64_t hfmask  = 0; // mask of the signficicant field, i.e. hidden bit + fraction bits
	static constexpr uint64_t fmsb    = 0; // mask of the most significant fraction bit
};
template<>
class ieee754_parameter<float> {
public:
	static constexpr uint64_t smask   = 0x8000'0000ull;
	static constexpr int      ebits   = 8;
	static constexpr int      bias    = 127;
	static constexpr uint64_t emask   = 0x7F80'0000ull;
	static constexpr uint64_t eallset = 0xFFull;
	static constexpr int      fbits   = 23;
	static constexpr uint64_t hmask   = 0x0080'0000ull;
	static constexpr uint64_t fmask   = 0x007F'FFFFull;
	static constexpr uint64_t hfmask  = 0x00FF'FFFFull;
	static constexpr uint64_t fmsb    = 0x0040'0000ull;
};
template<>
class ieee754_parameter<double> {
public:
	static constexpr uint64_t smask   = 0x8000'0000'0000'0000ull;
	static constexpr int      ebits   = 11;
	static constexpr int      bias    = 1023;
	static constexpr uint64_t emask   = 0x7FF0'0000'0000'0000ull;
	static constexpr uint64_t eallset = 0x7FF;
	static constexpr int      fbits   = 52;
	static constexpr uint64_t hmask   = 0x0010'0000'0000'0000ull;
	static constexpr uint64_t fmask   = 0x000F'FFFF'FFFF'FFFFull;
	static constexpr uint64_t hfmask  = 0x001F'FFFF'FFFF'FFFFull;
	static constexpr uint64_t fmsb    = 0x0008'0000'0000'0000ull;
};
// <long double> specializations are in the compiler specific sections

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

} // namespace sw::universal

#if BIT_CAST_SUPPORT
#include <universal/native/constexpr754.hpp>
#else
#include <universal/native/nonconstexpr754.hpp>
#endif

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

