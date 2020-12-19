// conversion.cpp : functional tests for conversion operators to posit numbers
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// if you want to trace the posit intermediate results
// #define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_CONVERT
// enable the ability to use literals in binary logic and arithmetic operators
#define POSIT_ENABLE_LITERALS 1
// minimum set of include files to reflect source code dependencies
#include "universal/posit/posit.hpp"
#include "universal/posit/posit_functions.hpp"
#include "universal/posit/posit_manipulators.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "../utils/posit_test_helpers.hpp"

template<size_t nbits, size_t es>
void GenerateLogicPattern(double input, const sw::unum::posit<nbits, es>& presult, const sw::unum::posit<nbits+1, es>& pnext) {
	const int VALUE_WIDTH = 15;
	// conceptually: 	bool fail = presult != pnext;
	sw::unum::bitblock<nbits> bbresult = presult.get();
	sw::unum::bitblock<nbits + 1> bbnext = pnext.get();
	bool fail = bbnext[0] == true; // if the last bit of bbnext is set, we fail
	for (size_t i = 0; i < nbits; ++i) {
		if (bbresult[i] != bbnext[i + 1]) {
			fail = true;
			break;
		}
	}
	sw::unum::value<52> v(input);
	std::cout << std::setw(VALUE_WIDTH) << input << " "
		<< " result " << std::setw(VALUE_WIDTH) << presult 
		<< "  scale= " << std::setw(3) << sw::unum::scale(presult) 
		<< "  k= " << std::setw(3) << sw::unum::calculate_k<nbits, es>(v.scale())
		<< "  exp= " << std::setw(3) << presult.get_exponent() << "  "
		<< presult.get() << " " 
		<< pnext.get() << " "
		<< std::setw(VALUE_WIDTH) << pnext << " "
		<< (fail ? "FAIL" : "    PASS")
		<< std::endl;
}

template<size_t nbits, size_t es>
void GenerateLogicPatternsForDebug() {
	// we are going to generate a test set that consists of all posit configs and their midpoints
	// we do this by enumerating a posit that is 1-bit larger than the test posit configuration
	const int NR_TEST_CASES = (1 << (nbits + 1));
	const int HALF = (1 << nbits);
	sw::unum::posit<nbits + 1, es> pref, pprev, pnext;

	// execute the test
	double minpos = sw::unum::minpos_value<nbits+1, es>();
	double eps = 1.0e-10;
	double da, input;
	sw::unum::posit<nbits, es> pa;
	std::cout << sw::unum::dynamic_range(pa) << std::endl;
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
void GenerateTestCase(float input, float reference, const sw::unum::posit<nbits, es>& presult) {
	if (fabs(double(presult) - reference) > 0.000000001) 
		ReportConversionError("test_case", "=", input, reference, presult);
	else
		ReportConversionSuccess("test_case", "=", input, reference, presult);
	std::cout << std::endl;
}

template<size_t nbits, size_t es>
void GenerateTestCase(double input, double reference, const sw::unum::posit<nbits, es>& presult) {
	if (fabs(double(presult) - reference) > 0.000000001)
		ReportConversionError("test_case", "=", input, reference, presult);
	else
		ReportConversionSuccess("test_case", "=", input, reference, presult);
	std::cout << std::endl;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 1

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Conversion test";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	// manual exhaustive testing
	tag = "Manual Testing";

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

	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<3, 0>(tag, true), "posit<3,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<4, 0>(tag, true), "posit<4,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<5, 0>(tag, true), "posit<5,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<6, 0>(tag, true), "posit<6,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<7, 0>(tag, true), "posit<7,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<8, 0>(tag, true), "posit<8,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<9, 0>(tag, true), "posit<9,0>", "conversion");

	nrOfFailedTestCases += ReportTestResult(ValidateConversion<3, 0>(tag, true), "posit<3,0>", "conversion");

	nrOfFailedTestCases += ReportTestResult(ValidateConversion<4, 1>(tag, true), "posit<4,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<5, 2>(tag, true), "posit<5,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<6, 3>(tag, true), "posit<6,3>", "conversion");

	nrOfFailedTestCases += ReportTestResult(ValidateConversion<4, 0>(tag, true), "posit<4,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<4, 1>(tag, true), "posit<4,1>", "conversion"); 
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<5, 0>(tag, true), "posit<5,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<5, 1>(tag, true), "posit<5,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<5, 2>(tag, true), "posit<5,2>", "conversion");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 0>("Posit<6,0> addition failed: ", bReportIndividualTestCases), "posit<6,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 1>("Posit<6,1> addition failed: ", bReportIndividualTestCases), "posit<6,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 2>("Posit<6,2> addition failed: ", bReportIndividualTestCases), "posit<6,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 3>("Posit<6,3> addition failed: ", bReportIndividualTestCases), "posit<6,3>", "addition");

#else

	cout << "Posit conversion validation" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateIntegerConversion<9, 0>(tag, bReportIndividualTestCases), "posit<9,0>", "conversion");

	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 9, 0>(tag, bReportIndividualTestCases), "posit<9,0>", "conversion");

	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 9, 1>(tag, bReportIndividualTestCases), "posit<9,1>", "conversion");

	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 9, 2>(tag, bReportIndividualTestCases), "posit<9,2>", "conversion");

	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 9, 3>(tag, bReportIndividualTestCases), "posit<9,3>", "conversion");


#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<10, 0>(tag, bReportIndividualTestCases), "posit<10,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<10, 2>(tag, bReportIndividualTestCases), "posit<10,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<10, 3>(tag, bReportIndividualTestCases), "posit<10,3>", "conversion");

	nrOfFailedTestCases += ReportTestResult(ValidateConversion<12, 0>(tag, bReportIndividualTestCases), "posit<12,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<12, 2>(tag, bReportIndividualTestCases), "posit<12,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<12, 3>(tag, bReportIndividualTestCases), "posit<12,3>", "conversion");

	nrOfFailedTestCases += ReportTestResult(ValidateConversion<14, 0>(tag, bReportIndividualTestCases), "posit<14,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<14, 2>(tag, bReportIndividualTestCases), "posit<14,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<14, 3>(tag, bReportIndividualTestCases), "posit<14,3>", "conversion");

	nrOfFailedTestCases += ReportTestResult(ValidateConversion<16, 0>(tag, bReportIndividualTestCases), "posit<16,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<16, 2>(tag, bReportIndividualTestCases), "posit<16,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(ValidateConversion<16, 3>(tag, bReportIndividualTestCases), "posit<16,3>", "conversion");

#endif // STRESS_TESTING


#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
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

