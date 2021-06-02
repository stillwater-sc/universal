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
	static constexpr int nbits         = 0; // number of bits total
	static constexpr uint64_t smask    = 0; // mask of the sign field
	static constexpr int ebits         = 0; // number of exponent bits
	static constexpr int bias          = 0; // exponent bias
	static constexpr uint64_t emask    = 0; // mask of the exponent field
	static constexpr uint64_t eallset  = 0; // mask of exponent value
	static constexpr int fbits         = 0; // number of fraction bits
	static constexpr uint64_t hmask    = 0; // mask of the hidden bit
	static constexpr uint64_t fmask    = 0; // mask of the fraction field
	static constexpr uint64_t hfmask   = 0; // mask of the signficicant field, i.e. hidden bit + fraction bits
	static constexpr uint64_t fmsb     = 0; // mask of the most significant fraction bit
	static constexpr uint64_t qnanmask = 0; // mask of quiet NaN
	static constexpr uint64_t snanmask = 0; // mask of signalling NaN
};

// IEEE-754 paramter specializations are in the compiler specific sections

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

// compiler specializations for IEEE-754 parameters
#include <universal/native/ieee754_msvc.hpp>
#include <universal/native/ieee754_clang.hpp>
#include <universal/native/ieee754_gcc.hpp>
#include <universal/native/ieee754_intelicc.hpp>
#include <universal/native/ieee754_riscv.hpp>
#include <universal/native/ieee754_ibmxlc.hpp>
#include <universal/native/ieee754_hpcc.hpp>
#include <universal/native/ieee754_pgi.hpp>
#include <universal/native/ieee754_sunpro.hpp>

#if BIT_CAST_SUPPORT
#include <universal/native/constexpr754.hpp>
#else
#include <universal/native/nonconstexpr754.hpp>
#endif

