// arithmetic_mul.cpp: functional tests for multiplication of arbitrary logarithmic number system
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// minimum set of include files to reflect source code dependencies
#include <universal/lns/lns.hpp>
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"

// generate specific test case that you can trace with the trace conditions in areal.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::unum::lns<nbits> pa, pb, pref, psum;
	pa = a;
	pb = b;
	ref = a * b;
	pref = ref;
	psum = pa + pb;
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " * " << std::setw(nbits) << b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " * " << pb.get() << " = " << psum.get() << " (reference: " << pref.get() << ")   " ;
	std::cout << (pref == psum ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<size_t nbits> 
int ValidateMultiplication(const std::string& tag, bool bReportIndividualTestCases) {
	int nrOfFailedTestCases = 0;

	return nrOfFailedTestCases;
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, double>(INFINITY, INFINITY);
	GenerateTestCase<8, float>(0.5f, -0.5f);

	constexpr double e = 2.71828182845904523536;
	lns<16> a, b, c;
	a = 0.5; cout << a << endl;
	a = e; cout << a << endl;
	b = 1.0 / e;
	c = a * b;

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<8>("Manual Testing", true), "lns<8>", "multiplication");

	nrOfFailedTestCases = 0;  // in manual testing mode, we ignore any failures
#else
	cout << "Arbitrary LNS multiplication validation" << endl;

	bool bReportIndividualTestCases = false;
	std::string tag = "multiplication failed: ";

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<8>(tag, bReportIndividualTestCases), "lns<8>", "addition");

#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
