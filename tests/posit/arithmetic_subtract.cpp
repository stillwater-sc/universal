// arithmetic_subtract.cpp: functional tests for subtraction
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "common.hpp"

// Configure the posit template environment
// first: enable  general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define POSIT_VERBOSE_OUTPUT executing an SUB the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_SUB

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
// for most bugs they are traceable with _trace_conversion and _trace_sub
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::unum::posit<nbits, es> pa, pb, pref, pdif;
	pa = a;
	pb = b;
	ref = a - b;
	pref = ref;
	pdif = pa - pb;
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " - " << std::setw(nbits) << b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " - " << pb.get() << " = " << pdif.get() << " (reference: " << pref.get() << ")  ";
	std::cout << (pref == pdif ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Subtraction failed: ";

#if MANUAL_TESTING

	//ValidateBitsetSubtraction<4>(true);

	// generate individual testcases to hand trace/debug
	GenerateTestCase<4, 0, double>(0.25, 0.75);
	GenerateTestCase<4, 0, double>(0.25, -0.75);
	GenerateTestCase<8, 0, double>(1.0, 0.25);
	GenerateTestCase<8, 0, double>(1.0, 0.125);
	GenerateTestCase<8, 0, double>(1.0, 1.0);

	// manual exhaustive testing
	std::string positCfg = "posit<4,0>";
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<4, 0>("Manual Testing", true), positCfg, "subtraction");

	// FAIL 011001011010110100000110111110010111010011001010 - 000010111000000110100000001010011011111111110110 != 011001011010110011111111111101100011010001110110 instead it yielded 011001011010110011111111111101100011010001110111
	unsigned long long a = 0b011001011010110100000110111110010111010011001010ull;
	unsigned long long b = 0b000010111000000110100000001010011011111111110110ull;
	posit<48, 2> pa(a);
	posit<48, 2> pb(b);
	posit<48, 2> pdiff = pa - pb;
	cout << pdiff.get() << endl;
	bitblock<48> ba = pa.get();
	cout << a << endl;
	cout << ba << endl;
	//nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<48, 2>(tag, true, OPCODE_SUB, 1000), "posit<48,2>", "subtraction");


#else

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<2, 0>(tag, bReportIndividualTestCases), "posit<2,0>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<3, 1>(tag, bReportIndividualTestCases), "posit<3,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<3, 2>(tag, bReportIndividualTestCases), "posit<3,2>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<3, 3>(tag, bReportIndividualTestCases), "posit<3,3>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<4, 2>(tag, bReportIndividualTestCases), "posit<4,2>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<5, 3>(tag, bReportIndividualTestCases), "posit<5,3>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<6, 4>(tag, bReportIndividualTestCases), "posit<6,4>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<16, 1>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<16,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<24, 1>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<24,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<32, 1>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<32,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<32, 2>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<32,2>", "subtraction");

#if STRESS_TESTING
	// nbits=48 is showing failures
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<48, 2>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<48,2>", "subtraction");

    // nbits=64 requires long double compiler support
    nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 2>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<64,2>", "subtraction");
    nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 3>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<64,3>", "subtraction");
    nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 4>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<64,4>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "subtraction");
#endif

#endif

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
