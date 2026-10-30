// pow.cpp: test suite runner for the integer power of adaptive precision binary integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <sstream>
// throw, so that the division by zero of 0^-n is observable
#define EINTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_suite.hpp>

/*
   ipow(a, b) is the exact integer power, by squaring (#1875). einteger grows, so the power never
   overflows; a^0 == 1 for every a, and a negative b gives the truncated 1 / a^|b|.
*/

namespace sw { namespace universal {

	// every base in [amin, amax] and every exponent in [0, nmax] whose power stays below 10^18
	// in magnitude, against a native product, through ipow(einteger, einteger) and ipow(einteger, long long)
	template<typename BlockType>
	int VerifyNativePower(long long amin, long long amax, unsigned nmax, bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		constexpr long long bound = 1000000000000000000ll;   // 10^18
		int nrOfFailedTests = 0;
		for (long long a = amin; a <= amax; ++a) {
			long long ref = 1;
			for (unsigned n = 0; n <= nmax; ++n) {
				const EInteger x(a);
				const EInteger r1 = ipow(x, EInteger(static_cast<long long>(n)));
				const EInteger r2 = ipow(x, static_cast<long long>(n));
				if (r1 != EInteger(ref) || r2 != EInteger(ref)) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: " << a << " ^ " << n << " = " << r1 << ", " << r2 << " reference " << ref << '\n';
				}
				if (a != 0 && (ref > bound / (a < 0 ? -a : a) || ref < -bound / (a < 0 ? -a : a))) break;
				ref *= a;
			}
		}
		return nrOfFailedTests;
	}

	// past native range: powers of two against a shift, and odd powers against repeated multiplication
	template<typename BlockType>
	int VerifyWidePower(unsigned nmax, bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		int nrOfFailedTests = 0;
		for (unsigned n = 0; n <= nmax; ++n) {
			EInteger ref(1);
			ref <<= static_cast<int>(n);
			if (ipow(EInteger(2), static_cast<long long>(n)) != ref) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 2 ^ " << n << " != " << ref << '\n';
			}
		}
		for (long long a : { 3ll, -3ll, 7ll, 10ll, -11ll, 12345ll }) {
			EInteger ref(1);
			for (unsigned n = 0; n <= nmax / 4; ++n) {
				if (ipow(EInteger(a), EInteger(static_cast<long long>(n))) != ref) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: " << a << " ^ " << n << " != " << ref << '\n';
				}
				ref *= EInteger(a);
			}
		}
		return nrOfFailedTests;
	}

	// a negative exponent: 1 / a^|n| truncated, and the division by zero for 0
	template<typename BlockType>
	int VerifyNegativeExponent(bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		int nrOfFailedTests = 0;
		for (long long a = -5; a <= 5; ++a) {
			for (long long n = -9; n <= -1; ++n) {
				const EInteger x(a);
				if (a == 0) {
					int caught = 0;
					try { (void)ipow(x, EInteger(n)); } catch (const einteger_divide_by_zero&) { ++caught; }
					try { (void)ipow(x, n); } catch (const einteger_divide_by_zero&) { ++caught; }
					if (caught != 2) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: 0 ^ " << n << " did not report a division by zero\n";
					}
					continue;
				}
				long long expected = 0;
				if (a == 1) expected = 1;
				if (a == -1) expected = (n % 2 != 0) ? -1 : 1;
				const EInteger r1 = ipow(x, EInteger(n));
				const EInteger r2 = ipow(x, n);
				if (r1 != EInteger(expected) || r2 != EInteger(expected)) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: " << a << " ^ " << n << " = " << r1 << ", " << r2 << " reference " << expected << '\n';
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

	std::string test_suite  = "einteger power function validation";
	std::string test_tag    = "ipow";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using EInteger = einteger<std::uint32_t>;
		std::cout << "100^100 = " << ipow(EInteger(100), 100ll) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNativePower<std::uint32_t>(-30, 30, 40, reportTestCases), "einteger<uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNegativeExponent<std::uint32_t>(reportTestCases), "einteger<uint32_t>", "ipow negative exponent");
	nrOfFailedTestCases += ReportTestResult(VerifyWidePower<std::uint32_t>(400, reportTestCases), "einteger<uint32_t>", test_tag);

	{
		// x *= x squares: it used to give 0, because operator*= cleared *this, which is also rhs
		using EInteger = einteger<std::uint32_t>;
		int start = nrOfFailedTestCases;
		for (long long v : { 7ll, -7ll, 65536ll, 123456789ll }) {
			EInteger x(v);
			x *= x;
			if (x != EInteger(v * v)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << v << " *= itself = " << x << " reference " << v * v << '\n';
			}
		}
		ReportTestResult(nrOfFailedTestCases - start, "einteger<uint32_t>", "x *= x");
	}

	{
		// 7^40 against its decimal digits
		using EInteger = einteger<std::uint32_t>;
		const std::string ref = "6366805760909027985741435139224001";
		std::ostringstream s;
		s << ipow(EInteger(7), 40ll);
		if (s.str() != ref) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 7 ^ 40 = " << s.str() << " reference " << ref << '\n';
		}
	}
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyNativePower<std::uint8_t>(-30, 30, 40, reportTestCases), "einteger<uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWidePower<std::uint8_t>(400, reportTestCases), "einteger<uint8_t>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyWidePower<std::uint32_t>(4000, reportTestCases), "einteger<uint32_t>", test_tag);
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
catch (const sw::universal::einteger_arithmetic_exception& err) {
	std::cerr << "Uncaught einteger arithmetic exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::einteger_internal_exception& err) {
	std::cerr << "Uncaught einteger internal exception: " << err.what() << '\n';
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
