// zero_and_sign.cpp: zero, sign and scale of cancelled and leading-zero-block elreal values (#2003)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// A cancelled result is a stream of zero blocks, not the empty stream, and before #2003
	// iszero() tested for the empty stream only. a - a was not zero, 1/(a-a) was 0, and
	// log(a-a) searched the zero blocks for a leading digit without end.
	template<typename Real>
	int VerifyCancelledZero(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		auto fail = [&](const std::string& what, double got, const char* expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: " << what << " = " << got << ", expected " << expected << '\n';
		};
		const Real a(1.5);
		const Real z = a - a;
		if (!z.iszero()) fail("(1.5 - 1.5).iszero()", 0, "true");
		if (z.sign() != 1 || z.isneg()) fail("(1.5 - 1.5).sign()", z.sign(), "+1");
		if (z.scale() != 0) fail("(1.5 - 1.5).scale()", double(z.scale()), "0");
		if (!(z == 0.0)) fail("(1.5 - 1.5) == 0", double(z), "true");

		const Real q = Real(1.0) / z;
		if (!q.isinf() || q.sign() != 1) fail("1 / (1.5 - 1.5)", double(q), "+inf");
		if (!(z / z).isnan()) fail("(a - a) / (a - a)", double(z / z), "nan");
		if (!(Real(std::numeric_limits<double>::infinity()) * z).isnan()) fail("inf * (a - a)", 0, "nan");
		if (!(z * Real(3.0)).iszero()) fail("(a - a) * 3", double(z * Real(3.0)), "0");

		// a product that cancels, not only a sum
		const Real p = a * Real(2.0) - Real(3.0);
		if (!p.iszero()) fail("1.5 * 2 - 3", double(p), "0");

		// math functions see the zero: log and sin of a cancelled zero used to hang
		const auto t0 = std::chrono::steady_clock::now();
		const double logz = double(log(z)), log0 = double(log(Real(0.0)));
		const double sinz = double(sin(z));
		const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
		if (!(logz == log0 || (std::isnan(logz) && std::isnan(log0)))) fail("log(a - a) vs log(0)", logz, "the same");
		if (sinz != 0.0) fail("sin(a - a)", sinz, "0");
		if (seconds > 5.0) fail("seconds for log and sin of a cancelled zero", seconds, "well under 5");
		return nrOfFailedTestCases;
	}

	// 1 - (1 + 2^-100): the leading block of the difference is a zero block, and sign, scale,
	// abs, signbit and significand used to read that block
	template<typename Real>
	int VerifyLeadingZeroBlock(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		auto fail = [&](const std::string& what, double got, const char* expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: " << what << " = " << got << ", expected " << expected << '\n';
		};
		const Real one(1.0);
		const Real x = one - (one + Real(std::ldexp(1.0, -100)));   // -2^-100
		if (x.iszero()) fail("x.iszero()", 1, "false");
		if (x.sign() != -1 || !x.isneg()) fail("x.sign()", x.sign(), "-1");
		if (!signbit(x)) fail("signbit(x)", 0, "true");
		if (x.scale() != -100) fail("x.scale()", double(x.scale()), "-100");
		if (significand(x) != -1.0) fail("significand(x)", significand(x), "-1");
		const Real ax = abs(x);
		if (ax.sign() != 1 || double(ax) != std::ldexp(1.0, -100)) fail("abs(x)", double(ax), "2^-100");
		if (!(x < 0.0) || x >= 0.0) fail("x < 0", double(x), "true");
		const Real nx = -x;
		if (nx.sign() != 1 || nx.scale() != -100) fail("(-x).sign()", nx.sign(), "+1");
		return nrOfFailedTestCases;
	}

	// elreal has one, unsigned zero, however it was formed
	template<typename Real>
	int VerifyUnsignedZero(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		auto fail = [&](const std::string& what, double got, const char* expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: " << what << " = " << got << ", expected " << expected << '\n';
		};
		const Real a(2.0);
		const Real nz = -(a - a);
		if (!nz.iszero() || nz.isneg() || nz.sign() != 1) fail("-(a - a)", nz.sign(), "an unsigned zero");
		const Real mz(-0.0);
		if (!mz.iszero() || mz.isneg()) fail("elreal(-0.0)", mz.sign(), "an unsigned zero");
		const Real q = Real(1.0) / mz;
		if (!q.isinf() || q.sign() != 1) fail("1 / elreal(-0.0)", double(q), "+inf");
		return nrOfFailedTestCases;
	}

	// integers are exact at every width; they used to go through a double
	template<typename Real>
	int VerifyIntegerConstruction(bool reportTestCases, const std::string& host) {
		int nrOfFailedTestCases = 0;
		auto check = [&](const Real& got, const Real& expected, const std::string& what) {
			if (got != expected) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << host << ' ' << what << " = " << double(got) << '\n';
			}
		};
		const long long p53 = 1ll << 53;
		check(Real(p53 + 1) - Real(p53), Real(1), "elreal(2^53 + 1) - 2^53");
		check(Real(-(p53 + 1)) + Real(p53), Real(-1), "elreal(-(2^53 + 1)) + 2^53");
		check(Real(16777217) - Real(16777216), Real(1), "elreal(2^24 + 1) - 2^24");
		check(Real(std::numeric_limits<long long>::min()), Real(-std::ldexp(1.0, 63)), "elreal(INT64_MIN)");
		check(Real(std::numeric_limits<unsigned long long>::max()) - Real(std::ldexp(1.0, 64)), Real(-1), "elreal(UINT64_MAX) - 2^64");
		check(Real(0), Real(0.0), "elreal(0)");
		check(Real(-7), Real(-7.0), "elreal(-7)");
		Real r;
		r = p53 + 3;
		check(r - Real(p53), Real(3), "operator=(2^53 + 3) - 2^53");
		return nrOfFailedTestCases;
	}

	// comparison with a double on either side, and the comparison window: 1/3 < 1/7 is decided
	// by the first block of the difference, and used to force all precision() + 1 of them
	template<typename Real>
	int VerifyMixedComparison(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		auto expect = [&](bool ok, const char* what) {
			if (!ok) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << what << '\n';
			}
		};
		const Real one(1.0), two(2.0);
		expect(one <= 1.0 && one >= 1.0 && !(one < 1.0) && !(one > 1.0), "elreal vs double, equal");
		expect(1.0 <= one && 1.0 >= one && 1.0 == one && !(1.0 != one), "double vs elreal, equal");
		expect(1.0 < two && two > 1.0 && 3.0 > two && 3.0 >= two && !(3.0 <= two), "double vs elreal, ordered");
		expect(Real(1.0) / Real(3.0) > Real(1.0) / Real(7.0), "1/3 > 1/7");
		expect(Real(1.0) / Real(3.0) == Real(1.0) / Real(3.0), "1/3 == 1/3");
		return nrOfFailedTestCases;
	}

}}  // namespace sw::universal

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

	std::string test_suite  = "elreal zero, sign and scale validation";
	std::string test_tag    = "zero and sign";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	elreal<double> a(1.5), z = a - a;
	std::cout << "iszero(a - a) = " << z.iszero() << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyCancelledZero<elreal<double>>(reportTestCases), "elreal<double>", "cancelled zero");
	nrOfFailedTestCases += ReportTestResult(VerifyLeadingZeroBlock<elreal<double>>(reportTestCases), "elreal<double>", "leading zero block");
	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedZero<elreal<double>>(reportTestCases), "elreal<double>", "unsigned zero");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConstruction<elreal<double>>(reportTestCases, "elreal<double>"), "elreal<double>", "integer construction");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConstruction<elreal<float>>(reportTestCases, "elreal<float>"), "elreal<float>", "integer construction");
	nrOfFailedTestCases += ReportTestResult(VerifyMixedComparison<elreal<double>>(reportTestCases), "elreal<double>", "mixed comparison");
	nrOfFailedTestCases += ReportTestResult(VerifyCancelledZero<elreal<float>>(reportTestCases), "elreal<float>", "cancelled zero");
#endif

#if REGRESSION_LEVEL_2
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
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
