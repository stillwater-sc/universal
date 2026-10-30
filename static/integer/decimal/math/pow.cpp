// pow.cpp: test suite runner for the integer power of decimal positional integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <sstream>
// throw, so that the division by zero of 0^-n is observable
#define POSITIONAL_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/positional/positional.hpp>
#include <universal/verification/test_suite.hpp>

/*
   ipow(a, b) is the exact integer power, by squaring, in positional's own arithmetic (#1875):
   a^0 == 1 for every a, and a negative b gives the truncated 1 / a^|b|. ipow(a, unsigned) and
   ipow(a, positional) are the two entry points.
*/

namespace sw { namespace universal {

	// every base in [amin, amax] and exponent in [0, nmax] whose power stays below 10^18 in
	// magnitude, against a native product
	template<unsigned ndigits, unsigned radix>
	int VerifyPositionalPower(long long amin, long long amax, unsigned nmax, bool reportTestCases) {
		using Positional = positional<ndigits, radix>;
		constexpr long long bound = 1000000000000000000ll;   // 10^18
		int nrOfFailedTests = 0;
		for (long long a = amin; a <= amax; ++a) {
			long long ref = 1;
			for (unsigned n = 0; n <= nmax; ++n) {
				const Positional x(a);
				const Positional r1 = ipow(x, n);
				const Positional r2 = ipow(x, Positional(static_cast<long long>(n)));
				if (r1 != Positional(ref) || r2 != Positional(ref)) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: " << a << " ^ " << n << " = " << r1 << ", " << r2 << " reference " << ref << '\n';
				}
				if (a != 0 && (ref > bound / (a < 0 ? -a : a) || ref < -bound / (a < 0 ? -a : a))) break;
				ref *= a;
			}
		}
		return nrOfFailedTests;
	}

	// a negative exponent: 1 / a^|n| truncated, and the division by zero for 0
	template<unsigned ndigits, unsigned radix>
	int VerifyNegativeExponent(bool reportTestCases) {
		using Positional = positional<ndigits, radix>;
		int nrOfFailedTests = 0;
		for (long long a = -5; a <= 5; ++a) {
			for (long long n = -9; n <= -1; ++n) {
				const Positional x(a);
				if (a == 0) {
					bool caught = false;
					try { (void)ipow(x, Positional(n)); } catch (const positional_divide_by_zero&) { caught = true; }
					if (!caught) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: 0 ^ " << n << " did not report a division by zero\n";
					}
					continue;
				}
				long long expected = 0;
				if (a == 1) expected = 1;
				if (a == -1) expected = (n % 2 != 0) ? -1 : 1;
				const Positional r = ipow(x, Positional(n));
				if (r != Positional(expected)) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: " << a << " ^ " << n << " = " << r << " reference " << expected << '\n';
				}
			}
		}
		return nrOfFailedTests;
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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "decimal positional integer power function validation";
	std::string test_tag    = "ipow";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using Decimal = positional<40, 10>;
		std::cout << "7^40 = " << ipow(Decimal(7), 40u) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyPositionalPower<20, 10>(-30, 30, 40, reportTestCases), "positional<20,10>", "ipow");
	nrOfFailedTestCases += ReportTestResult(VerifyNegativeExponent<20, 10>(reportTestCases), "positional<20,10>", "ipow negative exponent");

	{
		// past native range: 7^40, against its decimal digits
		using Decimal = positional<40, 10>;
		const std::string ref = "6366805760909027985741435139224001";
		std::ostringstream s1, s2;
		s1 << ipow(Decimal(7), 40u);
		s2 << ipow(Decimal(7), Decimal(40));
		if (s1.str() != ref || s2.str() != ref) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 7 ^ 40 = " << s1.str() << ", " << s2.str() << " reference " << ref << '\n';
		}
	}
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyPositionalPower<20, 3>(-30, 30, 40, reportTestCases), "positional<20,3>", "ipow");
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
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught universal arithmetic exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught universal internal exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
