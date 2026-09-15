// native_conversion.cpp: conversion of areals to float, double and long double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite.hpp>

/*
   Conversion to long double gave wrong values (#1509): on clang, long double(areal<32,8>(0.75))
   was -nan and long double(areal<32,8>(1.0/3)) twice the value; on every compiler a long double
   areal outside double's exponent range came back as inf or 0, and so did every subnormal of a
   configuration with es >= 12. The power of two was built from 1ull << e, which clang 18
   miscompiles in 1 / (long double)(1ull << n); as a double outside (-64, 64); and for
   subnormals from a table of doubles that holds 0 for es >= 12. It is now std::ldexp in the
   target type.

   The reference decodes an encoding's fields with integer arithmetic and std::ldexp, without any
   areal conversion. Every encoding of the small configurations is checked, and for the wide ones
   every exponent field with a handful of fractions, in float, double and long double wherever the
   target holds the significand exactly, so that the conversion rounds once. areal<128,15> holds
   every long double of a 64-bit significand, so long double -> areal<128,15> -> long double must
   give the value back.
*/

namespace sw {
namespace universal {

namespace native_conversion {

// the value of an areal encoding of at most 64 bits, from its fields: sign, exponent, fraction,
// ubit. The ubit does not change the value, which is the lower end of the interval, except in the
// reserved encodings, all-ones exponent and fraction, that are inf (ubit clear) and nan (ubit set).
// Correctly rounded to long double, and exact wherever the significand fits one.
template<typename A>
long double Decode(std::uint64_t bits) {
	constexpr unsigned      nbits = A::nbits, es = A::es, fbits = A::fbits;
	constexpr int           bias  = (1 << (es - 1)) - 1;
	constexpr std::uint64_t fmask = (std::uint64_t(1) << fbits) - 1u;
	constexpr std::uint64_t emask = (std::uint64_t(1) << es) - 1u;
	const bool              negative = ((bits >> (nbits - 1)) & 1u) != 0;
	const std::uint64_t     fraction = (bits >> 1) & fmask;
	const std::uint64_t     exponent = (bits >> (1 + fbits)) & emask;
	long double             v;
	if (exponent == emask && fraction == fmask) {
		v = (bits & 1u) ? std::numeric_limits<long double>::quiet_NaN() : std::numeric_limits<long double>::infinity();
	}
	else {
		// the significand as an integer, converted through int64_t: clang 18 miscompiles the
		// uint64_t -> long double conversion this test is about (#1509). fbits <= 61, so it fits.
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
	template<typename A, typename Real>
	void check(const std::string& target, const A& a, Real got, Real expected) {
		if (Same(got, expected))
			return;
		if (report || count < 6)
			std::cerr << "FAIL: " << target << '(' << to_binary(a) << ") gave " << std::hexfloat << got << ", expected "
			          << expected << std::defaultfloat << '\n';
		++count;
	}
};

// the conversion of one encoding to each target that holds its significand exactly: the value is
// then rounded once, by std::ldexp, and must equal the correctly rounded reference
template<typename A>
void VerifyEncoding(std::uint64_t bits, Failures& fail) {
	A a;
	a.setbits(bits);
	const long double reference = Decode<A>(bits);
	if constexpr (A::fbits < unsigned(std::numeric_limits<float>::digits))
		fail.check("float", a, static_cast<float>(a), static_cast<float>(reference));
	if constexpr (A::fbits < unsigned(std::numeric_limits<double>::digits))
		fail.check("double", a, static_cast<double>(a), static_cast<double>(reference));
	if constexpr (A::fbits < unsigned(std::numeric_limits<long double>::digits))
		fail.check("long double", a, static_cast<long double>(a), reference);
}

// every encoding
template<typename A>
int VerifyAllEncodings(bool reportTestCases) {
	static_assert(A::nbits <= 20, "enumerates every encoding");
	Failures fail(reportTestCases);
	for (std::uint64_t bits = 0; bits < (std::uint64_t(1) << A::nbits); ++bits)
		VerifyEncoding<A>(bits, fail);
	return fail.count;
}

// every exponent field, subnormal and reserved included, with a handful of fractions, both signs,
// ubit clear and set
template<typename A>
int VerifyExponentSweep(bool reportTestCases) {
	static_assert(A::nbits <= 64, "decodes a uint64_t");
	constexpr unsigned      fbits = A::fbits;
	constexpr std::uint64_t fmask = (std::uint64_t(1) << fbits) - 1u;
	Failures                fail(reportTestCases);
	const std::uint64_t     fractions[] = {0u,
	                                       1u,
	                                       std::uint64_t(1) << (fbits - 1),  // 0.5: 0.75 in the binade of 0.5
	                                       0x5555'5555'5555'5555ull & fmask,
	                                       fmask - 1u,
	                                       fmask};
	for (std::uint64_t e = 0; e < (std::uint64_t(1) << A::es); ++e) {
		for (std::uint64_t f : fractions) {
			for (std::uint64_t sign : {std::uint64_t(0), std::uint64_t(1)}) {
				for (std::uint64_t ubit : {std::uint64_t(0), std::uint64_t(1)}) {
					const std::uint64_t bits = (sign << (A::nbits - 1)) | (e << (1 + fbits)) | (f << 1) | ubit;
					VerifyEncoding<A>(bits, fail);
				}
			}
		}
	}
	return fail.count;
}

// the values #1509 reported: an areal<32,8> converts to long double as it does to double, which
// holds its value exactly
inline int VerifyReported(bool reportTestCases) {
	using A = areal<32, 8, std::uint32_t>;
	Failures fail(reportTestCases);
	for (double x : {0.75, 1.0 / 3.0, 0.1, 1.0e30, 1.0e-30, std::ldexp(1.0, -130), std::ldexp(3.0, -149)}) {
		for (double y : {x, -x}) {
			const A a(y);
			fail.check("long double", a, static_cast<long double>(a), static_cast<long double>(static_cast<double>(a)));
		}
	}
	return fail.count;
}

// long double -> areal<128,15> -> long double gives the value back for every long double of a
// significand of up to 112 bits: areal<128,15> has a 111-bit fraction and one more binade than a
// long double, and its subnormals reach below a long double's. The conversion to the areal is exact
// and is checked on its encoding in saturation.cpp.
inline int VerifyRoundTrip(bool reportTestCases) {
	using X     = areal<128, 15, std::uint32_t>;
	using limit = std::numeric_limits<long double>;
	Failures                 fail(reportTestCases);
	const long double        third = std::ldexp(static_cast<long double>(0x5555'5555'5555'5555ll), -64);  // 63 bits
	std::vector<long double> xs    = {0.75l,
	                                  third,
	                                  1.0l + std::ldexp(1.0l, -60),
	                                  1.0l - std::ldexp(1.0l, -60),
	                                  limit::min(),
	                                  limit::denorm_min(),
	                                  limit::min() / 2.0l,  // a subnormal long double, an areal subnormal on x87 and quad
	                                  limit::min() * 3.0l};
	if (limit::digits <= 112) {  // a full long double significand: not a quad's 113 bits
		xs.push_back(0.1l);
		xs.push_back(limit::max());
	}
	if (limit::max_exponent > 2000)
		xs.push_back(std::ldexp(1.0l, 2000));
	if (limit::min_exponent < -2000)
		xs.push_back(std::ldexp(1.0l, -2000));
	for (long double x : xs) {
		for (long double y : {x, -x}) {
			X a;
			a = y;
			fail.check("long double", a, static_cast<long double>(a), y);
		}
	}
	return fail.count;
}

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

	std::string test_suite          = "areal conversion to float, double and long double";
	std::string test_tag            = "native conversion";
	bool        reportTestCases     = false;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	areal<32, 8, std::uint32_t> a(0.75);
	std::cout << "long double(areal<32,8>(0.75)) = " << static_cast<long double>(a) << '\n';
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip(true), "areal<128,15>", "round trip");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyReported(reportTestCases), "areal<32,8>", "reported");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip(reportTestCases), "areal<128,15>", "round trip");
	// every encoding
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyAllEncodings<areal<8, 1, std::uint8_t>>(reportTestCases), "areal< 8,1,uint8_t >", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyAllEncodings<areal<8, 2, std::uint8_t>>(reportTestCases), "areal< 8,2,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAllEncodings<areal<12, 4, std::uint16_t>>(reportTestCases),
	                                        "areal<12,4,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAllEncodings<areal<16, 5, std::uint16_t>>(reportTestCases),
	                                        "areal<16,5,uint16_t>", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyAllEncodings<areal<16, 5, std::uint8_t>>(reportTestCases), "areal<16,5,uint8_t >", test_tag);
	// every exponent field: the whole float range, double's, past double's (es = 12: subnormals
	// used to read 0), and long double's; exponent fields straddling uint8_t limbs
	nrOfFailedTestCases += ReportTestResult(VerifyExponentSweep<areal<32, 8, std::uint32_t>>(reportTestCases),
	                                        "areal<32,8,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentSweep<areal<32, 8, std::uint8_t>>(reportTestCases),
	                                        "areal<32,8,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentSweep<areal<64, 11, std::uint64_t>>(reportTestCases),
	                                        "areal<64,11,uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentSweep<areal<64, 12, std::uint8_t>>(reportTestCases),
	                                        "areal<64,12,uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExponentSweep<areal<64, 15, std::uint32_t>>(reportTestCases),
	                                        "areal<64,15,uint32_t>", test_tag);
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
