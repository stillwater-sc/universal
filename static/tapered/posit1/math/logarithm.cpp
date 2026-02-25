// logarithm.cpp: test suite runner for the logarithm functions (log2, log10, ln)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// when you define ALGORITHM_VERBOSE_OUTPUT the code will print intermediate results for selected arithmetic operations
//#define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_SQRT

// use default number system library configuration
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

// generate specific test case that you can trace with the trace conditions in posit.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pref, plog;
	pa = a;
	ref = std::log(a);
	pref = ref;
	plog = sw::universal::log(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> log(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " -> log( " << pa << ") = " << plog.get() << " (reference: " << pref.get() << ")   " ;
	std::cout << (pref == plog ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define GENERATE_LOG_TABLES 0
// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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

	std::string test_suite  = "posit logarithm function validation";
	std::string test_tag    = "logarithm";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, 1, float>(4.0f);

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
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<2, 0>>(reportTestCases), "posit<2,0>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<3, 0>>(reportTestCases), "posit<3,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<3, 1>>(reportTestCases), "posit<3,1>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<4, 0>>(reportTestCases), "posit<4,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<4, 1>>(reportTestCases), "posit<4,1>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<5, 0>>(reportTestCases), "posit<5,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<5, 1>>(reportTestCases), "posit<5,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<5, 2>>(reportTestCases), "posit<5,2>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<8, 4>>(reportTestCases), "posit<8,4>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2<posit<8, 4>>(reportTestCases), "posit<8,4>", "log2");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10<posit<8, 4>>(reportTestCases), "posit<8,4>", "log10");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<2, 0>>(reportTestCases), "posit<2,0>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<3, 0>>(reportTestCases), "posit<3,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<3, 1>>(reportTestCases), "posit<3,1>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<4, 0>>(reportTestCases), "posit<4,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<4, 1>>(reportTestCases), "posit<4,1>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<5, 0>>(reportTestCases), "posit<5,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<5, 1>>(reportTestCases), "posit<5,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<5, 2>>(reportTestCases), "posit<5,2>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<6, 0>>(reportTestCases), "posit<6,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<6, 1>>(reportTestCases), "posit<6,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<6, 2>>(reportTestCases), "posit<6,2>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<6, 3>>(reportTestCases), "posit<6,3>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<7, 0>>(reportTestCases), "posit<7,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<7, 1>>(reportTestCases), "posit<7,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<7, 2>>(reportTestCases), "posit<7,2>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<7, 3>>(reportTestCases), "posit<7,3>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<7, 4>>(reportTestCases), "posit<7,4>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<8, 0>>(reportTestCases), "posit<8,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<8, 1>>(reportTestCases), "posit<8,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<8, 2>>(reportTestCases), "posit<8,2>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<8, 3>>(reportTestCases), "posit<8,3>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<8, 4>>(reportTestCases), "posit<8,4>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<8, 5>>(reportTestCases), "posit<8,5>", "log");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<9, 0>>(reportTestCases), "posit<9,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<9, 1>>(reportTestCases), "posit<9,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<9, 2>>(reportTestCases), "posit<9,2>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<9, 3>>(reportTestCases), "posit<9,3>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<9, 4>>(reportTestCases), "posit<9,4>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<9, 5>>(reportTestCases), "posit<9,5>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<9, 6>>(reportTestCases), "posit<9,6>", "log");
	
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<10, 0>>(reportTestCases), "posit<10,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<10, 1>>(reportTestCases), "posit<10,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<10, 2>>(reportTestCases), "posit<10,2>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<10, 7>>(reportTestCases), "posit<10,7>", "log");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<12, 0>>(reportTestCases), "posit<12,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<12, 1>>(reportTestCases), "posit<12,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<12, 2>>(reportTestCases), "posit<12,2>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<16, 0>>(reportTestCases), "posit<16,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<16, 1>>(reportTestCases), "posit<16,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<16, 2>>(reportTestCases), "posit<16,2>", "log");
#endif

#if REGRESSION_LEVEL_4
	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 2>>(reportTestCases, OPCODE_SQRT, 1000), "posit<64,2>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 3>>(reportTestCases, OPCODE_SQRT, 1000), "posit<64,3>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 4>>(reportTestCases, OPCODE_SQRT, 1000), "posit<64,4>", "log");


	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<10, 1>>(reportTestCases), "posit<10,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<12, 1>>(reportTestCases), "posit<12,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<14, 1>>(reportTestCases), "posit<14,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<posit<16, 1>>(reportTestCases), "posit<16,1>", "log");
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
