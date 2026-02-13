// addition.cpp: test suite runner for posit addition
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define ALGORITHM_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define ALGORITHM_VERBOSE_OUTPUT
//#define ALGORITHM_TRACE_ADD
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_randoms.hpp>

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pb, pref, psum;
	pa = a;
	pb = b;
	ref = a + b;
	pref = ref;
	psum = pa + pb;
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " + " << std::setw(nbits) << b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " + " << pb.get() << " = " << psum.get() << " (reference: " << pref.get() << ")   " ;
	std::cout << (pref == psum ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
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

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "posit addition verification";
	std::string test_tag    = "addition";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<6, 3, double>(INFINITY, INFINITY);
	GenerateTestCase<8, 4, float>(0.5f, -0.5f);
	GenerateTestCase<3, 0>(0.5f, 1.0f);

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<3, 0>>(reportTestCases), "posit<3,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<3, 1>>(reportTestCases), "posit<3,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<3, 2>>(reportTestCases), "posit<3,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<3, 3>>(reportTestCases), "posit<3,3>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 2>>(reportTestCases, OPCODE_ADD, 1000), "posit<64,2>", "addition");
	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<2, 0>>(reportTestCases), "posit< 2,0>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<3, 0>>(reportTestCases), "posit< 3,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<3, 1>>(reportTestCases), "posit< 3,1>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<4, 0>>(reportTestCases), "posit< 4,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<4, 1>>(reportTestCases), "posit< 4,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<4, 2>>(reportTestCases), "posit< 4,2>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<5, 0>>(reportTestCases), "posit< 5,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<5, 1>>(reportTestCases), "posit< 5,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<5, 2>>(reportTestCases), "posit< 5,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<5, 3>>(reportTestCases), "posit< 5,3>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<6, 0>>(reportTestCases), "posit< 6,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<6, 1>>(reportTestCases), "posit< 6,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<6, 2>>(reportTestCases), "posit< 6,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<6, 3>>(reportTestCases), "posit< 6,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<6, 4>>(reportTestCases), "posit< 6,4>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<7, 0>>(reportTestCases), "posit< 7,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<7, 1>>(reportTestCases), "posit< 7,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<7, 2>>(reportTestCases), "posit< 7,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<7, 3>>(reportTestCases), "posit< 7,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<7, 4>>(reportTestCases), "posit< 7,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<7, 5>>(reportTestCases), "posit< 7,5>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<8, 0>>(reportTestCases), "posit< 8,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<8, 1>>(reportTestCases), "posit< 8,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<8, 2>>(reportTestCases), "posit< 8,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<8, 3>>(reportTestCases), "posit< 8,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<8, 4>>(reportTestCases), "posit< 8,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<8, 5>>(reportTestCases), "posit< 8,5>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<8, 6>>(reportTestCases), "posit< 8,6>", "addition");
#endif

#if REGRESSION_LEVEL_2
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<10, 0>>(reportTestCases), "posit<10,0>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<10, 1>>(reportTestCases), "posit<10,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<10, 2>>(reportTestCases), "posit<10,2>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<10, 3>>(reportTestCases), "posit<10,3>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<16, 2>>(reportTestCases, OPCODE_ADD, 1000), "posit<16,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<24, 2>> (reportTestCases, OPCODE_ADD, 1000), "posit<24,1>", "addition");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<20, 1>> (reportTestCases, OPCODE_ADD, 1000), "posit<20,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<28, 1>> (reportTestCases, OPCODE_ADD, 1000), "posit<28,1>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<32, 1>> (reportTestCases, OPCODE_ADD, 1000), "posit<32,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<32, 2>> (reportTestCases, OPCODE_ADD, 1000), "posit<32,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<32, 3>> (reportTestCases, OPCODE_ADD, 1000), "posit<32,3>", "addition");
#endif

#if REGRESSION_LEVEL_4
	// nbits=48 also shows failures
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<48, 2>> (reportTestCases, OPCODE_ADD, 1000), "posit<48,2>", "addition");

	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 2>>(reportTestCases, OPCODE_ADD, 1000), "posit<64,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 3>>(reportTestCases, OPCODE_ADD, 1000), "posit<64,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 4>> (reportTestCases, OPCODE_ADD, 1000), "posit<64,4>", "addition");

#ifdef HARDWARE_ACCELERATION
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<12, 1>>(reportTestCases), "posit<12,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<14, 1>>(reportTestCases), "posit<14,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<posit<16, 1>>(reportTestCases), "posit<16,1>", "addition");
#endif // HARDWARE_ACCELERATION

#endif // REGRESSION_LEVEL_4

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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
