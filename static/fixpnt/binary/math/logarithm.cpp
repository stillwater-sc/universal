// logarithm.cpp: test suite runner for the logarithm functions (log2, log10, ln)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default library configuration
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/fixpnt_test_suite_mathlib.hpp>

// generate specific test case 
template<size_t nbits, size_t rbits, bool arithmetic, typename bt, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	sw::universal::fixpnt<nbits, rbits, arithmetic, bt> pa, pref, plog;
	pa = a;
	ref = std::log(a);
	pref = ref;
	plog = sw::universal::log(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> log(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << sw::universal::to_binary(pa) << " -> log( " << pa << ") = " << sw::universal::to_binary(plog) << " (reference: " << sw::universal::to_binary(pref) << ")   " ;
	std::cout << (pref == plog ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
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
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif
#define GENERATE_LOG_TABLES 0

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fixed-point mathlib logarithm ";
	std::string test_tag    = "mathlib log";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, 1, Saturate, uint8_t, float>(4.0f);

#if GENERATE_LOG_TABLES
	GenerateLogarithmTable<3, 0>();
	GenerateLogarithmTable<4, 0>();
	GenerateLogarithmTable<4, 1>();
	GenerateLogarithmTable<5, 0>();
	GenerateLogarithmTable<5, 1>();
	GenerateLogarithmTable<5, 2>();
	GenerateLogarithmTable<6, 0>();
	GenerateLogarithmTable<6, 1>();
	GenerateLogarithmTable<6, 2>();
	GenerateLogarithmTable<6, 3>();
	GenerateLogarithmTable<7, 0>();
#endif

	// manual exhaustive test
	using FixedPoint = fixpnt<10, 5, Saturate, uint8_t>;
	nrOfFailedTestCases += ReportTestResult(VerifyLog< FixedPoint >(reportTestCases), type_tag(FixedPoint()), "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2< FixedPoint >(reportTestCases), type_tag(FixedPoint()), "log2");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10< FixedPoint >(reportTestCases), type_tag(FixedPoint()), "log10");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	using FixedPoint = fixpnt<10, 5, Saturate, uint8_t>;
	nrOfFailedTestCases += ReportTestResult(VerifyLog< FixedPoint >(reportTestCases), type_tag(FixedPoint()), "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2< FixedPoint >(reportTestCases), type_tag(FixedPoint()), "log2");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10< FixedPoint >(reportTestCases), type_tag(FixedPoint()), "log10");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
	// nbits=64 requires long double compiler support

	{
		using FixedPoint = fixpnt<10, 5, Saturate, uint8_t>;
		nrOfFailedTestCases += ReportTestResult(VerifyLog< FixedPoint >(reportTestCases), type_tag(FixedPoint()), "log");
		nrOfFailedTestCases += ReportTestResult(VerifyLog2< FixedPoint >(reportTestCases), type_tag(FixedPoint()), "log2");
		nrOfFailedTestCases += ReportTestResult(VerifyLog10< FixedPoint >(reportTestCases), type_tag(FixedPoint()), "log10");
	}
	{
		using FixedPoint = fixpnt<12, 6, Saturate, uint8_t>;
		nrOfFailedTestCases += ReportTestResult(VerifyLog< FixedPoint >(reportTestCases), type_tag(FixedPoint()), "log");
	}
	{
		using FixedPoint = fixpnt<14, 7, Saturate, uint8_t>;
		nrOfFailedTestCases += ReportTestResult(VerifyLog< FixedPoint >(reportTestCases), type_tag(FixedPoint()), "log");
	}
	{
		using FixedPoint = fixpnt<16, 8, Saturate, uint8_t>;
		nrOfFailedTestCases += ReportTestResult(VerifyLog< FixedPoint >(reportTestCases), type_tag(FixedPoint()), "log");
	}
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
//catch (const sw::universal::fixpnt_quire_exception& err) {
//	std::cerr << "Uncaught fixpnt quire exception: " << err.what() << std::endl;
//	return EXIT_FAILURE;
//}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
