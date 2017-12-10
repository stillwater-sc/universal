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
using namespace sw::unum;

template<size_t nbits, size_t es>
void GenerateLogicPattern(double input, const posit<nbits, es>& presult, const posit<nbits+1, es>& pnext) {
	const int VALUE_WIDTH = 15;
	bool fail = fabs(presult.to_double() - pnext.to_double()) > 0.000000001;
	value<52> v(input);
	std::cout << setw(VALUE_WIDTH) << input << " "
		<< " result " << setw(VALUE_WIDTH) << presult 
		<< "  scale= " << std::setw(3) << presult.scale() 
		<< "  k= " << std::setw(3) << calculate_k<nbits, es>(v.scale())
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
	/*
	conversion failures for <4,1>
	no exp left : geo-dw d          0.125  result          0.0625  scale=  -4  k=  -2  exp=   -  0001 00010          0.0625     PASS
	no rounding alltaken u          0.125  result             0.5  scale=  -1  k=  -1  exp=   1  0011 00100            0.25 FAIL
	no rounding alltaken u           0.25  result               1  scale=   0  k=  -1  exp=   0  0100 00100            0.25 FAIL
	no rounding alltaken d           0.25  result            0.25  scale= - 2  k=  -1  exp=   0  0010 00100            0.25     PASS
	no rounding alltaken u          -0.25  result           -0.25  scale=  -2  k=  -1  exp=   0  1110 11100           -0.25     PASS
	no rounding alltaken d          -0.25  result              -1  scale=   0  k=  -1  exp=   0  1100 11100           -0.25 FAIL
	no rounding alltaken d         -0.125  result            -0.5  scale=  -1  k=  -1  exp=   1  1101 11100           -0.25 FAIL
	no exp left:  geo-dw u         -0.125  result         -0.0625  scale=  -4  k=  -2  exp=   -  1111 11110         -0.0625     PASS
	// incoming values and their corresponding bits
	float:            0.1249900013 Sign: 0 Scale: -4 Fraction: b11111111111101011000010
	float:                   0.125 Sign: 0 Scale: -3 Fraction: b00000000000000000000000
	float:            0.1250099987 Sign: 0 Scale: -3 Fraction: b00000000000001010011111
	float:            0.2499900013 Sign: 0 Scale: -3 Fraction: b11111111111110101100001
	float:                    0.25 Sign: 0 Scale: -2 Fraction: b00000000000000000000000
	float:            0.2500100136 Sign: 0 Scale: -2 Fraction: b00000000000000101010000
	float:           -0.2500100136 Sign: 1 Scale: -2 Fraction: b00000000000000101010000
	float:                   -0.25 Sign: 1 Scale: -2 Fraction: b00000000000000000000000
	float:           -0.2499900013 Sign: 1 Scale: -3 Fraction: b11111111111110101100001
	float:           -0.1250099987 Sign: 1 Scale: -3 Fraction: b00000000000001010011111
	float:                  -0.125 Sign: 1 Scale: -3 Fraction: b00000000000000000000000
	float:           -0.1249900013 Sign: 1 Scale: -4 Fraction: b11111111111101011000010

	conversion failures for <5,1>
	no exp left:  geo-dw d        0.03125  result        0.015625  scale=  -6  k=  -3  exp=   -  00001 000010        0.015625     PASS
	no rounding alltaken u        0.03125  result           0.125  scale=  -3  k=  -2  exp=   1  00011 000100          0.0625 FAIL
	no rounding alltaken u         0.0625  result            0.25  scale=  -2  k=  -2  exp=   0  00100 000100          0.0625 FAIL
	no rounding alltaken d         0.0625  result          0.0625  scale=  -4  k=  -2  exp=   0  00010 000100          0.0625     PASS
	no rounding alltaken u          0.125  result           0.125  scale=  -3  k=  -2  exp=   1  00011 000110           0.125     PASS
	arithmetic  rounding d          0.125  result             0.5  scale=  -1  k=  -1  exp=   1  00110 000110           0.125 FAIL
	arithmetic  rounding d         0.1875  result            0.75  scale=  -1  k=  -1  exp=   1  00111 000110           0.125 FAIL
	arithmetic  rounding u         0.1875  result            0.75  scale=  -1  k=  -1  exp=   1  00111 001000            0.25 FAIL
	arithmetic  rounding u           0.25  result               1  scale=   0  k=  -1  exp=   0  01000 001000            0.25 FAIL
	arithmetic  rounding d           0.25  result            0.25  scale=  -2  k=  -1  exp=   0  00100 001000            0.25     PASS
	arithmetic  rounding u          -0.25  result           -0.25  scale=  -2  k=  -1  exp=   0  11100 111000           -0.25     PASS
	arithmetic  rounding d          -0.25  result              -1  scale=   0  k=  -1  exp=   0  11000 111000           -0.25 FAIL
	arithmetic  rounding d        -0.1875  result           -0.75  scale=  -1  k=  -1  exp=   1  11001 111000           -0.25 FAIL
	arithmetic  rounding u        -0.1875  result           -0.75  scale=  -1  k=  -1  exp=   1  11001 111010          -0.125 FAIL
	arithmetic  rounding u         -0.125  result            -0.5  scale=  -1  k=  -1  exp=   1  11010 111010          -0.125 FAIL
	no rounding alltaken d         -0.125  result          -0.125  scale=  -3  k=  -2  exp=   1  11101 111010          -0.125     PASS
	no rounding alltaken d       -0.09375  result          -0.125  scale=  -3  k=  -2  exp=   1  11101 111010          -0.125     PASS
	no rounding alltaken u       -0.09375  result         -0.0625  scale=  -4  k=  -2  exp=   0  11110 111100         -0.0625     PASS
	no rounding alltaken u        -0.0625  result         -0.0625  scale=  -4  k=  -2  exp=   0  11110 111100         -0.0625     PASS
	no rounding alltaken d        -0.0625  result           -0.25  scale=  -2  k=  -2  exp=   0  11100 111100         -0.0625 FAIL
	no rounding alltaken d       -0.03125  result          -0.125  scale=  -3  k=  -2  exp=   1  11101 111100         -0.0625 FAIL
	no exp left:  geo-dw u       -0.03125  result       -0.015625  scale=  -6  k=  -3  exp=   -  11111 111110       -0.015625     PASS

	conversion failures for <5,2>
	no exp left:  geo-dw d    0.000244141  result     0.000244141  scale= -12  k=  -3  exp=   --  00001 000010     0.000244141     PASS
	truncated exp geo-up d    0.000976562  result          0.0625  scale=  -4  k=  -2  exp=   0-  00100 000010     0.000244141 FAIL
	truncated exp geo-dw u    0.000976563  result        0.015625  scale=  -6  k=  -2  exp=   1-  00011 000100      0.00390625 FAIL
	truncated exp geo-up u     0.00390625  result        0.015625  scale=  -6  k=  -2  exp=   1-  00011 000100      0.00390625 FAIL
	truncated exp geo-dw d     0.00390625  result      0.00390625  scale=  -8  k=  -2  exp=   0-  00010 000100      0.00390625     PASS
	truncated exp geo-dw d      0.0078125  result      0.00390625  scale=  -8  k=  -2  exp=   0-  00010 000100      0.00390625     PASS
	no rounding alltaken u      0.0078125  result             0.5  scale=  -1  k=  -1  exp=   11  00111 000110        0.015625 FAIL
	no rounding alltaken u       0.015625  result               1  scale=   0  k=  -1  exp=   00  01000 000110        0.015625 FAIL
	no rounding alltaken d       0.015625  result            0.25  scale=  -2  k=  -1  exp=   10  00110 000110        0.015625 FAIL
	no rounding alltaken d        0.03125  result             0.5  scale=  -1  k=  -1  exp=   11  00111 000110        0.015625 FAIL
	no rounding alltaken u        0.03125  result           0.125  scale=  -3  k=  -1  exp=   01  00101 001000          0.0625 FAIL
	no rounding alltaken u         0.0625  result            0.25  scale=  -2  k=  -1  exp=   10  00110 001000          0.0625 FAIL
	no rounding alltaken d         0.0625  result          0.0625  scale=  -4  k=  -1  exp=   00  00100 001000          0.0625     PASS
	no exp left:  geo-dw d            256  result             256  scale=   8  k=   2  exp=   --  01110 011100             256     PASS
	no exp left:  geo-dw d           1024  result             512  scale=   9  k=   2  exp=   --  01110 011100             256 FAIL
	no exp left:  geo-up u           1024  result            2048  scale=  11  k=   2  exp=   --  01110 011110            4096 FAIL
	no exp left:  geo-up u           4096  result            4096  scale=  12  k=   2  exp=   --  01111 011110            4096     PASS
	no exp left:  geo-up d          -4096  result           -4096  scale=  12  k=   2  exp=   --  10001 100010           -4096     PASS
	no exp left:  geo-up d          -1024  result           -2048  scale=  11  k=   2  exp=   --  10010 100010           -4096 FAIL
	no exp left:  geo-dw u          -1024  result            -512  scale=   9  k=   2  exp=   --  10010 100100            -256 FAIL
	no exp left:  geo-dw u           -256  result            -256  scale=   8  k=   2  exp=   --  10010 100100            -256     PASS
	no rounding alltaken u        -0.0625  result         -0.0625  scale=  -4  k=  -1  exp=   00  11100 111000         -0.0625     PASS
	no rounding alltaken d        -0.0625  result           -0.25  scale=  -2  k=  -1  exp=   10  11010 111000         -0.0625 FAIL
	no rounding alltaken d       -0.03125  result          -0.125  scale=  -3  k=  -1  exp=   01  11011 111000         -0.0625 FAIL
	no rounding alltaken u       -0.03125  result            -0.5  scale=  -1  k=  -1  exp=   11  11001 111010       -0.015625 FAIL
	no rounding alltaken u      -0.015625  result           -0.25  scale=  -2  k=  -1  exp=   10  11010 111010       -0.015625 FAIL
	no rounding alltaken d      -0.015625  result              -1  scale=   0  k=  -1  exp=   00  11000 111010       -0.015625 FAIL
	no rounding alltaken d     -0.0078125  result            -0.5  scale=  -1  k=  -1  exp=   11  11001 111010       -0.015625 FAIL
	truncated exp geo-dw u     -0.0078125  result     -0.00390625  scale=  -8  k=  -2  exp=   0-  11110 111100     -0.00390625     PASS
	truncated exp geo-dw u    -0.00390625  result     -0.00390625  scale=  -8  k=  -2  exp=   0-  11110 111100     -0.00390625     PASS
	truncated exp geo-up d    -0.00390625  result       -0.015625  scale=  -6  k=  -2  exp=   1-  11101 111100     -0.00390625 FAIL
	truncated exp geo-dw d   -0.000976563  result       -0.015625  scale=  -6  k=  -2  exp=   1-  11101 111100     -0.00390625 FAIL
	truncated exp geo-up u   -0.000976562  result         -0.0625  scale=  -4  k=  -2  exp=   0-  11100 111110    -0.000244141 FAIL
	no exp left:  geo-dw p   -0.000244141  result    -0.000244141  scale= -12  k=  -3  exp=   --  11111 111110    -0.000244141     PASS

	 */
#endif
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
	//GenerateLogicPatternsForDebug<4, 1>();
	//GenerateLogicPatternsForDebug<5, 1>();
	GenerateLogicPatternsForDebug<5, 2>();
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

