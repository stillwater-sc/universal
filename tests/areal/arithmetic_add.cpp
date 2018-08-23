// arithmetic_add.cpp: functional tests for addition on arbitrary reals
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
// minimum set of include files to reflect source code dependencies
#include "../../posit/exceptions.hpp"
#include "../../posit/trace_constants.hpp"
#include "../../bitblock/bitblock.hpp"
#include "../../posit/bit_functions.hpp"
#include "../../areal/areal.hpp"
#include "../test_helpers.hpp"
#include "areal_test_helpers.hpp"

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::unum::areal<nbits, es> pa, pb, pref, psum;
	pa = a;
	pb = b;
	ref = a + b;
	pref = ref;
	psum = pa + pb;
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " + " << std::setw(nbits) << b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " + " << pb.get() << " = " << psum.get() << " (reference: " << pref.get() << ")   " ;
	std::cout << (pref == psum ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

	std::string tag = "Addition failed: ";

#if MANUAL_TESTING
//	bool bReportIndividualTestCases = false;

	// generate individual testcases to hand trace/debug
	//GenerateTestCase<16, 5, double>(INFINITY, INFINITY);
	//GenerateTestCase<8, 2, float>(0.5f, -0.5f);

	// manual exhaustive test
	//nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 2>("Manual Testing", true), "areal<8,2>", "addition");

#else

	cout << "Posit addition validation" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<2, 0>(tag, bReportIndividualTestCases), "posit<2,0>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<3, 1>(tag, bReportIndividualTestCases), "posit<3,1>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<16, 1>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<16,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<24, 1>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<24,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 1>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<32,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 2>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<32,2>", "addition");

#if STRESS_TESTING
	// nbits=48 also shows failures
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<48, 2>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<48,2>", "addition");

	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 2>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<64,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 3>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<64,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 4>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<64,4>", "addition");


	nrOfFailedTestCases += ReportTestResult(ValidateAddition<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "addition");
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
