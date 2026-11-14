// integer_edges.cpp: integers into posits wider than 64 bits, and posits into int/long/long long (#2070)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Integer construction and assignment for nbits > 64 went through double, so posit<128,2>(2^60 + 1)
// was 2^60. operator int() and operator long() went through double as well: inexact past 53 bits,
// and undefined behaviour for a value out of the integer's range or NaR. Integers now enter exactly
// and round once; the casts truncate toward zero, exactly, saturate, and give the type's minimum
// for NaR.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	int report(bool ok, const char* what, bool reportTestCases) {
		if (!ok && reportTestCases) std::cerr << "FAIL " << what << '\n';
		return ok ? 0 : 1;
	}

	int VerifyWideIntegerConstruction(bool reportTestCases) {
		int fails = 0;
		using P = posit<128, 2>;
		constexpr long long p60 = 1ll << 60;
		fails += report(double(P(p60 + 1) - P(p60)) == 1.0, "posit<128,2>(2^60 + 1) - 2^60 == 1", reportTestCases);
		fails += report(double(P(std::numeric_limits<std::uint64_t>::max()) - P(std::numeric_limits<std::uint64_t>::max() - 1u)) == 1.0,
		                "posit<128,2>(UINT64_MAX) - posit<128,2>(UINT64_MAX - 1) == 1", reportTestCases);
		fails += report(double(P(-(p60 + 1)) + P(p60)) == -1.0, "posit<128,2>(-(2^60 + 1)) + 2^60 == -1", reportTestCases);
		P m;
		m = std::numeric_limits<long long>::min();   // the assignment path, and -2^63, whose magnitude is not a long long
		fails += report(m == P(-std::ldexp(1.0, 63)), "posit<128,2> = INT64_MIN is -2^63", reportTestCases);
		P u;
		u = std::uint64_t(12345678901234567891ull);
		fails += report(static_cast<long long>(u) == std::numeric_limits<long long>::max(), "posit<128,2> = 12345678901234567891 saturates long long", reportTestCases);
		fails += report(double(u - P(12345678901234567000ull)) == 891.0, "posit<128,2> = 12345678901234567891 is exact", reportTestCases);
		// posit<72,2> has 67 fraction bits at 1 but fewer near 2^63: one rounding, nearest even
		using Q = posit<72, 2>;
		const Q q(std::numeric_limits<std::uint64_t>::max());
		fails += report(q == Q(std::ldexp(1.0, 64)), "posit<72,2>(UINT64_MAX) rounds to 2^64", reportTestCases);
		return fails;
	}

	int VerifyIntegerCasts(bool reportTestCases) {
		int fails = 0;
		using P = posit<128, 2>;
		constexpr long long p60 = 1ll << 60;
		const P a(p60 + 1);
		fails += report(static_cast<long>(a) == static_cast<long>(p60 + 1), "(long)posit<128,2>(2^60 + 1) is exact", reportTestCases);
		fails += report(static_cast<long long>(a) == p60 + 1, "(long long)posit<128,2>(2^60 + 1) is exact", reportTestCases);
		fails += report(static_cast<long long>(-a) == -(p60 + 1), "(long long)posit<128,2>(-(2^60 + 1)) is exact", reportTestCases);
		using S = posit<32, 2>;
		fails += report(static_cast<int>(S(1e30)) == std::numeric_limits<int>::max(), "(int)posit<32,2>(1e30) saturates to INT_MAX", reportTestCases);
		fails += report(static_cast<int>(S(-1e30)) == std::numeric_limits<int>::min(), "(int)posit<32,2>(-1e30) saturates to INT_MIN", reportTestCases);
		fails += report(static_cast<long long>(S(SpecificValue::maxpos)) == std::numeric_limits<long long>::max(), "(long long)maxpos saturates", reportTestCases);
		S nar;
		nar.setnar();
		fails += report(static_cast<int>(nar) == std::numeric_limits<int>::min(), "(int)NaR is INT_MIN", reportTestCases);
		fails += report(static_cast<long long>(nar) == std::numeric_limits<long long>::min(), "(long long)NaR is LLONG_MIN", reportTestCases);
		fails += report(static_cast<int>(S(-2.75)) == -2 && static_cast<int>(S(2.75)) == 2, "(int) truncates toward zero", reportTestCases);
		fails += report(static_cast<int>(S(SpecificValue::minpos)) == 0 && static_cast<int>(S(0)) == 0, "(int) of minpos and 0 is 0", reportTestCases);
		fails += report(static_cast<int>(S(-std::ldexp(1.0, 31))) == std::numeric_limits<int>::min(), "(int)-2^31 is INT_MIN", reportTestCases);
		return fails;
	}

	// every encoding: the cast is trunc of the value, clamped to the integer's range
	template<unsigned nbits, unsigned es, typename Integer>
	int VerifyCastExhaustive(bool reportTestCases) {
		int fails = 0;
		for (std::uint64_t i = 0; i < (std::uint64_t(1) << nbits); ++i) {
			posit<nbits, es> p;
			p.setbits(i);
			Integer expected;
			if (p.isnar()) expected = std::numeric_limits<Integer>::min();
			else {
				const double t = std::trunc(double(p));
				if (t >= std::ldexp(1.0, std::numeric_limits<Integer>::digits)) expected = std::numeric_limits<Integer>::max();
				else if (t < -std::ldexp(1.0, std::numeric_limits<Integer>::digits)) expected = std::numeric_limits<Integer>::min();
				else expected = static_cast<Integer>(t);
			}
			const Integer got = static_cast<Integer>(p);
			if (got != expected) {
				++fails;
				if (reportTestCases && fails < 10) std::cerr << "FAIL (" << typeid(Integer).name() << ")" << p << " = " << got << ", expected " << expected << '\n';
			}
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

	std::string test_suite  = "posit integer conversion edges (#2070)";
	std::string test_tag    = "integer edges";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyWideIntegerConstruction(reportTestCases), test_tag, "integer -> posit<128,2>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyWideIntegerConstruction(reportTestCases), test_tag, "integer -> posit > 64 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCasts(reportTestCases), test_tag, "posit -> int/long/long long");
	nrOfFailedTestCases += ReportTestResult((VerifyCastExhaustive<12, 2, int>(reportTestCases)), test_tag, "(int)posit<12,2>");
	nrOfFailedTestCases += ReportTestResult((VerifyCastExhaustive<10, 3, int>(reportTestCases)), test_tag, "(int)posit<10,3>");
	nrOfFailedTestCases += ReportTestResult((VerifyCastExhaustive<12, 4, long long>(reportTestCases)), test_tag, "(long long)posit<12,4>");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyCastExhaustive<16, 2, int>(reportTestCases)), test_tag, "(int)posit<16,2>");
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
