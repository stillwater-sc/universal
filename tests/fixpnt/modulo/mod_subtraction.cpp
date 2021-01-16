// mod_subtraction.cpp: test suite runner for arbitrary configuration fixed-point modulo subtraction
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include <universal/fixpnt/fixed_point.hpp>
#include <universal/fixpnt/manipulators.hpp>
#include <universal/fixpnt/math_functions.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in fixed_point.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t rbits, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::universal::fixpnt<nbits, rbits> a, b, cref, result;
	a = _a;
	b = _b;
	result = a - b;
	ref = _a - _b;
	cref = ref;
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " - " << std::setw(nbits) << _b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << a << " - " << b << " = " << result << " (reference: " << cref << ")   " ;
	std::cout << (cref == result ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "modular subtraction failed: ";

#if MANUAL_TESTING

	fixpnt<8, 4> f;
	f = 3.5f;
	bitset<8> bs(f.byte(0));
	cout << bs << endl;
	cout << f << endl;

	// generate individual testcases to hand trace/debug
	GenerateTestCase<8, 4>(0.5f, 1.0f);

	bReportIndividualTestCases = true;
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 0, Modulo, uint8_t>("Manual Testing", bReportIndividualTestCases), "fixpnt<4,0,Modulo,uint8_t>", "subtraction");

#if STRESS_TESTING
	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 0, Modulo, uint8_t>("Manual Testing", true), "fixpnt<4,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 1>("Manual Testing", true), "fixpnt<4,1,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 2>("Manual Testing", true), "fixpnt<4,2,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 3>("Manual Testing", true), "fixpnt<4,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 4>("Manual Testing", true), "fixpnt<4,4,Modulo,uint8_t>", "subtraction");
#endif

#else

	cout << "Fixed-point modular subtraction validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<5, 0, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<5,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<5, 1, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<5,1,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<5, 2, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<5,2,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<5, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<5,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<5, 4, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<5,4,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<5, 5, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<5,5,Modulo,uint8_t>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 0, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 1, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,1,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 2, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,2,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 4, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,4,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 5, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,5,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 6, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,6,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 7, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,7,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 8, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,8,Modulo,uint8_t>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<9, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<9,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<9, 5, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<9,5,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<9, 7, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<9,7,Modulo,uint8_t>", "subtraction");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyModuloAddition<13, 0, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<13,0,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyModuloAddition<13, 5, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<13,5,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyModuloAddition<13, 9, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<13,9,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyModuloAddition<13, 12, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<13,12,Modulo,uint8_t>", "addition");

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
