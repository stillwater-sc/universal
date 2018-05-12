// math_trigonometric.cpp: functional tests for trigonometric functions (sin/cos/tan/cotan/sec/cosec)
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"

// when you define POSIT_VERBOSE_OUTPUT the code will print intermediate results for selected arithmetic operations
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_SQRT

// minimum set of include files to reflect source code dependencies
#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../../math/trigonometric.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	sw::unum::posit<nbits, es> pa, pref, psin;
	pa = a;
	ref = std::sin(a);
	pref = ref;
	psin = sw::unum::sin(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> sin(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " -> sin( " << pa << ") = " << psin.get() << " (reference: " << pref.get() << ")   " ;
	std::cout << (pref == psin ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0


int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "Addition failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, 1, float>(4.0f);

	cout << endl;

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(ValidateSine<2, 0>("Manual Testing", true), "posit<2,0>", "sin");

	nrOfFailedTestCases += ReportTestResult(ValidateSine<3, 0>("Manual Testing", true), "posit<3,0>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<3, 1>("Manual Testing", true), "posit<3,1>", "sin");

	nrOfFailedTestCases += ReportTestResult(ValidateSine<4, 0>("Manual Testing", true), "posit<4,0>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<4, 1>("Manual Testing", true), "posit<4,1>", "sin");

	nrOfFailedTestCases += ReportTestResult(ValidateSine<5, 0>("Manual Testing", true), "posit<5,0>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<5, 1>("Manual Testing", true), "posit<5,1>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<5, 2>("Manual Testing", true), "posit<5,2>", "sin");

	nrOfFailedTestCases += ReportTestResult(ValidateSine<8, 0>("Manual Testing", true), "posit<8,0>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateCosine<8, 0>("Manual Testing", true), "posit<8,0>", "cos");
	nrOfFailedTestCases += ReportTestResult(ValidateTangent<8, 0>("Manual Testing", true), "posit<8,0>", "tan");
	nrOfFailedTestCases += ReportTestResult(ValidateAtan<8, 0>("Manual Testing", true), "posit<8,0>", "atan");
	nrOfFailedTestCases += ReportTestResult(ValidateAsin<8, 0>("Manual Testing", true), "posit<8,0>", "asin");
	nrOfFailedTestCases += ReportTestResult(ValidateAcos<8, 0>("Manual Testing", true), "posit<8,0>", "acos");
#else

	cout << "Posit sine function validation" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateSine<2, 0>(tag, bReportIndividualTestCases), "posit<2,0>", "sin");

	nrOfFailedTestCases += ReportTestResult(ValidateSine<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<3, 1>(tag, bReportIndividualTestCases), "posit<3,1>", "sin");

	nrOfFailedTestCases += ReportTestResult(ValidateSine<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "sin");

	nrOfFailedTestCases += ReportTestResult(ValidateSine<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "sin");

	nrOfFailedTestCases += ReportTestResult(ValidateSine<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "sin");

	nrOfFailedTestCases += ReportTestResult(ValidateSine<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "sin");

	nrOfFailedTestCases += ReportTestResult(ValidateSine<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "sin");

	nrOfFailedTestCases += ReportTestResult(ValidateSine<9, 0>(tag, bReportIndividualTestCases), "posit<9,0>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<9, 1>(tag, bReportIndividualTestCases), "posit<9,1>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<9, 2>(tag, bReportIndividualTestCases), "posit<9,2>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<9, 3>(tag, bReportIndividualTestCases), "posit<9,3>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<9, 4>(tag, bReportIndividualTestCases), "posit<9,4>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<9, 5>(tag, bReportIndividualTestCases), "posit<9,5>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<9, 6>(tag, bReportIndividualTestCases), "posit<9,6>", "sin");
	
	nrOfFailedTestCases += ReportTestResult(ValidateSine<10, 0>(tag, bReportIndividualTestCases), "posit<10,0>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<10, 2>(tag, bReportIndividualTestCases), "posit<10,2>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<10, 7>(tag, bReportIndividualTestCases), "posit<10,7>", "sin");

	nrOfFailedTestCases += ReportTestResult(ValidateSine<12, 0>(tag, bReportIndividualTestCases), "posit<12,0>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<12, 2>(tag, bReportIndividualTestCases), "posit<12,2>", "sin");

	nrOfFailedTestCases += ReportTestResult(ValidateSine<16, 0>(tag, bReportIndividualTestCases), "posit<16,0>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<16, 2>(tag, bReportIndividualTestCases), "posit<16,2>", "sin");


#if STRESS_TESTING
	// nbits=64 requires long double compiler support
	// nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 2>(tag, bReportIndividualTestCases, OPCODE_SQRT, 1000), "posit<64,2>", "sin");


	nrOfFailedTestCases += ReportTestResult(ValidateSine<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "sin");
	nrOfFailedTestCases += ReportTestResult(ValidateSine<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "sin");
	
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
