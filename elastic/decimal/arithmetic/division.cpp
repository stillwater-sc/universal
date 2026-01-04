// division.cpp: test suite runner for division on adaptive precision decimal integers
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

// configure the decimal type
#define EDECIMAL_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/edecimal/edecimal.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

		// enumerate all addition cases for a decimal integer configuration
		template<size_t nbits>
		int VerifyEdecimalDivision(bool reportTestCases) {
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
#if EDECIMAL_THROW_ARITHMETIC_EXCEPTION
					try {
						ic = ia / ib;    // <-- this will throw
						iref = i64a / i64b;
					}
					catch (const edecimal_integer_divide_by_zero& e) {
						if (ib.iszero()) {
							// correctly caught the exception
							continue;
						}
						else {
							std::cerr << "unexpected : " << e.what() << std::endl;
							nrOfFailedTests++;
						}
					}
					catch (...) {
						std::cerr << "unexpected exception" << std::endl;
						nrOfFailedTests++;
					}
#else 
					ic = ia / ib;
					iref = i64a / i64b;
#endif
					if (ic != iref) {
						if (ic.iszero() && iref.iszero()) continue;
						nrOfFailedTests++;
						if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", ia, ib, ic, iref);
					}
					else {
						//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/", ia, ib, ic, iref);
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
	sw::universal::edecimal a, b, aref, aratio;
	a = _a;
	b = _b;
	aratio = a / b;
	ref = _a / _b;
	aref = ref;
	constexpr size_t ndigits = 30;
	std::cout << std::setw(ndigits) << _a << " / " << std::setw(ndigits) << _b << " = " << std::setw(ndigits) << ref << std::endl;
	std::cout << a << " / " << b << " = " << aratio << " (reference: " << aref << ")   " ;
	std::cout << (aref == aratio ? "PASS" : "FAIL") << std::endl << std::endl;
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

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "adaptive precision decimal integer division";
	std::string test_tag    = "decimal division";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
//	bool bReportIndividualTestCases = false;

	// generate individual testcases to hand trace/debug
	GenerateTestCase(3, 2);
	GenerateTestCase(999, 9);

	nrOfFailedTestCases += ReportTestResult(VerifyEdecimalDivision<8>(reportTestCases), "decimal division nbits=8", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEdecimalDivision<3>(reportTestCases), "decimal division 2^3 test cases", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyEdecimalDivision<10>(reportTestCases), "decimal division 2^10 test cases", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyEdecimalDivision<16>(reportTestCases), "decimal division 2^16 test cases", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyEdecimalDivision<20>(reportTestCases), "decimal division 2^20 test cases", test_tag);
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
