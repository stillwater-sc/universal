// function_pow.cpp: test suite runner for pow function
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>  // standard C++ version of pow()

// when you define POSIT_VERBOSE_OUTPUT the code will print intermediate results for selected arithmetic operations
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_POW

// minimum set of include files to reflect source code dependencies
#include <universal/native/integers.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/posit/manipulators.hpp>
#include <universal/number/posit/math/pow.hpp>
#include <universal/number/quire/exceptions.hpp>  // math library might use quire
#include <universal/verification/posit_math_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pb, pref, ppow;
	pa = a;
	pb = b;
	ref = std::pow(a,b);
	pref = ref;
	ppow = sw::universal::pow(pa,pb);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << " -> pow(" << a << "," << b << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << " -> pow( " << pa << "," << pb << ") = " << ppow.get() << " (reference: " << pref.get() << ")   " ;
	std::cout << (pref == ppow ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::string tag = "Addition failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, 1, float>(4.0f, 2.0f);

#if GENERATE_POW_TABLES
	GeneratePowTable<3, 0>();
	GeneratePowTable<4, 0>();
	GeneratePowTable<4, 1>();
	GeneratePowTable<5, 0>();
	GeneratePowTable<5, 1>();
	GeneratePowTable<5, 2>();
	GeneratePowTable<6, 0>();
	GeneratePowTable<6, 1>();
	GeneratePowTable<6, 2>();
	GeneratePowTable<6, 3>();
	GeneratePowTable<7, 0>();
#endif

	cout << endl;

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<2, 0>("Manual Testing", true), "posit<2,0>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<3, 0>("Manual Testing", true), "posit<3,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<3, 1>("Manual Testing", true), "posit<3,1>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<4, 0>("Manual Testing", true), "posit<4,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<4, 1>("Manual Testing", true), "posit<4,1>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<5, 0>("Manual Testing", true), "posit<5,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<5, 1>("Manual Testing", true), "posit<5,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<5, 2>("Manual Testing", true), "posit<5,2>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 0>("Manual Testing", true), "posit<8,0>", "pow");	
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 1>("Manual Testing", true), "posit<8,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 4>("Manual Testing", true), "posit<8,4>", "pow");

	//nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<16, 1>("Manual Testing", true), "posit<16,1>", "pow");

#else
	bool bReportIndividualTestCases = false;

	cout << "Integer power function\n";
	int a = 2;
	int b = 32;
	cout << "2 ^ 32   = " << ipow(a, b) << endl;
	cout << "2 ^ 32   = " << fastipow(a, b) << endl;

	uint64_t c = 1024;
	uint8_t d = 2;
	cout << "1024 ^ 2 = " << ipow(c, d) << endl;
	cout << "1M ^ 2   = " << ipow(ipow(c, d), d) << endl;

	cout << "Posit Power function validation\n";

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<2, 0>(tag, bReportIndividualTestCases), "posit<2,0>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<3, 1>(tag, bReportIndividualTestCases), "posit<3,1>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<9, 0>(tag, bReportIndividualTestCases), "posit<9,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<9, 1>(tag, bReportIndividualTestCases), "posit<9,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<9, 2>(tag, bReportIndividualTestCases), "posit<9,2>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<9, 3>(tag, bReportIndividualTestCases), "posit<9,3>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<9, 4>(tag, bReportIndividualTestCases), "posit<9,4>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<9, 5>(tag, bReportIndividualTestCases), "posit<9,5>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<9, 6>(tag, bReportIndividualTestCases), "posit<9,6>", "pow");
	
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<10, 0>(tag, bReportIndividualTestCases), "posit<10,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<10, 2>(tag, bReportIndividualTestCases), "posit<10,2>", "pow");
	// fails due to regime representation not being able to be represented by double
	// nrOfFailedTestCases += ReportTestResult(VerifyPowMethod<10, 7>(tag, bReportIndividualTestCases), "posit<10,7>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<12, 0>(tag, bReportIndividualTestCases), "posit<12,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<12, 2>(tag, bReportIndividualTestCases), "posit<12,2>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<16, 0>(tag, bReportIndividualTestCases), "posit<16,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<16, 2>(tag, bReportIndividualTestCases), "posit<16,2>", "pow");


#if STRESS_TESTING
	
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

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
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
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
