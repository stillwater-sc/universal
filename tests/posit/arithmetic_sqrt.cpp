// arithmetic_sqrt.cpp: functional tests for posit sqrt
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define POSIT_VERBOSE_OUTPUT executing an SQRT the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_SQRT

// minimum set of include files to reflect source code dependencies
#include "../../posit/posit.hpp"
#include "../../posit/numeric_limits.hpp"
#include "../../posit/specializations.hpp"
// posit type manipulators such as pretty printers
#include "../../posit/posit_manipulators.hpp"
#include "../../posit/math_functions.hpp"
// test helpers
#include "../test_helpers.hpp"
#include "../posit_test_helpers.hpp"
#include "../posit_math_helpers.hpp"

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	sw::unum::posit<nbits, es> pa, pref, psqrt;
	pa = a;
	ref = std::sqrt(a);
	pref = ref;
	psqrt = sw::unum::sqrt(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> sqrt(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " -> sqrt( " << pa << ") = " << psqrt.get() << " (reference: " << pref.get() << ")   " ;
	std::cout << (pref == psqrt ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 0
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
	//GenerateTestCase<6, 3, double>(INFINITY);
	my_test_sqrt(0.25f);
	GenerateTestCase<3, 1, float>(4.0f);
	posit<3, 1> p(2.0000000001f);
	cout << p.get() << endl;

	return 0;

#if GENERATE_SQRT_TABLES
	GenerateSqrtTable<3, 0>();
	GenerateSqrtTable<4, 0>();
	GenerateSqrtTable<4, 1>();
	GenerateSqrtTable<5, 0>();
	GenerateSqrtTable<5, 1>();
	GenerateSqrtTable<5, 2>();
	GenerateSqrtTable<6, 0>();
	GenerateSqrtTable<6, 1>();
	GenerateSqrtTable<6, 2>();
	GenerateSqrtTable<6, 3>();
	GenerateSqrtTable<7, 0>();
#endif

#if CHECK_REFERENCE_SQRT_ALGORITHM
	// std::sqrt(negative) returns a -NaN(ind)
	cout << setprecision(17);
	float base = 0.5f;
	for (int i = 0; i < 32; i++) {
		float square = base*base;
		float root = sw::unum::my_test_sqrt(square);
		cout << "base " << base << " root " << root << endl;
		base *= 2.0f;
	}
	cout << "sqrt(2.0) " << sw::unum::my_test_sqrt(2.0f) << endl;

#endif

	cout << endl;

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<2, 0>("Manual Testing", true), "posit<2,0>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<3, 0>("Manual Testing", true), "posit<3,0>", "sqrt");
//	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<3, 1>("Manual Testing", true), "posit<3,1>", "sqrt");   // TODO: these configs where nbits < es+sign+regime don't work

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<4, 0>("Manual Testing", true), "posit<4,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<4, 1>("Manual Testing", true), "posit<4,1>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<5, 0>("Manual Testing", true), "posit<5,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<5, 1>("Manual Testing", true), "posit<5,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<5, 2>("Manual Testing", true), "posit<5,2>", "sqrt");

	//nrOfFailedTestCases += ReportTestResult(ValidateSqrt<8, 4>("Manual Testing", true), "posit<8,4>", "sqrt");

#else

	cout << "Posit sqrt validation" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<2, 0>(tag, bReportIndividualTestCases), "posit<2,0>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "sqrt");
//	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<3, 1>(tag, bReportIndividualTestCases), "posit<3,1>", "sqrt");	// TODO: these configs where nbits < es+sign+regime don't work

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<9, 0>(tag, bReportIndividualTestCases), "posit<9,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<9, 1>(tag, bReportIndividualTestCases), "posit<9,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<9, 2>(tag, bReportIndividualTestCases), "posit<9,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<9, 3>(tag, bReportIndividualTestCases), "posit<9,3>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<9, 4>(tag, bReportIndividualTestCases), "posit<9,4>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<9, 5>(tag, bReportIndividualTestCases), "posit<9,5>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<9, 6>(tag, bReportIndividualTestCases), "posit<9,6>", "sqrt");
	
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<10, 0>(tag, bReportIndividualTestCases), "posit<10,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<10, 2>(tag, bReportIndividualTestCases), "posit<10,2>", "sqrt");
	// fails due to regime representation not being able to be represented by double
	// nrOfFailedTestCases += ReportTestResult(ValidateSqrt<10, 7>(tag, bReportIndividualTestCases), "posit<10,7>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<12, 0>(tag, bReportIndividualTestCases), "posit<12,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<12, 2>(tag, bReportIndividualTestCases), "posit<12,2>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<16, 0>(tag, bReportIndividualTestCases), "posit<16,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<16, 2>(tag, bReportIndividualTestCases), "posit<16,2>", "sqrt");


#if STRESS_TESTING
	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 2>(tag, bReportIndividualTestCases, OPCODE_SQRT, 1000), "posit<64,2>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 3>(tag, bReportIndividualTestCases, OPCODE_SQRT, 1000), "posit<64,3>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 4>(tag, bReportIndividualTestCases, OPCODE_SQRT, 1000), "posit<64,4>", "sqrt");


	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "sqrt");
	
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
