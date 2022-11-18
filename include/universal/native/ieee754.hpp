#pragma once
// ieee754.hpp: manipulation functions for IEEE-754 native types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>
#include <iomanip>
#include <cmath>    // for frexpf/frexp/frexpl  float/double/long double fraction/exponent extraction
#include <limits>
#include <tuple>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/native/integers.hpp>
#include <universal/native/manipulators.hpp>
#include <universal/native/bit_functions.hpp>

namespace sw { namespace universal {

	// IEEE-754 parameter constexpressions
	template<typename Real>
	class ieee754_parameter {
	public:
		static constexpr int      nbits           = 0; // number of bits total
		static constexpr uint64_t smask           = 0; // mask of the sign field
		static constexpr int      ebits           = 0; // number of exponent bits
		static constexpr int      bias            = 0; // exponent bias
		static constexpr uint64_t emask           = 0; // mask of the exponent field
		static constexpr uint64_t eallset         = 0; // mask of exponent value
		static constexpr int      fbits           = 0; // number of fraction bits
		static constexpr uint64_t hmask           = 0; // mask of the hidden bit
		static constexpr uint64_t fmask           = 0; // mask of the fraction field
		static constexpr uint64_t hfmask          = 0; // mask of the signficicant field, i.e. hidden bit + fraction bits
		static constexpr uint64_t fmsb            = 0; // mask of the most significant fraction bit
		static constexpr uint64_t qnanmask        = 0; // mask of quiet NaN
		static constexpr uint64_t snanmask        = 0; // mask of signalling NaN
		static constexpr Real     minNormal       = Real(0.0); // value of smallest normal value
		static constexpr Real     minSubnormal    = Real(0.0); // value of the smallest subnormal value
		static constexpr int      minNormalExp    = 0;   // exponent value of smallest normal value
		static constexpr int      minSubnormalExp = 0;   // exponent value of smallest subnormal value
	};

	// IEEE-754 parameter specializations are in the compiler specific sections

}} // namespace sw::universal

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

namespace sw { namespace universal {


	template<typename Real>
	std::ostream& operator<<(std::ostream& ostr, const ieee754_parameter<Real>& v) {
		ostr << "Total number of bits        : " << v.nbits << '\n';
		ostr << "number of exponent bits     : " << v.ebits << '\n';              // number of exponent bits
		ostr << "number of fraction bits     : " << v.fbits << '\n';              // number of fraction bits		
		ostr << "exponent bias               : " << v.bias << '\n';               // exponent bias
		ostr << "sign field mask             : " << to_binary(v.smask, v.nbits, true) << '\n';   // mask of the sign field
		ostr << "exponent field mask         : " << to_binary(v.emask, v.nbits, true) << '\n';   // mask of the exponent field
		ostr << "mask of exponent value      : " << to_binary(v.eallset, v.ebits, true) << '\n'; // mask of exponent value

		ostr << "mask of hidden bit          : " << to_binary(v.hmask, v.nbits, true) << '\n';    // mask of the hidden bit
		ostr << "fraction field mask         : " << to_binary(v.fmask, v.nbits, true) << '\n';    // mask of the fraction field
		ostr << "significant field mask      : " << to_binary(v.hfmask, v.nbits, true) << '\n';   // mask of the signficicant field, i.e. hidden bit + fraction bits
		ostr << "MSB fraction bit mask       : " << to_binary(v.fmsb, v.nbits, true) << '\n';     // mask of the most significant fraction bit
		ostr << "qNaN pattern                : " << to_binary(v.qnanmask, v.nbits, true) << '\n'; // mask of quiet NaN
		ostr << "sNaN pattern                : " << to_binary(v.snanmask, v.nbits, true) << '\n'; // mask of signalling NaN
		ostr << "smallest normal value       : " << v.minNormal << '\n';           // value of smallest normal value
		ostr << "                            : " << to_binary(v.minNormal) << '\n';
		ostr << "smallest subnormal value    : " << v.minSubnormal << '\n';        // value of the smallest subnormal value
		ostr << "                            : " << to_binary(v.minSubnormal) << '\n';
		ostr << "exponent smallest normal    : " << v.minNormalExp << '\n';      // exponent value of smallest    normal value
		ostr << "exponent smallest subnormal : " << v.minSubnormalExp << '\n';   // exponent value of smallest subnormal value
		return ostr;
	}

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

// compile time power of 2
template<typename Real, size_t powerOfTwo,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
>
inline constexpr Real ipow() {
	Real base = 2.0f;
	Real result = 1.0f;
	size_t exp = powerOfTwo;
	for (;;) {
		if (exp & 1) result *= base;
		exp >>= 1;
		if (!exp) break;
		base *= base;
	}
	return result;
}

// fast power of 2 with positive exponent
template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
>
inline constexpr Real ipow(size_t exp) {
	Real base = 2.0f;
	Real result = 1.0f;
	for (;;) {
		if (exp & 1) result *= base;
		exp >>= 1;
		if (!exp) break;
		base *= base;
	}
	return result;
}

#ifdef DEPRECATED
/// <summary>
/// return the binary scale ( = 2^scale ) of a float
/// </summary>
/// <param name="v">single precision value</param>
/// <returns>binary scale</returns>
inline int scale(float v) {
	int exponent{ 0 };
	float frac = frexpf(v, &exponent);
	if (frac == 0.0f) exponent = 0;
	return exponent - 1;
}
/// <summary>
/// return the binary scale ( = 2^scale ) of a double
/// </summary>
/// <param name="v">double precision value</param>
/// <returns>binary scale</returns>
inline int scale(double v) {
	int exponent{ 0 };
	double frac = std::frexp(v, &exponent);
	if (frac == 0.0) exponent = 0;
	return exponent - 1;
}

#if LONG_DOUBLE_SUPPORT
/// <summary>
/// return the binary scale ( = 2^scale ) of a long double
/// </summary>
/// <param name="v">quad precision value</param>
/// <returns>binary scale</returns>
inline int scale(long double v) {
	int exponent{ 0 };
	long double frac = frexpl(v, &exponent);
	if (frac == 0.0l) exponent = 0;
	return exponent - 1;
}
#endif
#endif // DEPRECATED

template<typename DestinationType, typename SourceType>
DestinationType BitCast(SourceType source) {
	static_assert(sizeof(DestinationType) == sizeof(SourceType),
			"source and destination type sizes do not match");
	DestinationType dest;
	memmove(&dest, &source, sizeof(DestinationType));
	return dest;
}

// internal function to extract exponent
template<typename Uint, typename Real>
int _extractExponent(Real v) {
	static_assert(sizeof(Real) == sizeof(Uint), "mismatched sizes");
	Uint raw{BitCast<Uint>(v)};
	raw &= static_cast<Uint>(~ieee754_parameter<Real>::smask);
	Uint frac{ raw };
	raw >>= ieee754_parameter<Real>::fbits;
	// debias
	int e = static_cast<int>(raw) - static_cast<int>(ieee754_parameter<Real>::bias);
	if (raw == 0) { // a subnormal encoding
		int msb = findMostSignificantBit(frac);
		e -= (static_cast<int>(ieee754_parameter<Real>::fbits) - msb);
	}
	return e;
}

template<typename Real,
	 typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type
>
int scale(Real v) {
	int _e{0};
	if constexpr (sizeof(Real) == 2) { // half precision floating-point
		_e = _extractExponent<std::uint16_t>(v);
	}
	if constexpr (sizeof(Real) == 4) { // single precision floating-point
		_e = _extractExponent<std::uint32_t>(v);
	}
	else if constexpr (sizeof(Real) == 8) { // double precision floating-point
		_e = _extractExponent<std::uint64_t>(v);
	}
	else if constexpr (sizeof(Real) == 16) { // long double precision floating-point
		long double frac = frexpl(v, &_e);
		_e -= 1;
	}
	return _e;
}

// internal function to extract significant
template<typename Uint, typename Real>
Uint _extractSignificant(Real v) {
	static_assert(sizeof(Real) == sizeof(Uint), "mismatched sizes");
	Uint raw{ BitCast<Uint>(v) };
	raw &= ieee754_parameter<Real>::fmask;
	raw |= ieee754_parameter<Real>::hmask; // add the hidden bit
	return raw;
}

template<typename Real,
	     typename = typename std::enable_if<std::is_floating_point<Real>::value, Real>::type
>
std::uint64_t significant(Real v) {
	std::uint64_t _f{ 0 };
	if constexpr (sizeof(Real) == 2) { // half precision floating-point
		_f = _extractSignificant<std::uint16_t>(v);
	}
	if constexpr (sizeof(Real) == 4) { // single precision floating-point
		_f = _extractSignificant<std::uint32_t>(v);
	}
	else if constexpr (sizeof(Real) == 8) { // double precision floating-point
		_f = _extractSignificant<std::uint64_t>(v);
	}
	else if constexpr (sizeof(Real) == 16) { // long double precision floating-point
		_f = 0;
	}
	return _f;
}

// print representations of an IEEE-754 floating-point
template<typename Real>
void valueRepresentations(Real value) {
	using namespace sw::universal;
	std::cout << "IEEE-754 type : " << type_tag<Real>() << '\n';
	std::cout << "hex    : " << to_hex(value) << '\n';
	std::cout << "binary : " << to_binary(value) << '\n';
	std::cout << "triple : " << to_triple(value) << '\n';
	std::cout << "base2  : " << to_base2_scientific(value) << '\n';
	std::cout << "base10 : " << value << '\n';
	std::cout << "color  : " << color_print(value) << '\n';
}

}} // namespace sw::universal
