// reciprocation.cpp: test suite runner for posit arithmetic reciprocation
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
// when you define ALGORITHM_VERBOSE_OUTPUT executing an reciprocate the code will print intermediate results
//#define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_RECIPROCAL
#define ALGORITHM_TRACE_CONVERSION
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in posit.hpp
// Most bugs are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a) {
	Ty reference;
	sw::universal::posit<nbits, es> pa, pref, preciprocal;
	pa = a;
	reference = (Ty)1.0 / a;
	pref = reference;
	preciprocal = pa.reciprocate();
	std::cout << "input " << a << " reference 1/fa " << reference << " pref " << double(pref) << '(' << pref << ") result " << double(preciprocal) << '(' << preciprocal << ')' << std::endl;
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

	std::string test_suite  = "posit reciprocation verification";
	std::string test_tag    = "reciprocate";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	posit<5, 0> p1(0.75);
	posit<5, 0> p1_reciprocal;
	posit<5, 0> p2(0.75), p2_reciprocal;

	p2_reciprocal = p2.reciprocate();
	p1_reciprocal = p1.reciprocate();

	cout << "posit    : " << to_string(p1_reciprocal) << endl;
	cout << "reference: " << double(p2_reciprocal) << endl;

	GenerateTestCase<4, 0, double>(0.75);
	GenerateTestCase<5, 0, double>(0.75);
	GenerateTestCase<6, 0, double>(0.75);
	GenerateTestCase<16, 0, double>(0.75);
	posit<16, 0> p(1 / 0.75);
	cout << p.get() << " " << pretty_print(p, 17) << endl;

	tag = "Manual Testing: ";
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<3, 0>(tag, true), "posit<3,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<4, 0>(tag, true), "posit<4,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<5, 0>(tag, true), "posit<5,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<6, 0>(tag, true), "posit<6,0>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<5, 1>(tag, true), "posit<5,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<6, 1>(tag, true), "posit<6,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<7, 1>(tag, true), "posit<7,1>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<8, 2>(tag, true), "posit<8,2>", "reciprocation");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<3, 0>>(reportTestCases), "posit<3,0>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<4, 0>>(reportTestCases), "posit<4,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<4, 1>>(reportTestCases), "posit<4,1>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<5, 0>>(reportTestCases), "posit<5,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<5, 1>>(reportTestCases), "posit<5,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<5, 2>>(reportTestCases), "posit<5,2>", "reciprocation");
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<6, 0>>(reportTestCases), "posit<6,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<6, 1>>(reportTestCases), "posit<6,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<6, 2>>(reportTestCases), "posit<6,2>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<6, 3>>(reportTestCases), "posit<6,3>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<7, 0>>(reportTestCases), "posit<7,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<7, 1>>(reportTestCases), "posit<7,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<7, 2>>(reportTestCases), "posit<7,2>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<7, 3>>(reportTestCases), "posit<7,3>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<7, 4>>(reportTestCases), "posit<7,4>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<8, 0>>(reportTestCases), "posit<8,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<8, 1>>(reportTestCases), "posit<8,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<8, 2>>(reportTestCases), "posit<8,2>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<8, 3>>(reportTestCases), "posit<8,3>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<8, 4>>(reportTestCases), "posit<8,4>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<8, 5>>(reportTestCases), "posit<8,5>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<10, 1>>(reportTestCases), "posit<10,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<12, 1>>(reportTestCases), "posit<12,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<14, 1>>(reportTestCases), "posit<14,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<16, 1>>(reportTestCases), "posit<16,1>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<18, 1>>(reportTestCases), "posit<18,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<20, 1>>(reportTestCases), "posit<20,1>", "reciprocation");
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