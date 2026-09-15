// native_conversion.cpp: conversion of cfloats to float, double and long double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

/*
   Conversion to native floating-point lost values (#1513): a long double cfloat outside double's
   exponent range came back as inf or 0; a double cfloat of scale -1024 or below came back as 0,
   although double's subnormals reach 2^-1074; and every subnormal of a configuration with
   es >= 12 read as 0, in every target. The power of two was a double from ipow() outside
   (-64, 64), and 2^-e was 1 / 2^e, which is 1 / inf past 2^1023; subnormals took their scale
   from a table of doubles that is 0 for es >= 12. It is now constexpr_ldexp in the target type.

   to_native is constexpr, and std::ldexp is not before C++23: clang rejected a constant-evaluated
   double(cfloat) of 0.5. constexpr_ldexp scales by exact powers of two in a constant expression,
   and the conversions of a sample of encodings are evaluated at compile time here and compared
   with the run-time ones.

   The reference decodes an encoding's fields with integer arithmetic and std::ldexp, without any
   cfloat conversion. Only the special encodings, whose layout depends on the configuration, are
   classified with the cfloat's own isnan() and isinf(). Every encoding of the small
   configurations is checked in all four subnormal/max-exponent combinations, and for the wide
   ones every exponent field with a handful of fractions, in float, double and long double
   wherever the target holds the significand exactly, so that the conversion rounds once.
   cfloat<128,15> with subnormals is the IEEE binary128 layout and holds every long double, so a
   cfloat<128,15> holding a long double must convert back to it. Its encoding is built from frexp
   rather than with the conversion from long double, which reads a quad long double as x87 and
   gets the value wrong (#1515).
*/

namespace sw {
namespace universal {

namespace native_conversion {

// the value of a cfloat encoding of at most 64 bits: sign, exponent, fraction. Correctly rounded
// to long double, and exact wherever the significand fits one.
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
long double Decode(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& a, std::uint64_t bits) {
	constexpr unsigned      fbits = nbits - 1u - es;
	constexpr int           bias  = (1 << (es - 1)) - 1;
	constexpr std::uint64_t fmask = (std::uint64_t(1) << fbits) - 1u;
	constexpr std::uint64_t emask = (std::uint64_t(1) << es) - 1u;
	const bool              negative = ((bits >> (nbits - 1)) & 1u) != 0;
	const std::uint64_t     fraction = bits & fmask;
	const std::uint64_t     exponent = (bits >> fbits) & emask;
	long double             v;
	if (a.isnan()) {
		v = std::numeric_limits<long double>::quiet_NaN();
	}
	else if (a.isinf()) {
		v = std::numeric_limits<long double>::infinity();
	}
	else if (exponent == 0 && !hasSubnormals) {
		v = 0.0l;  // no subnormals: the zero exponent field is zero
	}
	else {
		// the significand as an integer, converted through int64_t: clang 18 miscompiles some
		// uint64_t -> long double conversions (#1509). fbits <= 62, so it fits.
		const std::uint64_t significand = (exponent == 0) ? fraction : ((std::uint64_t(1) << fbits) | fraction);
		const int           scale       = (exponent == 0) ? 1 - bias : int(exponent) - bias;  // subnormals: 0.f * 2^(1 - bias)
		v = std::ldexp(static_cast<long double>(static_cast<std::int64_t>(significand)), scale - int(fbits));
	}
	return negative ? -v : v;
}

// equal as values, in the sign of zero, or both nan
template<typename Real>
bool Same(Real a, Real b) {
	if (std::isnan(a) || std::isnan(b))
		return std::isnan(a) && std::isnan(b);
	return a == b && std::signbit(a) == std::signbit(b);
}

struct Failures {
	int  count = 0;
	bool report;
	explicit Failures(bool reportTestCases) : report(reportTestCases) {}
	template<typename C, typename Real>
	void check(const std::string& target, const C& c, Real got, Real expected) {
		if (Same(got, expected))
			return;
		if (report || count < 6)
			std::cerr << "FAIL: " << target << '(' << to_binary(c) << ") gave " << std::hexfloat << got << ", expected "
			          << expected << std::defaultfloat << '\n';
		++count;
	}
};

// the conversion of one encoding to each target that holds its significand exactly: the value is
// then rounded once and must equal the correctly rounded reference
template<typename C>
void VerifyEncoding(std::uint64_t bits, Failures& fail) {
	C c;
	c.setbits(bits);
	const long double reference = Decode(c, bits);
	if constexpr (C::fbits < unsigned(std::numeric_limits<float>::digits))
		fail.check("float", c, static_cast<float>(c), static_cast<float>(reference));
	if constexpr (C::fbits < unsigned(std::numeric_limits<double>::digits))
		fail.check("double", c, static_cast<double>(c), static_cast<double>(reference));
#if LONG_DOUBLE_SUPPORT  // cfloat has no long double conversion without it (RISC-V)
	if constexpr (C::fbits < unsigned(std::numeric_limits<long double>::digits))
		fail.check("long double", c, static_cast<long double>(c), reference);
#endif
}

// every encoding
template<typename C>
int VerifyAllEncodings(bool reportTestCases) {
	static_assert(C::nbits <= 20, "enumerates every encoding");
	Failures fail(reportTestCases);
	for (std::uint64_t bits = 0; bits < (std::uint64_t(1) << C::nbits); ++bits)
		VerifyEncoding<C>(bits, fail);
	return fail.count;
}

// every encoding in the four subnormal/max-exponent combinations
template<unsigned nbits, unsigned es, typename bt>
int VerifyAllEncodingsAllModes(bool reportTestCases) {
	return VerifyAllEncodings<cfloat<nbits, es, bt, false, false, false>>(reportTestCases) +
	       VerifyAllEncodings<cfloat<nbits, es, bt, true, false, false>>(reportTestCases) +
	       VerifyAllEncodings<cfloat<nbits, es, bt, false, true, false>>(reportTestCases) +
	       VerifyAllEncodings<cfloat<nbits, es, bt, true, true, false>>(reportTestCases);
}

// a handful of fractions: zero, the smallest, one half (0.75 in the binade of 0.5), alternating
// bits, the largest but one and the largest
template<unsigned fbits>
constexpr std::array<std::uint64_t, 6> Fractions() {
	constexpr std::uint64_t fmask = (std::uint64_t(1) << fbits) - 1u;
	return {0u, 1u, std::uint64_t(1) << (fbits - 1), 0x5555'5555'5555'5555ull & fmask, fmask - 1u, fmask};
}

// every exponent field, subnormal and max-exponent included, with the fractions above, both signs
template<typename C>
int VerifyExponentSweep(bool reportTestCases) {
	static_assert(C::nbits <= 64, "decodes a uint64_t");
	Failures fail(reportTestCases);
	for (std::uint64_t e = 0; e < (std::uint64_t(1) << C::es); ++e) {
		for (std::uint64_t f : Fractions<C::fbits>()) {
			for (std::uint64_t sign : {std::uint64_t(0), std::uint64_t(1)}) {
				VerifyEncoding<C>((sign << (C::nbits - 1)) | (e << C::fbits) | f, fail);
			}
		}
	}
	return fail.count;
}

// encodings for the compile-time check: the exponent fields at the edges of the cfloat's range and
// of float's, double's and long double's normal and subnormal ranges, with three fractions, the
// sign alternating. Kept small for the constexpr step limits (MSVC's default is 100000).
constexpr std::size_t nrEdgeEncodings = 48;

template<typename C>
constexpr std::array<std::uint64_t, nrEdgeEncodings> EdgeEncodings() {
	constexpr int           bias  = (1 << (C::es - 1)) - 1;
	constexpr int           emax  = (1 << C::es) - 1;
	constexpr std::uint64_t fmask = (std::uint64_t(1) << C::fbits) - 1u;
	constexpr int           scales[16] = {-16500, -16445, -16382, -1100, -1074, -1030, -1022, -149, -126, -64, -1, 0, 63, 128, 1024, 16384};
	std::array<std::uint64_t, nrEdgeEncodings> r{};
	std::size_t                                n = 0;
	for (int s : scales) {
		int e = s + bias;
		e = (e < 0) ? 0 : ((e > emax) ? emax : e);  // clamped into the cfloat's own range
		for (std::uint64_t f : {std::uint64_t(0), std::uint64_t(1) << (C::fbits - 1), fmask - 1u}) {
			const std::uint64_t sign = std::uint64_t(n & 1u) << (C::nbits - 1);
			r[n++]                   = sign | (std::uint64_t(e) << C::fbits) | f;
		}
	}
	return r;
}

template<typename C, typename Real>
constexpr std::array<Real, nrEdgeEncodings> ConstexprConversions() {
	std::array<Real, nrEdgeEncodings> r{};
	constexpr auto       encodings = EdgeEncodings<C>();
	for (std::size_t i = 0; i < encodings.size(); ++i) {
		C c{};
		c.setbits(encodings[i]);
		r[i] = static_cast<Real>(c);
	}
	return r;
}

// the compile-time conversions of the edge encodings equal the run-time ones, which the sweeps
// check against the reference
template<typename C, typename Real>
void VerifyConstexprTarget(Failures& fail, const std::string& target) {
	constexpr std::array<Real, nrEdgeEncodings> atCompileTime = ConstexprConversions<C, Real>();
	constexpr auto                              encodings     = EdgeEncodings<C>();
	for (std::size_t i = 0; i < encodings.size(); ++i) {
		C c;
		c.setbits(encodings[i]);
		fail.check("constexpr " + target, c, atCompileTime[i], static_cast<Real>(c));
	}
}

template<typename C>
int VerifyConstexpr(bool reportTestCases) {
	Failures fail(reportTestCases);
	VerifyConstexprTarget<C, float>(fail, "float");
	VerifyConstexprTarget<C, double>(fail, "double");
#if LONG_DOUBLE_SUPPORT
	VerifyConstexprTarget<C, long double>(fail, "long double");
#endif
	return fail.count;
}

// constexpr_ldexp in a constant expression against std::ldexp at run time: significands with
// rounding ties and near-ties at every exponent from below the smallest subnormal into the normal
// range, where the one rounding step happens, and past the largest finite value
template<typename Real>
constexpr std::array<Real, 8> LdexpSignificands() {
	constexpr Real ulp = std::numeric_limits<Real>::epsilon();
	return {Real(1), Real(1.5), Real(1.25), Real(1.75), Real(1) + ulp, Real(2) - ulp, Real(-1.5), Real(-1) - ulp};
}

template<typename Real>
constexpr int LdexpExponent(std::size_t j) {  // j < 64: the subnormal range and its edges; then overflow
	using limits = std::numeric_limits<Real>;
	return (j < 60) ? limits::min_exponent - limits::digits - 4 + int(j) * (limits::digits + 8) / 60
	                : limits::max_exponent - 3 + int(j - 60);
}

template<typename Real>
constexpr std::array<Real, 8 * 64> ConstexprLdexps() {
	std::array<Real, 8 * 64> r{};
	constexpr auto           xs = LdexpSignificands<Real>();
	for (std::size_t i = 0; i < xs.size(); ++i)
		for (std::size_t j = 0; j < 64; ++j) r[i * 64 + j] = constexpr_ldexp(xs[i], LdexpExponent<Real>(j));
	return r;
}

template<typename Real>
int VerifyConstexprLdexp(bool reportTestCases) {
	constexpr std::array<Real, 8 * 64> atCompileTime = ConstexprLdexps<Real>();
	constexpr auto                     xs            = LdexpSignificands<Real>();
	int                                fails         = 0;
	for (std::size_t i = 0; i < xs.size(); ++i) {
		for (std::size_t j = 0; j < 64; ++j) {
			const int  e        = LdexpExponent<Real>(j);
			const Real expected = std::ldexp(xs[i], e);
			if (Same(atCompileTime[i * 64 + j], expected))
				continue;
			if (reportTestCases || fails < 6)
				std::cerr << "FAIL: constexpr_ldexp(" << std::hexfloat << xs[i] << ", " << e << ") gave "
				          << atCompileTime[i * 64 + j] << ", expected " << expected << std::defaultfloat << '\n';
			++fails;
		}
	}
	return fails;
}

#if LONG_DOUBLE_SUPPORT
using Binary128 = cfloat<128, 15, std::uint32_t, true, false, false>;

// the cfloat<128,15> encoding of a long double, built bit by bit from frexp rather than with the
// conversion from long double, which reads a quad long double as x87: the low 64 bits of its 112-bit
// fraction as the top 63 (#1515). Exact: binary128 holds every x87 and every quad long double.
inline Binary128 Encode128(long double x) {
	constexpr int fbits = 112, bias = 16383;
	Binary128     c;
	c.clear();
	c.setbit(127, std::signbit(x));
	int e = 0;
	std::frexp(std::fabs(x), &e);  // |x| in [2^(e-1), 2^e)
	const int   biased = e - 1 + bias;
	long double t      = (biased >= 1) ? std::ldexp(std::fabs(x), 1 - e) - 1.0l  // the fraction of 1.f
	                                   : std::ldexp(std::fabs(x), bias - 1);     // subnormal: 0.f * 2^(1 - bias)
	const int   field  = (biased >= 1) ? biased : 0;
	for (int i = 0; i < 15; ++i)
		c.setbit(unsigned(fbits + i), ((field >> i) & 1) != 0);
	for (int i = fbits - 1; i >= 0; --i) {
		t *= 2.0l;
		const bool bit = (t >= 1.0l);
		c.setbit(unsigned(i), bit);
		if (bit)
			t -= 1.0l;
	}
	return c;
}

// cfloat<128,15> with subnormals is the IEEE binary128 layout: holding a long double, it converts
// back to exactly that long double
inline int VerifyWide(bool reportTestCases) {
	using limit = std::numeric_limits<long double>;
	Failures                 fail(reportTestCases);
	std::vector<long double> xs = {0.75l,
	                               1.0l / 3.0l,  // a full long double significand
	                               0.1l,
	                               1.0l + std::ldexp(1.0l, -60),
	                               1.0l - std::ldexp(1.0l, -60),
	                               limit::max(),
	                               limit::min(),
	                               limit::min() / 2.0l,  // a subnormal long double
	                               limit::min() * 3.0l,
	                               limit::denorm_min()};
	if (limit::max_exponent > 2000)
		xs.push_back(std::ldexp(1.0l, 2000));
	if (limit::min_exponent < -2000)
		xs.push_back(std::ldexp(1.0l, -2000));
	for (long double x : xs) {
		for (long double y : {x, -x}) {
			const Binary128 c = Encode128(y);
			fail.check("long double", c, static_cast<long double>(c), y);
		}
	}
	return fail.count;
}
#endif  // LONG_DOUBLE_SUPPORT

}  // namespace native_conversion

}  // namespace universal
}  // namespace sw

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;
	using namespace sw::universal::native_conversion;

	std::string test_suite          = "cfloat conversion to float, double and long double";
	std::string test_tag            = "native conversion";
	bool        reportTestCases     = false;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	cfloat<64, 12, std::uint32_t, true, false, false> c;
	c = std::ldexp(1.0, -1030);
	std::cout << "double(cfloat<64,12>(2^-1030)) = " << static_cast<double>(c) << '\n';
	nrOfFailedTestCases += ReportTestResult(VerifyExponentSweep<decltype(c)>(true), "cfloat<64,12>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#else

#	if REGRESSION_LEVEL_1
#		if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyWide(reportTestCases), "cfloat<128,15>", "long double");
#		endif
	// every encoding, with and without subnormals and max-exponent values
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyAllEncodingsAllModes<8, 2, std::uint8_t>(reportTestCases), "cfloat< 8,2,uint8_t >", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyAllEncodingsAllModes<12, 4, std::uint8_t>(reportTestCases), "cfloat<12,4,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAllEncodingsAllModes<16, 5, std::uint16_t>(reportTestCases),
	                                        "cfloat<16,5,uint16_t>", test_tag);
	// every exponent field: the whole float range, double's, past double's (es = 12: subnormals
	// used to read 0), and long double's; exponent fields straddling uint8_t limbs
	nrOfFailedTestCases += ReportTestResult(VerifyExponentSweep<cfloat<32, 8, std::uint32_t, true, false, false>>(reportTestCases),
	                                        "cfloat<32,8,uint32_t,tff>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentSweep<cfloat<32, 8, std::uint8_t, true, true, false>>(reportTestCases),
	                                        "cfloat<32,8,uint8_t,ttf>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentSweep<cfloat<64, 11, std::uint64_t, true, false, false>>(reportTestCases),
	                                        "cfloat<64,11,uint64_t,tff>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentSweep<cfloat<64, 12, std::uint8_t, true, false, false>>(reportTestCases),
	                                        "cfloat<64,12,uint8_t,tff>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentSweep<cfloat<64, 15, std::uint32_t, true, false, false>>(reportTestCases),
	                                        "cfloat<64,15,uint32_t,tff>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentSweep<cfloat<64, 15, std::uint32_t, false, true, false>>(reportTestCases),
	                                        "cfloat<64,15,uint32_t,ftf>", test_tag);
	// the constant-evaluated conversion, and the scaling it uses
	nrOfFailedTestCases += ReportTestResult(VerifyConstexprLdexp<float>(reportTestCases), "float", "constexpr_ldexp");
	nrOfFailedTestCases += ReportTestResult(VerifyConstexprLdexp<double>(reportTestCases), "double", "constexpr_ldexp");
#		if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyConstexprLdexp<long double>(reportTestCases), "long double", "constexpr_ldexp");
#		endif
	nrOfFailedTestCases += ReportTestResult(VerifyConstexpr<cfloat<32, 8, std::uint32_t, true, false, false>>(reportTestCases),
	                                        "cfloat<32,8,uint32_t,tff>", "constexpr");
	nrOfFailedTestCases += ReportTestResult(VerifyConstexpr<cfloat<64, 15, std::uint32_t, true, false, false>>(reportTestCases),
	                                        "cfloat<64,15,uint32_t,tff>", "constexpr");
	nrOfFailedTestCases += ReportTestResult(VerifyConstexpr<cfloat<64, 15, std::uint32_t, false, true, false>>(reportTestCases),
	                                        "cfloat<64,15,uint32_t,ftf>", "constexpr");
#	endif

#	if REGRESSION_LEVEL_2
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
} catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
} catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
