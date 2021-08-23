// conversion.cpp : test suite runner for conversion operators to posit numbers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// if you want to trace the posit intermediate results
// #define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_CONVERT
// enable the ability to use literals in binary logic and arithmetic operators
#define POSIT_ENABLE_LITERALS 1
// minimum set of include files to reflect source code dependencies
#include <universal/number/posit/posit_impl.hpp>
#include <universal/number/posit/manipulators.hpp>
#include <universal/verification/posit_test_suite.hpp>

template<size_t nbits, size_t es>
void GenerateLogicPattern(double input, const sw::universal::posit<nbits, es>& presult, const sw::universal::posit<nbits+1, es>& pnext) {
	const int VALUE_WIDTH = 15;
	// conceptually: 	bool fail = presult != pnext;
	sw::universal::bitblock<nbits> bbresult = presult.get();
	sw::universal::bitblock<nbits + 1> bbnext = pnext.get();
	bool fail = bbnext[0] == true; // if the last bit of bbnext is set, we fail
	for (size_t i = 0; i < nbits; ++i) {
		if (bbresult[i] != bbnext[i + 1]) {
			fail = true;
			break;
		}
	}
	sw::universal::internal::value<52> v(input);
	std::cout << std::setw(VALUE_WIDTH) << input << " "
		<< " result " << std::setw(VALUE_WIDTH) << presult 
		<< "  scale= " << std::setw(3) << sw::universal::scale(presult) 
		<< "  k= " << std::setw(3) << sw::universal::calculate_k<nbits, es>(v.scale())
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

template<size_t nbits, size_t es>
void GenerateTestCase(double input, double reference, const sw::universal::posit<nbits, es>& presult) {
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
	using namespace sw::universal;

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

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<3, 0>(true), "posit<3,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<4, 0>(true), "posit<4,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<5, 0>(true), "posit<5,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<6, 0>(true), "posit<6,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<7, 0>(true), "posit<7,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<8, 0>(true), "posit<8,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<9, 0>(true), "posit<9,0>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<3, 0>(true), "posit<3,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 1>(true), "posit<4,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<5, 2>(true), "posit<5,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<6, 3>(true), "posit<6,3>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 0>(true), "posit<4,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 1>(true), "posit<4,1>", "conversion"); 
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<5, 0>(true), "posit<5,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<5, 1>(true), "posit<5,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<5, 2>(true), "posit<5,2>", "conversion");

#else

	std::cout << "Posit conversion validation\n";

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<3, 0>(bReportIndividualTestCases), "posit<3,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<4, 0>(bReportIndividualTestCases), "posit<4,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<5, 0>(bReportIndividualTestCases), "posit<5,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<6, 0>(bReportIndividualTestCases), "posit<6,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<7, 0>(bReportIndividualTestCases), "posit<7,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<8, 0>(bReportIndividualTestCases), "posit<8,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<9, 0>(bReportIndividualTestCases), "posit<9,0>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 3, 0>(bReportIndividualTestCases), "posit<3,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 4, 0>(bReportIndividualTestCases), "posit<4,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 5, 0>(bReportIndividualTestCases), "posit<5,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 0>(bReportIndividualTestCases), "posit<6,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 7, 0>(bReportIndividualTestCases), "posit<7,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 0>(bReportIndividualTestCases), "posit<8,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 0>(bReportIndividualTestCases), "posit<9,0>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 4, 1>(bReportIndividualTestCases), "posit<4,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 5, 1>(bReportIndividualTestCases), "posit<5,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 1>(bReportIndividualTestCases), "posit<6,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 7, 1>(bReportIndividualTestCases), "posit<7,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 1>(bReportIndividualTestCases), "posit<8,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 1>(bReportIndividualTestCases), "posit<9,1>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 5, 2>(bReportIndividualTestCases), "posit<5,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 2>(bReportIndividualTestCases), "posit<6,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 7, 2>(bReportIndividualTestCases), "posit<7,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 2>(bReportIndividualTestCases), "posit<8,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 2>(bReportIndividualTestCases), "posit<9,2>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 3>(bReportIndividualTestCases), "posit<6,3>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 7, 3>(bReportIndividualTestCases), "posit<7,3>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 3>(bReportIndividualTestCases), "posit<8,3>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 3>(bReportIndividualTestCases), "posit<9,3>", "conversion");


#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 0>(bReportIndividualTestCases), "posit<10,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 1>(bReportIndividualTestCases), "posit<10,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 2>(bReportIndividualTestCases), "posit<10,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 3>(bReportIndividualTestCases), "posit<10,3>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 0>(bReportIndividualTestCases), "posit<12,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 1>(bReportIndividualTestCases), "posit<12,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 2>(bReportIndividualTestCases), "posit<12,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 3>(bReportIndividualTestCases), "posit<12,3>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 0>(bReportIndividualTestCases), "posit<14,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 1>(bReportIndividualTestCases), "posit<14,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 2>(bReportIndividualTestCases), "posit<14,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 3>(bReportIndividualTestCases), "posit<14,3>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 0>(bReportIndividualTestCases), "posit<16,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 1>(bReportIndividualTestCases), "posit<16,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 2>(bReportIndividualTestCases), "posit<16,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 3>(bReportIndividualTestCases), "posit<16,3>", "conversion");

#endif // STRESS_TESTING


#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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

