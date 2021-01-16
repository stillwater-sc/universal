// function_logarithm.cpp: test suite runner for the logarithm functions (log2, log10, ln)
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// when you define POSIT_VERBOSE_OUTPUT the code will print intermediate results for selected arithmetic operations
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_SQRT

// minimum set of include files to reflect source code dependencies
#include <universal/posit/posit.hpp>
#include <universal/posit/manipulators.hpp>
#include <universal/posit/math/logarithm.hpp>
#include <universal/verification/posit_math_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pref, plog;
	pa = a;
	ref = std::log(a);
	pref = ref;
	plog = sw::universal::log(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> log(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " -> log( " << pa << ") = " << plog.get() << " (reference: " << pref.get() << ")   " ;
	std::cout << (pref == plog ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0


int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	//bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "Addition failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, 1, float>(4.0f);

#if GENERATE_LOG_TABLES
	GenerateLogarithmTable<3, 0>();
	GenerateLogarithmTable<4, 0>();
	GenerateLogarithmTable<4, 1>();
	GenerateLogarithmTable<5, 0>();
	GenerateLogarithmTable<5, 1>();
	GenerateLogarithmTable<5, 2>();
	GenerateLogarithmTable<6, 0>();
	GenerateLogarithmTable<6, 1>();
	GenerateLogarithmTable<6, 2>();
	GenerateLogarithmTable<6, 3>();
	GenerateLogarithmTable<7, 0>();
#endif

	cout << endl;

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyLog<2, 0>("Manual Testing", true), "posit<2,0>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<3, 0>("Manual Testing", true), "posit<3,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<3, 1>("Manual Testing", true), "posit<3,1>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<4, 0>("Manual Testing", true), "posit<4,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<4, 1>("Manual Testing", true), "posit<4,1>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<5, 0>("Manual Testing", true), "posit<5,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<5, 1>("Manual Testing", true), "posit<5,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<5, 2>("Manual Testing", true), "posit<5,2>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<8, 4>("Manual Testing", true), "posit<8,4>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2<8, 4>("Manual Testing", true), "posit<8,4>", "log2");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10<8, 4>("Manual Testing", true), "posit<8,4>", "log10");

#else

	cout << "Posit log validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyLog<2, 0>(tag, bReportIndividualTestCases), "posit<2,0>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<3, 1>(tag, bReportIndividualTestCases), "posit<3,1>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<9, 0>(tag, bReportIndividualTestCases), "posit<9,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<9, 1>(tag, bReportIndividualTestCases), "posit<9,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<9, 2>(tag, bReportIndividualTestCases), "posit<9,2>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<9, 3>(tag, bReportIndividualTestCases), "posit<9,3>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<9, 4>(tag, bReportIndividualTestCases), "posit<9,4>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<9, 5>(tag, bReportIndividualTestCases), "posit<9,5>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<9, 6>(tag, bReportIndividualTestCases), "posit<9,6>", "log");
	
	nrOfFailedTestCases += ReportTestResult(VerifyLog<10, 0>(tag, bReportIndividualTestCases), "posit<10,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<10, 2>(tag, bReportIndividualTestCases), "posit<10,2>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<10, 7>(tag, bReportIndividualTestCases), "posit<10,7>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<12, 0>(tag, bReportIndividualTestCases), "posit<12,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<12, 2>(tag, bReportIndividualTestCases), "posit<12,2>", "log");

	nrOfFailedTestCases += ReportTestResult(VerifyLog<16, 0>(tag, bReportIndividualTestCases), "posit<16,0>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<16, 2>(tag, bReportIndividualTestCases), "posit<16,2>", "log");


#if STRESS_TESTING
	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 2>(tag, bReportIndividualTestCases, OPCODE_SQRT, 1000), "posit<64,2>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 3>(tag, bReportIndividualTestCases, OPCODE_SQRT, 1000), "posit<64,3>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 4>(tag, bReportIndividualTestCases, OPCODE_SQRT, 1000), "posit<64,4>", "log");


	nrOfFailedTestCases += ReportTestResult(VerifyLog<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "log");
	
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
