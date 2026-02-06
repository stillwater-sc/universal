// division.cpp: test suite runner for division on areal (arbitrary real) numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure areal environment
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_case.hpp>
#include <universal/verification/areal_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions
template<size_t nbits, size_t es, typename bt, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::universal::areal<nbits, es, bt> pa, pb, pref, pquot;
	pa = a;
	pb = b;
	ref = a / b;
	pref = ref;
	pquot = pa / pb;
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " / " << std::setw(nbits) << b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << sw::universal::to_binary(pa) << " / " << sw::universal::to_binary(pb) << " = " << sw::universal::to_binary(pquot) << " (reference: " << sw::universal::to_binary(pref) << ")   ";
	std::cout << (pref == pquot ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
// NOTE: areal uses specialized VerifyDivision that only tests exact values (ubit=0).
// Interval values (ubit=1) represent open intervals and cannot be compared against double reference.
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
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "areal division verification";
	std::string test_tag    = "areal division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	GenerateTestCase<8, 2, uint8_t, float>(1.0f, 2.0f);
	GenerateTestCase<8, 2, uint8_t, float>(1.0f, -0.5f);
	GenerateTestCase<16, 5, uint16_t, float>(1.0f, 3.0f);
	GenerateTestCase<16, 8, uint16_t, double>(INFINITY, INFINITY);

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<4, 1, uint8_t> >(reportTestCases), "areal<4,1,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<5, 2, uint8_t> >(reportTestCases), "areal<5,2,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<6, 2, uint8_t> >(reportTestCases), "areal<6,2,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<8, 2, uint8_t> >(reportTestCases), "areal<8,2,uint8_t>", "division");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// areal<nbits, es> requires nbits > es + 2 (for sign + exponent + ubit + at least 1 fraction bit)
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<4, 1, uint8_t> >(reportTestCases), "areal< 4,1>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<5, 1, uint8_t> >(reportTestCases), "areal< 5,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<5, 2, uint8_t> >(reportTestCases), "areal< 5,2>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<6, 1, uint8_t> >(reportTestCases), "areal< 6,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<6, 2, uint8_t> >(reportTestCases), "areal< 6,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<6, 3, uint8_t> >(reportTestCases), "areal< 6,3>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<7, 1, uint8_t> >(reportTestCases), "areal< 7,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<7, 2, uint8_t> >(reportTestCases), "areal< 7,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<7, 3, uint8_t> >(reportTestCases), "areal< 7,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<7, 4, uint8_t> >(reportTestCases), "areal< 7,4>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<8, 1, uint8_t> >(reportTestCases), "areal< 8,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<8, 2, uint8_t> >(reportTestCases), "areal< 8,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<8, 3, uint8_t> >(reportTestCases), "areal< 8,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<8, 4, uint8_t> >(reportTestCases), "areal< 8,4>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<8, 5, uint8_t> >(reportTestCases), "areal< 8,5>", "division");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<9, 2, uint8_t> >(reportTestCases), "areal< 9,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<9, 3, uint8_t> >(reportTestCases), "areal< 9,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<9, 4, uint8_t> >(reportTestCases), "areal< 9,4>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<9, 5, uint8_t> >(reportTestCases), "areal< 9,5>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<9, 6, uint8_t> >(reportTestCases), "areal< 9,6>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<10, 2, uint8_t> >(reportTestCases), "areal<10,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<10, 3, uint8_t> >(reportTestCases), "areal<10,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<10, 4, uint8_t> >(reportTestCases), "areal<10,4>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<10, 5, uint8_t> >(reportTestCases), "areal<10,5>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<10, 6, uint8_t> >(reportTestCases), "areal<10,6>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<10, 7, uint8_t> >(reportTestCases), "areal<10,7>", "division");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<11, 2, uint8_t> >(reportTestCases), "areal<11,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<11, 3, uint8_t> >(reportTestCases), "areal<11,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<11, 4, uint8_t> >(reportTestCases), "areal<11,4>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<11, 5, uint8_t> >(reportTestCases), "areal<11,5>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<12, 2, uint8_t> >(reportTestCases), "areal<12,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<12, 3, uint8_t> >(reportTestCases), "areal<12,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<12, 4, uint8_t> >(reportTestCases), "areal<12,4>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<12, 5, uint8_t> >(reportTestCases), "areal<12,5>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<12, 6, uint8_t> >(reportTestCases), "areal<12,6>", "division");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<13, 3, uint8_t> >(reportTestCases), "areal<13,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<13, 4, uint8_t> >(reportTestCases), "areal<13,4>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<13, 5, uint8_t> >(reportTestCases), "areal<13,5>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<13, 6, uint8_t> >(reportTestCases), "areal<13,6>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<14, 3, uint8_t> >(reportTestCases), "areal<14,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<14, 4, uint8_t> >(reportTestCases), "areal<14,4>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<14, 5, uint8_t> >(reportTestCases), "areal<14,5>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< areal<14, 6, uint8_t> >(reportTestCases), "areal<14,6>", "division");
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
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
