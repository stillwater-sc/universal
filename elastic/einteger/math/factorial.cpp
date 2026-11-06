// factorial.cpp: test suite runner for factorials on adaptive precision binary integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//  SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cassert>
#include <cstdint>
#include <limits>

// minimum set of include files to reflect source code dependencies
#include <universal/number/einteger/einteger.hpp>
#include <math/functions/factorial.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	template<typename BlockType>
	std::string ToDecimal(const einteger<BlockType>& v) {
		std::ostringstream s;
		s << v;
		return s.str();
	}

	// n! for every n whose factorial fits a native unsigned long long (20! < 2^64 < 21!),
	// through the recursive and the iterative factorial
	template<typename BlockType>
	int VerifyNativeFactorials(bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		int nrOfFailedTests = 0;
		unsigned long long ref = 1;
		for (int n = 0; n <= 20; ++n) {
			if (n > 1) ref *= static_cast<unsigned long long>(n);
			const EInteger fr = sw::math::function::factorial(EInteger(n));
			const EInteger fi = sw::math::function::factoriali(EInteger(n));
			if (fr != EInteger(ref) || fi != EInteger(ref)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: " << n << "! = " << fr << ", " << fi << " reference " << ref << '\n';
			}
		}
		return nrOfFailedTests;
	}

	// factorials past native range against their decimal digits
	template<typename BlockType>
	int VerifyWideFactorials(bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		const struct { int n; const char* digits; } refs[] = {
			{ 21, "51090942171709440000" },
			{ 25, "15511210043330985984000000" },
			{ 50, "30414093201713378043612608166064768844377641568960512000000000000" },
			{ 100, "93326215443944152681699238856266700490715968264381621468592963895217599993229915608941463976156518286253697920827223758251185210916864000000000000000000000000" },
		};
		int nrOfFailedTests = 0;
		for (const auto& r : refs) {
			const std::string fr = ToDecimal(sw::math::function::factorial(EInteger(r.n)));
			const std::string fi = ToDecimal(sw::math::function::factoriali(EInteger(r.n)));
			if (fr != r.digits || fi != r.digits) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: " << r.n << "! = " << fr << ", " << fi << " reference " << r.digits << '\n';
			}
		}
		return nrOfFailedTests;
	}

	// the identities a factorial table must satisfy: n! / (n-1)! == n with no remainder, n! is
	// divisible by every k <= n, and n! has exactly floor(n/5) + floor(n/25) + ... trailing zeros
	template<typename BlockType>
	int VerifyFactorialIdentities(int nmax, bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		int nrOfFailedTests = 0;
		EInteger previous(1), current(1);
		for (int n = 1; n <= nmax; ++n) {
			current = previous * EInteger(n);
			if (current / previous != EInteger(n) || current % previous != EInteger(0)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: " << n << "! / " << (n - 1) << "! = " << current / previous << '\n';
			}
			for (int k = 2; k <= n; k += (n / 8) + 1) {
				if (current % EInteger(k) != EInteger(0)) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: " << n << "! % " << k << " = " << current % EInteger(k) << '\n';
				}
			}
			int zeros = 0;
			for (int p = 5; p <= n; p *= 5) zeros += n / p;
			const std::string digits = ToDecimal(current);
			const std::size_t last = digits.find_last_not_of('0');
			if (static_cast<int>(digits.size() - 1 - last) != zeros) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: " << n << "! has " << (digits.size() - 1 - last) << " trailing zeros, expected " << zeros << '\n';
			}
			previous = current;
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

	std::string test_suite  = "einteger factorial validation";
	std::string test_tag    = "factorial";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using EInteger = einteger<std::uint32_t>;
		std::cout << "30! = " << sw::math::function::factoriali(EInteger(30)) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNativeFactorials<std::uint8_t>(reportTestCases), "einteger<uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeFactorials<std::uint32_t>(reportTestCases), "einteger<uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWideFactorials<std::uint8_t>(reportTestCases), "einteger<uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWideFactorials<std::uint16_t>(reportTestCases), "einteger<uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWideFactorials<std::uint32_t>(reportTestCases), "einteger<uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFactorialIdentities<std::uint32_t>(100, reportTestCases), "einteger<uint32_t>", "factorial identities");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyFactorialIdentities<std::uint8_t>(100, reportTestCases), "einteger<uint8_t>", "factorial identities");
	nrOfFailedTestCases += ReportTestResult(VerifyFactorialIdentities<std::uint32_t>(300, reportTestCases), "einteger<uint32_t>", "factorial identities");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyFactorialIdentities<std::uint16_t>(500, reportTestCases), "einteger<uint16_t>", "factorial identities");
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
catch (const sw::universal::einteger_arithmetic_exception& err) {
	std::cerr << "Uncaught einteger arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::einteger_internal_exception& err) {
	std::cerr << "Uncaught einteger internal exception: " << err.what() << std::endl;
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
