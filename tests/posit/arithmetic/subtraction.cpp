// arithmetic_subtract.cpp: test suite runner for posit subtraction
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

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
#include <universal/number/posit/posit.hpp>
#include <universal/number/posit/numeric_limits.hpp>
#include <universal/number/posit/specializations.hpp>
#include <universal/number/posit/math_functions.hpp>
#include <universal/number/posit/manipulators.hpp>
//#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_randoms.hpp>
#include <universal/verification/posit_math_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_sub
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pb, pref, pdif;
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
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Subtraction failed: ";

#if MANUAL_TESTING

	//VerifyBitsetSubtraction<4>(true);

	// generate individual testcases to hand trace/debug
	GenerateTestCase<4, 0, double>(0.25, 0.75);
	GenerateTestCase<4, 0, double>(0.25, -0.75);
	GenerateTestCase<8, 0, double>(1.0, 0.25);
	GenerateTestCase<8, 0, double>(1.0, 0.125);
	GenerateTestCase<8, 0, double>(1.0, 1.0);

	// manual exhaustive testing
	std::string positCfg = "posit<4,0>";
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 0>("Manual Testing", true), positCfg, "subtraction");

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
	//nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<48, 2>(tag, true, OPCODE_SUB, 1000), "posit<48,2>", "subtraction");


#else

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<2, 0>(bReportIndividualTestCases), "posit<2,0>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<3, 0>(bReportIndividualTestCases), "posit<3,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<3, 1>(bReportIndividualTestCases), "posit<3,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<3, 2>(bReportIndividualTestCases), "posit<3,2>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<3, 3>(bReportIndividualTestCases), "posit<3,3>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 0>(bReportIndividualTestCases), "posit<4,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 1>(bReportIndividualTestCases), "posit<4,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 2>(bReportIndividualTestCases), "posit<4,2>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<5, 0>(bReportIndividualTestCases), "posit<5,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<5, 1>(bReportIndividualTestCases), "posit<5,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<5, 2>(bReportIndividualTestCases), "posit<5,2>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<5, 3>(bReportIndividualTestCases), "posit<5,3>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<6, 0>(bReportIndividualTestCases), "posit<6,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<6, 1>(bReportIndividualTestCases), "posit<6,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<6, 2>(bReportIndividualTestCases), "posit<6,2>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<6, 3>(bReportIndividualTestCases), "posit<6,3>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<6, 4>(bReportIndividualTestCases), "posit<6,4>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<7, 0>(bReportIndividualTestCases), "posit<7,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<7, 1>(bReportIndividualTestCases), "posit<7,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<7, 2>(bReportIndividualTestCases), "posit<7,2>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<7, 3>(bReportIndividualTestCases), "posit<7,3>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<7, 4>(bReportIndividualTestCases), "posit<7,4>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 0>(bReportIndividualTestCases), "posit<8,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 1>(bReportIndividualTestCases), "posit<8,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 2>(bReportIndividualTestCases), "posit<8,2>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 3>(bReportIndividualTestCases), "posit<8,3>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 4>(bReportIndividualTestCases), "posit<8,4>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<8, 5>(bReportIndividualTestCases), "posit<8,5>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<16, 1>(bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<16,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<24, 1>(bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<24,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<32, 1>(bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<32,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<32, 2>(bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<32,2>", "subtraction");

#if STRESS_TESTING
	// nbits=48 is showing failures
	nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<48, 2>(bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<48,2>", "subtraction");

    // nbits=64 requires long double compiler support
    nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 2>(bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<64,2>", "subtraction");
    nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 3>(bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<64,3>", "subtraction");
    nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 4>(bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<64,4>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<10, 1>(bReportIndividualTestCases), "posit<10,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<12, 1>(bReportIndividualTestCases), "posit<12,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<14, 1>(bReportIndividualTestCases), "posit<14,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<16, 1>(bReportIndividualTestCases), "posit<16,1>", "subtraction");
#endif

#endif

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
