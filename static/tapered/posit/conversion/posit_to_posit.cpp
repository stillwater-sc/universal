// posit_to_posit.cpp: conversion between posit configurations, rounded once on the encoding (#2070)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The converting constructor posit<n,e,b>(posit<nn,ee,bb>) went through double. That lost every bit
// past 53, rounded twice, and turned values outside double's range into NaR (above) or 0 (below),
// where a posit saturates. It now decodes the source on its encoding and rounds once.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <string>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// every encoding of Source into Target, against Target(double(source)): exact while both
	// configurations sit inside double's precision and range, which these do
	template<typename Source, typename Target>
	int VerifyExhaustiveThroughDouble(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		constexpr unsigned N = Source::nbits;
		for (std::uint64_t i = 0; i < (std::uint64_t(1) << N); ++i) {
			Source a;
			a.setbits(i);
			const Target got(a);
			Target expected;
			if (a.isnar()) expected.setnar();
			else expected = double(a);
			if (got != expected) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases < 10) {
					std::cerr << "FAIL " << type_tag(a) << ' ' << to_binary(a) << " -> " << type_tag(got) << ' ' << to_binary(got)
					          << ", expected " << to_binary(expected) << '\n';
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// Narrow -> Wide -> Narrow is the identity: every Narrow value is a Wide value
	template<typename Narrow, typename Wide>
	int VerifyWidenNarrowIdentity(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		constexpr unsigned N = Narrow::nbits;
		for (std::uint64_t i = 0; i < (std::uint64_t(1) << N); ++i) {
			Narrow a;
			a.setbits(i);
			const Wide w(a);
			const Narrow back(w);
			if (back != a) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases < 10) std::cerr << "FAIL round trip " << to_binary(a) << " -> " << to_binary(back) << '\n';
			}
		}
		return nrOfFailedTestCases;
	}

	int report(bool ok, const char* what, bool reportTestCases) {
		if (!ok && reportTestCases) std::cerr << "FAIL " << what << '\n';
		return ok ? 0 : 1;
	}

	// the cases the QA sweep found (#2047, #2070): each failed through double
	int VerifySweepCases(bool reportTestCases) {
		int fails = 0;
		{   // a 128-bit value through blocks of another size and back: double kept 53 of its 125 bits
			using P8 = posit<128, 2, std::uint8_t>;
			using P32 = posit<128, 2, std::uint32_t>;
			P8 a(1.0);
			a += P8(std::ldexp(1.0, -100));
			const P32 b(a);
			const P8 c(b);
			fails += report(c == a && c != P8(1.0), "posit<128,2,u8> -> <128,2,u32> -> back is the identity", reportTestCases);
		}
		{   // one rounding: 1 + 2^-13 + 2^-58 is above the tie between 1 and 1 + 2^-12 in posit<16,1>;
			// double kept 1 + 2^-13, the tie, which rounded to even, 1
			posit<64, 2> a(1.0);
			a += posit<64, 2>(std::ldexp(1.0, -13));
			a += posit<64, 2>(std::ldexp(1.0, -58));
			const posit<16, 1> b(a);
			fails += report(double(b) == 1.0 + std::ldexp(1.0, -12), "posit<64,2>(1 + 2^-13 + 2^-58) -> posit<16,1> rounds up", reportTestCases);
		}
		{   // maxpos 2^1920 of posit<32,6> is past double: it was NaR
			const posit<32, 6> m(SpecificValue::maxpos);
			const posit<32, 6, std::uint16_t> b(m);
			fails += report(b == posit<32, 6, std::uint16_t>(SpecificValue::maxpos), "posit<32,6> maxpos -> posit<32,6,u16> is maxpos", reportTestCases);
		}
		{   // past the target's range: saturates to maxpos, and maxneg to maxneg
			const posit<64, 7> m(SpecificValue::maxpos), n(SpecificValue::maxneg);
			fails += report(posit<64, 2>(m) == posit<64, 2>(SpecificValue::maxpos), "posit<64,7> maxpos -> posit<64,2> is maxpos", reportTestCases);
			fails += report(posit<64, 2>(n) == posit<64, 2>(SpecificValue::maxneg), "posit<64,7> maxneg -> posit<64,2> is maxneg", reportTestCases);
			const posit<64, 7> tiny(SpecificValue::minpos);
			fails += report(posit<64, 2>(tiny) == posit<64, 2>(SpecificValue::minpos), "posit<64,7> minpos -> posit<64,2> is minpos", reportTestCases);
		}
		{   // posit<64,2> minpos is 2^-248, a posit<64,7> value: it was 0
			const posit<64, 2> m(SpecificValue::minpos);
			const posit<64, 7> b(m);
			fails += report(b == posit<64, 7>(std::ldexp(1.0, -248)), "posit<64,2> minpos -> posit<64,7> is 2^-248", reportTestCases);
			fails += report(posit<64, 7>(-m) == posit<64, 7>(-std::ldexp(1.0, -248)), "posit<64,2> -minpos -> posit<64,7> is -2^-248", reportTestCases);
		}
		{   // zero and NaR
			posit<8, 0> nar;
			nar.setnar();
			fails += report(posit<32, 2>(nar).isnar() && posit<32, 2>(posit<8, 0>(0)).iszero(), "NaR and 0 convert to NaR and 0", reportTestCases);
		}
		return fails;
	}

}} // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "posit to posit conversion (#2070)";
	std::string test_tag    = "posit<> -> posit<>";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifySweepCases(reportTestCases), test_tag, "sweep cases");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySweepCases(reportTestCases), test_tag, "sweep cases");
	// narrowing, with one rounding to compare against
	nrOfFailedTestCases += ReportTestResult((VerifyExhaustiveThroughDouble<posit<10, 1>, posit<6, 0>>(reportTestCases)), test_tag, "posit<10,1> -> posit<6,0>");
	nrOfFailedTestCases += ReportTestResult((VerifyExhaustiveThroughDouble<posit<12, 2>, posit<8, 1>>(reportTestCases)), test_tag, "posit<12,2> -> posit<8,1>");
	nrOfFailedTestCases += ReportTestResult((VerifyExhaustiveThroughDouble<posit<12, 0>, posit<9, 3>>(reportTestCases)), test_tag, "posit<12,0> -> posit<9,3>");
	// widening, and back
	nrOfFailedTestCases += ReportTestResult((VerifyWidenNarrowIdentity<posit<8, 0>, posit<16, 1>>(reportTestCases)), test_tag, "posit<8,0> -> posit<16,1> -> back");
	nrOfFailedTestCases += ReportTestResult((VerifyWidenNarrowIdentity<posit<10, 2>, posit<72, 2, std::uint32_t>>(reportTestCases)), test_tag, "posit<10,2> -> posit<72,2,u32> -> back");
	nrOfFailedTestCases += ReportTestResult((VerifyWidenNarrowIdentity<posit<12, 1, std::uint16_t>, posit<12, 1, std::uint8_t>>(reportTestCases)), test_tag, "posit<12,1,u16> -> posit<12,1,u8> -> back");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyExhaustiveThroughDouble<posit<16, 1>, posit<10, 2>>(reportTestCases)), test_tag, "posit<16,1> -> posit<10,2>");
	nrOfFailedTestCases += ReportTestResult((VerifyWidenNarrowIdentity<posit<16, 2>, posit<128, 4>>(reportTestCases)), test_tag, "posit<16,2> -> posit<128,4> -> back");
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
