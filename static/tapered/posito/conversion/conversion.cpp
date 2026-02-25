// conversion.cpp : test suite runner for conversion operators to posit numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// if you want to trace the posit intermediate results
// #define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_CONVERT
// enable the ability to use literals in binary logic and arithmetic operators
#define POSIT_ENABLE_LITERALS 1
//#define POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_8_2 1
#define POSIT_FAST_POSIT_16_2 1
#include <universal/number/posit1/posit1.hpp>
#include <universal/number/posito/posito.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/posit_test_suite.hpp>

template<size_t nbits, size_t es>
void GenerateLogicPatternsForDebug() {
	// we are going to generate a test set that consists of all posit configs and their midpoints
	// we do this by enumerating a posit that is 1-bit larger than the test posit configuration
	const int NR_TEST_CASES = (1 << (nbits + 1));
	const int HALF = (1 << nbits);
	sw::universal::posit<nbits + 1, es> pref, pprev, pnext;

	// execute the test
	double minpos = double(sw::universal::posit<nbits+1, es>(sw::universal::SpecificValue::minpos));
	double eps = 1.0e-10;
	double da, input;
	sw::universal::posit<nbits, es> pa;
	std::cout << sw::universal::dynamic_range(pa) << std::endl;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		pref.set_raw_bits(i);
		da = double(pref);
		if (i == 0) {
			eps = minpos / 2.0;
		}
		else {
			eps = da > 0 ? da * 1.0e-6 : da * -1.0e-6;
		}	
		if (i % 2) {
			if (i == 1) {
				// special case of projecting to +minpos
				// even the -delta goes to +minpos
				input = da - eps;
				pa = input;
				pnext.set_raw_bits(i + 1);
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pnext);
				input = da + eps;
				pa = input;
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pnext);
			}
			else if (i == HALF - 1) {
				// special case of projecting to +maxpos
				input = da - eps;
				pa = input;
				pprev.set_raw_bits(HALF - 2);
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pprev);
			}
			else if (i == HALF + 1) {
				// special case of projecting to -maxpos
				input = da - eps;
				pa = input;
				pprev.set_raw_bits(HALF + 2);
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pprev);
			}
			else if (i == NR_TEST_CASES - 1) {
				// special case of projecting to -minpos
				// even the +delta goes to -minpos
				input = da - eps;
				pa = input;
				pprev.set_raw_bits(i - 1);
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pprev);
				input = da + eps;
				pa = input;
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pprev);
			}
			else {
				// for odd values, we are between posit values, so we create the round-up and round-down cases
				// round-down
				input = da - eps;
				pa = input;
				pprev.set_raw_bits(i - 1);
				std::cout << "d"; // indicate that this needs to round down
				GenerateLogicPattern(input, pa, pprev);
				// round-up
				input = da + eps;
				pa = input;
				pnext.set_raw_bits(i + 1);
				std::cout << "u"; // indicate that this needs to round up
				GenerateLogicPattern(input, pa, pnext);
			}
		}
		else {
			// for the even values, we generate the round-to-actual cases
			if (i == 0) {
				// special case of projecting to +minpos
				input = da + eps;
				pa = input;
				pnext.set_raw_bits(i + 2);
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pnext);
			}
			else if (i == NR_TEST_CASES - 2) {
				// special case of projecting to -minpos
				input = da - eps;
				pa = input;
				pprev.set_raw_bits(NR_TEST_CASES - 2);
				std::cout << "p"; // indicate that this needs to 'project'
				GenerateLogicPattern(input, pa, pprev);
			}
			else {
				// round-up
				input = da - eps;
				pa = input;
				std::cout << "u"; // indicate that this needs to round up
				GenerateLogicPattern(input, pa, pref);
				// round-down
				input = da + eps;
				pa = input;
				std::cout << "d"; // indicate that this needs to round down
				GenerateLogicPattern(input, pa, pref);
			}
		}
	}
}
// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es>
void GenerateTestCase(float input, float reference, const sw::universal::posit<nbits, es>& presult) {
	if (fabs(double(presult) - reference) > 0.000000001) 
		ReportConversionError("test_case", "=", input, reference, presult);
	else
		ReportConversionSuccess("test_case", "=", input, reference, presult);
	std::cout << std::endl;
}

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
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "posit conversion validation";
	std::string test_tag    = "conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	// manual exhaustive testing

#ifdef VERBOSE_ENUMERATION_TESTING
	GenerateLogicPatternsForDebug<3, 0>();
	GenerateLogicPatternsForDebug<4, 0>();	
	GenerateLogicPatternsForDebug<4, 1>();
	GenerateLogicPatternsForDebug<5, 1>();
	GenerateLogicPatternsForDebug<5, 2>();
	GenerateLogicPatternsForDebug<6, 2>();
	GenerateLogicPatternsForDebug<7, 3>();
	GenerateLogicPatternsForDebug<8, 0>();
	GenerateLogicPatternsForDebug<8, 1>();
	GenerateLogicPatternsForDebug<8, 2>();
	cout << "----------------\n";
#endif

	using TestType = posit<16, 2>;
	using EnvelopeType = posit<17, 2>;
	TestType p = 0.06251519627f;
	std::string typeTag = type_tag(p);
	ReportValue(p);
	// posito<16, 2> p1 = 0.06253051758f;

	// conversion tests
	std::cout << "Assignment/conversion tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<TestType>(reportTestCases), typeTag, "integer conversion (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<TestType, EnvelopeType, float>(reportTestCases), typeTag, "float conversion   (native)  ");
	// FAIL = 0.06251519627             did not convert to 0.06253051758             instead it yielded  0.0625                     raw 0b0.01.00.00000000000
	// FAIL = 0.9998789296              did not convert to 0.9997558594              instead it yielded  1                          raw 0b0.10.00.00000000000
	//	posit< 16, 2>                                                double assign(native)   FAIL 2 failed test cases
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<TestType, EnvelopeType, double>(reportTestCases), typeTag, "double conversion  (native)  ");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 8, 2>, posit<9, 2>, float>(reportTestCases), "posit< 8, 2>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 5, 2>, posit<6, 2>, float>(reportTestCases), "posit<5,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 6, 2>, posit<7, 2>, float>(reportTestCases), "posit<6,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 7, 2>, posit<8, 2>, float>(reportTestCases), "posit<7,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 8, 2>, posit<9, 2>, float>(reportTestCases), "posit<8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posito< 8, 2>, posito<9, 2>, float>(reportTestCases), "posito<8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit< 9, 2>, posit<10, 2>, float>(reportTestCases), "posit<9,2>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posit<16, 2>, posit<17, 2>, float>(true), "posit<16,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<posito<16, 2>, posito<17, 2>, float>(true), "posito<16,2>", test_tag);

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

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

