// conversion.cpp : functional tests for conversion operators to posit numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

// if you want to trace the posit intermediate results
// #define POSIT_VERBOSE_OUTPUT

#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;

template<size_t nbits, size_t es>
void GenerateLogicPattern(double input, const posit<nbits, es>& presult, const posit<nbits+1, es>& pnext) {
	const int VALUE_WIDTH = 15;
	bool fail = fabs(presult.to_double() - pnext.to_double()) > 0.000000001;
	value<52> v(input);
	std::cout << setw(VALUE_WIDTH) << input << " "
		<< " result " << setw(VALUE_WIDTH) << presult 
		<< "  scale= " << std::setw(3) << presult.scale() 
		<< "  k= " << std::setw(3) << presult.calculate_k(v.scale())
		<< "  exp= " << std::setw(3) << presult.get_exponent() << "  "
		<< presult.get() << " " 
		<< pnext.get() << " "
		<< setw(VALUE_WIDTH) << pnext << " "
		<< (fail ? "FAIL" : "    PASS")
		<< std::endl;
}

template<size_t nbits, size_t es>
void GenerateLogicPatternsForDebug() {
	// we are going to generate a test set that consists of all posit configs and their midpoints
	// we do this by enumerating a posit that is 1-bit larger than the test posit configuration
	const int NR_TEST_CASES = (1 << (nbits + 1));
	const int HALF = (1 << nbits);
	posit<nbits + 1, es> pref, pprev, pnext;

	// execute the test
	int nrOfFailedTests = 0;
	const double eps = 1.0e-10;  // TODO for big posits, eps is important to resolve differences
	double da, input;
	posit<nbits, es> pa;
	std::cout << spec_to_string(pa) << std::endl;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		pref.set_raw_bits(i);
		da = pref.to_double();
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
void GenerateTestCase(float input, float reference, const posit<nbits, es>& presult) {
	if (fabs(presult.to_double() - reference) > 0.000000001) 
		ReportConversionError("test_case", "=", input, reference, presult);
	else
		ReportConversionSuccess("test_case", "=", input, reference, presult);
	cout << endl;
}

template<size_t nbits, size_t es>
void GenerateTestCase(double input, double reference, const posit<nbits, es>& presult) {
	if (fabs(presult.to_double() - reference) > 0.000000001)
		ReportConversionError("test_case", "=", input, reference, presult);
	else
		ReportConversionSuccess("test_case", "=", input, reference, presult);
	cout << endl;
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Conversion failed";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	double input, reference;
	
#if PREVIOUS_FAILURE_INPUTS
	posit<4, 1> p;
	input = 0.0625f; reference = 0.0625f; 
	input = 0.1249f; reference = 0.0625f;
	input = 0.1251f; reference = 0.25f;
	input = 0.249999999f; reference = 0.25f;
	input = 4.000001f; reference = 4.0f;
	posit<5, 2> p;
	input = 32.0001; reference = 64; 
	input = 63.9999; reference = 64; 
	input = 128.0001; reference = 256;
	input = 255.9999; reference = 256;
	input = 256.0001; reference = 256;
	input = 1023.9999; reference = 256;
	input = 0.5; reference = 0.5;
	input = 0.50001; reference = 0.5;
	input = 0.74999; reference = 0.5;
#endif
	value<52> v(0.031250001);
	cout << components(v) << endl;
	return 0;
	posit<4, 1> p;
	input = 0.015625; reference = 0.0625;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 0.03125; reference = 0.0625;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 0.0624; reference = 0.0625;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 0.06251; reference = 0.0625;
	p = input;
	GenerateTestCase(input, reference, p);

	input = 0.1249; reference = 0.0625;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 0.1251; reference = 0.25;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 0.125; reference = 0.25;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 0.2499; reference = 0.25;
	p = input;
	GenerateTestCase(input, reference, p);

	input = 3.99; reference = 4.0;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 4.01; reference = 4.0;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 7.99; reference = 4.0;
	p = input;
	GenerateTestCase(input, reference, p);
	input = 8.01; reference = 16.0;
	p = input;
	GenerateTestCase(input, reference, p);
	// return 0;
	// manual exhaustive testing
	tag = "Manual Testing";

	//GenerateLogicPatternsForDebug<3, 0>();
	//GenerateLogicPatternsForDebug<4, 0>();	
	GenerateLogicPatternsForDebug<4, 1>();
	GenerateLogicPatternsForDebug<5, 1>();
	//GenerateLogicPatternsForDebug<6, 2>();
	//GenerateLogicPatternsForDebug<7, 3>();
	//GenerateLogicPatternsForDebug<8, 0>();
	//GenerateLogicPatternsForDebug<8, 1>();
	 return 0;

	nrOfFailedTestCases += ReportTestResult(ValidateConversion<8, 0>(tag, true), "posit<8,0>", "conversion");
	return 0;

	nrOfFailedTestCases += ReportTestResult(ValidateConversion<4, 1>(tag, true), "posit<4,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<5, 2>(tag, true), "posit<5,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<6, 3>(tag, true), "posit<6,3>", "conversion");
	return 0;

	nrOfFailedTestCases += ReportTestResult(ValidateConversion<4, 0>(tag, true), "posit<4,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<4, 1>(tag, true), "posit<4,1>", "conversion"); 
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<5, 0>(tag, true), "posit<5,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<5, 1>(tag, true), "posit<5,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<5, 2>(tag, true), "posit<5,2>", "conversion");
	return 0;
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 0>("Posit<6,0> addition failed: ", bReportIndividualTestCases), "posit<6,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 1>("Posit<6,1> addition failed: ", bReportIndividualTestCases), "posit<6,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 2>("Posit<6,2> addition failed: ", bReportIndividualTestCases), "posit<6,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 3>("Posit<6,3> addition failed: ", bReportIndividualTestCases), "posit<6,3>", "addition");
	return 0;

#else

	cout << "Posit conversion validation" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<10, 0>(tag, bReportIndividualTestCases), "posit<10,0>", "conversion");

	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "conversion");

	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "conversion");

	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "conversion");


#ifdef STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(ValidateConversion<16, 0>(tag, bReportIndividualTestCases), "posit<16,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<16, 2>(tag, bReportIndividualTestCases), "posit<16,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<16, 3>(tag, bReportIndividualTestCases), "posit<16,3>", "conversion");

#endif // STRESS_TESTING


#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}

