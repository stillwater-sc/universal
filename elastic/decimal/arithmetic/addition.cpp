// addition.cpp: test suite runner for addition on adaptive precision decimal integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>

// minimum set of include files to reflect source code dependencies
#include <universal/number/edecimal/edecimal.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

		// enumerate all addition cases for a decimal integer configuration
		template<size_t nbits>
		int VerifyEdecimalAddition(bool reportTestCases) {
			using Integer = edecimal;
			constexpr size_t NR_ENCODINGS = (size_t(1) << nbits);
			constexpr size_t signBitMask = (1ull << (nbits - 1));
			constexpr size_t valueBitMask = ~signBitMask;

			Integer ia, ib, ic, iref;

			int nrOfFailedTests = 0;
			size_t increment = std::max(1ull, NR_ENCODINGS / 1024ull);
			for (size_t i = 0; i < NR_ENCODINGS; i += increment) {
				if (signBitMask & i) {
					ia = (i & valueBitMask);
					ia.setsign(true);
				}
				else {
					ia = i;
					ia.setsign(false);
				}
				int64_t i64a = int64_t(ia);
				for (size_t j = 0; j < NR_ENCODINGS; j += increment) {
					if (signBitMask & j) {
						ib = (j & valueBitMask);
						ib.setsign(true);
					}
					else {
						ib = j;
						ib.setsign(false);
					}
					int64_t i64b = int64_t(ib);
					iref = i64a + i64b;
					ic = ia + ib;

					if (ic != iref) {
						if (ic.iszero() && iref.iszero()) continue;
						nrOfFailedTests++;
						if (reportTestCases) ReportBinaryArithmeticError("FAIL", "+", ia, ib, ic, iref);
					}
					else {
						//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", ia, ib, ic, iref);
					}
					if (nrOfFailedTests > 100) return nrOfFailedTests;
				}
				if (reportTestCases) if (i % 1024 == 0) std::cout << '.';
			}
			if (reportTestCases) std::cout << std::endl;
			return nrOfFailedTests;
		}
}} // namespace sw::universal

// generate specific test case that you can trace with the trace conditions in mpreal.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::universal::edecimal a, b, aref, asum;
	a = _a;
	b = _b;
	asum = a + b;
	ref = _a + _b;
	aref = ref;
	constexpr size_t ndigits = 30;
	std::cout << std::setw(ndigits) << _a << " + " << std::setw(ndigits) << _b << " = " << std::setw(ndigits) << ref << std::endl;
	std::cout << a << " + " << b << " = " << asum << " (reference: " << aref << ")   " ;
	std::cout << (aref == asum ? "PASS" : "FAIL") << std::endl << std::endl;
}

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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::string test_suite  = "adaptive precision decimal integer addition";
	std::string test_tag    = "decimal addition";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
//	bool bReportIndividualTestCases = false;

	// generate individual testcases to hand trace/debug
	GenerateTestCase(1, 2);
	GenerateTestCase(1, 9);

	nrOfFailedTestCases += VerifyDecimalAddition<8>(reportTestCases);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEdecimalAddition<10>(reportTestCases), "decimal addition nbits=10", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyEdecimalAddition<16>(reportTestCases), "decimal addition nbits=16", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyEdecimalAddition<32>(reportTestCases), "decimal addition nbits=32", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyEdecimalAddition<63>(reportTestCases), "decimal addition nbits=63", test_tag);
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
