// arithmetic_sqrt.cpp: functional tests for sqrt
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

// when you define POSIT_VERBOSE_OUTPUT the code will print intermediate results for selected arithmetic operations
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_SQRT

// minimum set of include files to reflect source code dependencies
#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../../posit/math_functions.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;
using namespace sw::unum;

template<size_t nbits, size_t es>
void GenerateSqrtTable() {
	constexpr unsigned int NR_POSITS = (unsigned int(1) << (nbits-1)); // no need for negative posits

	std::cout << setprecision(20);
	posit<nbits, es> p;
	for (unsigned int i = 0; i < NR_POSITS; i++) {
		p.set_raw_bits(i);
		double ref = std::sqrt(double(p));
		posit<nbits, es> psqrt(ref);
		std::cout << p.get() << " " << psqrt.get() << "      " << p << " " << psqrt << " ref: " << ref << std::endl;
	}
	std::cout << setprecision(5);
}

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	posit<nbits, es> pa, pref, psqrt;
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

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Addition failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<6, 3, double>(INFINITY);
	GenerateTestCase<16, 0, float>(0.5f);

#if LATER
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

	cout << endl;

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<3, 0>("Manual Testing", true), "posit<3,0>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<4, 0>("Manual Testing", true), "posit<4,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<4, 1>("Manual Testing", true), "posit<4,1>", "sqrt");

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<5, 0>("Manual Testing", true), "posit<5,0>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<5, 1>("Manual Testing", true), "posit<5,1>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<5, 2>("Manual Testing", true), "posit<5,2>", "sqrt");

	//nrOfFailedTestCases += ReportTestResult(ValidateSqrt<8, 4>("Manual Testing", true), "posit<8,4>", "sqrt");

#else

	cout << "Posit addition validation" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateSqrt<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "sqrt");

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
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
	return EXIT_FAILURE;
}
