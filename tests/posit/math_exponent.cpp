// math_exponent.cpp: functional tests for exponent (exp, exp2, exp10) function
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"

// when you define POSIT_VERBOSE_OUTPUT the code will print intermediate results for selected arithmetic operations
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_POW

// minimum set of include files to reflect source code dependencies
#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../../posit/math/exponent.hpp"
#include "../test_helpers.hpp"
#include "../posit_math_helpers.hpp"

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	sw::unum::posit<nbits, es> pa, pref, pexp;
	pa = a;
	ref = std::exp(a);
	pref = ref;
	pexp = sw::unum::exp(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> exp(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " -> exp( " << pa << ") = " << pexp.get() << " (reference: " << pref.get() << ")   ";
	std::cout << (pref == pexp ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0


int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	//bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "Addition failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, 1, float>(4.0f);

#if GENERATE_EXPONENT_TABLES
	GenerateExponentTable<3, 0>();
	GenerateExponentTable<4, 0>();
	GenerateExponentTable<4, 1>();
	GenerateExponentTable<5, 0>();
	GenerateExponentTable<5, 1>();
	GenerateExponentTable<5, 2>();
	GenerateExponentTable<6, 0>();
	GenerateExponentTable<6, 1>();
	GenerateExponentTable<6, 2>();
	GenerateExponentTable<6, 3>();
	GenerateExponentTable<7, 0>();
#endif

	cout << endl;

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(ValidateExp<2, 0>("Manual Testing", true), "posit<2,0>", "exp");

	nrOfFailedTestCases += ReportTestResult(ValidateExp<3, 0>("Manual Testing", true), "posit<3,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<3, 1>("Manual Testing", true), "posit<3,1>", "exp");

	nrOfFailedTestCases += ReportTestResult(ValidateExp<4, 0>("Manual Testing", true), "posit<4,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<4, 1>("Manual Testing", true), "posit<4,1>", "exp");

	nrOfFailedTestCases += ReportTestResult(ValidateExp<5, 0>("Manual Testing", true), "posit<5,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<5, 1>("Manual Testing", true), "posit<5,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<5, 2>("Manual Testing", true), "posit<5,2>", "exp");

	nrOfFailedTestCases += ReportTestResult(ValidateExp<8, 4>("Manual Testing", true), "posit<8,4>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp2<8, 4>("Manual Testing", true), "posit<8,4>", "exp2");

#else

	cout << "Posit Power function validation" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateExp<2, 0>(tag, bReportIndividualTestCases), "posit<2,0>", "exp");

	nrOfFailedTestCases += ReportTestResult(ValidateExp<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<3, 1>(tag, bReportIndividualTestCases), "posit<3,1>", "exp");

	nrOfFailedTestCases += ReportTestResult(ValidateExp<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "exp");

	nrOfFailedTestCases += ReportTestResult(ValidateExp<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "exp");

	nrOfFailedTestCases += ReportTestResult(ValidateExp<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "exp");

	nrOfFailedTestCases += ReportTestResult(ValidateExp<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "exp");

	nrOfFailedTestCases += ReportTestResult(ValidateExp<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "exp");

	nrOfFailedTestCases += ReportTestResult(ValidateExp<9, 0>(tag, bReportIndividualTestCases), "posit<9,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<9, 1>(tag, bReportIndividualTestCases), "posit<9,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<9, 2>(tag, bReportIndividualTestCases), "posit<9,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<9, 3>(tag, bReportIndividualTestCases), "posit<9,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<9, 4>(tag, bReportIndividualTestCases), "posit<9,4>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<9, 5>(tag, bReportIndividualTestCases), "posit<9,5>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<9, 6>(tag, bReportIndividualTestCases), "posit<9,6>", "exp");
	
	nrOfFailedTestCases += ReportTestResult(ValidateExp<10, 0>(tag, bReportIndividualTestCases), "posit<10,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<10, 2>(tag, bReportIndividualTestCases), "posit<10,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<10, 7>(tag, bReportIndividualTestCases), "posit<10,7>", "exp");

	nrOfFailedTestCases += ReportTestResult(ValidateExp<12, 0>(tag, bReportIndividualTestCases), "posit<12,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<12, 2>(tag, bReportIndividualTestCases), "posit<12,2>", "exp");

	nrOfFailedTestCases += ReportTestResult(ValidateExp<16, 0>(tag, bReportIndividualTestCases), "posit<16,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(ValidateExp<16, 2>(tag, bReportIndividualTestCases), "posit<16,2>", "exp");


#if STRESS_TESTING
	
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

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
