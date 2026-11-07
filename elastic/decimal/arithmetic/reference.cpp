// reference.cpp: test suite runner checking edecimal arithmetic against native 64-bit integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/edecimal/edecimal.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// r must hold the value ref, with no negative zero and no high-order zero digits. A -0 or a
	// padded zero compares unequal to edecimal(0), which is how -6 % 3, -edecimal(0) and 0 << 3 failed (#1989).
	inline bool matches(const edecimal& r, long long ref) {
		if (r.iszero() && r.isneg()) return false;
		if (r.size() > 1 && r.back() == 0) return false;
		return r == edecimal(ref);
	}

	// + - * / % and unary minus on every pair of operands in [-range, range]
	inline int VerifyBinaryOperators(long long range, bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		auto check = [&](const edecimal& r, long long ref, long long a, const char* op, long long b) {
			if (!matches(r, ref)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << a << ' ' << op << ' ' << b << " = " << r << (r.isneg() ? " (negative)" : "") << ", expected " << ref << '\n';
			}
		};
		for (long long a = -range; a <= range; ++a) {
			const edecimal da(a);
			check(-da, -a, 0, "-", a);
			for (long long b = -range; b <= range; ++b) {
				const edecimal db(b);
				check(da + db, a + b, a, "+", b);
				check(da - db, a - b, a, "-", b);
				check(da * db, a * b, a, "*", b);
				if (b == 0) continue;
				check(da / db, a / b, a, "/", b);
				check(da % db, a % b, a, "%", b);
			}
		}
		return nrOfFailedTestCases;
	}

	// << and >> scale by powers of ten; >> truncates toward zero like native division
	inline int VerifyShifts(long long range, bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		for (long long a = -range; a <= range; ++a) {
			long long scale = 1;
			for (int k = 0; k <= 4; ++k, scale *= 10) {
				const edecimal da(a);
				const edecimal l = da << k, r = da >> k;
				if (!matches(l, a * scale) || !matches(r, a / scale)) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: " << a << " << " << k << " = " << l << ", " << a << " >> " << k << " = " << r << '\n';
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// division whose quotient has many digits, some of them zero, against native division
	inline int VerifyLongDivision(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const long long dividends[] = { 1000000000000LL, 987654321987654321LL, -900000000090000LL, 4611686018427387904LL };
		const long long divisors[]  = { 1, 3, 7, 10, 99, 101, -1000, 123456789LL, -4294967296LL };
		for (long long a : dividends) {
			for (long long b : divisors) {
				const edecimal q = edecimal(a) / edecimal(b), r = edecimal(a) % edecimal(b);
				if (!matches(q, a / b) || !matches(r, a % b)) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: " << a << " / " << b << " = " << q << " rem " << r << '\n';
				}
			}
		}
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

	std::string test_suite  = "edecimal arithmetic against native integers";
	std::string test_tag    = "reference";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	edecimal a(-6), b(3);
	std::cout << a << " % " << b << " = " << (a % b) << " : equals 0 : " << ((a % b) == edecimal(0)) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperators(24, reportTestCases), "edecimal", "+ - * / % and negate");
	nrOfFailedTestCases += ReportTestResult(VerifyShifts(120, reportTestCases), "edecimal", "<< and >>");
	nrOfFailedTestCases += ReportTestResult(VerifyLongDivision(reportTestCases), "edecimal", "long division");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperators(128, reportTestCases), "edecimal", "+ - * / % and negate");
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
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
