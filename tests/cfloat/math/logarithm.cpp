// logarithm.cpp: test suite runner for the logarithm functions (log2, log10, ln)
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default library configuration
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/cfloat_math_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in cfloat.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	sw::universal::cfloat<nbits, es> pa, pref, plog;
	pa = a;
	ref = std::log(a);
	pref = ref;
	plog = sw::universal::log(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> log(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << sw::universal::to_binary(pa) << " -> log( " << pa << ") = " << sw::universal::to_binary(plog) << " (reference: " << sw::universal::to_binary(pref) << ")   " ;
	std::cout << (pref == plog ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0
#define GENERATE_LOG_TABLES 0

int main()
try {
	using namespace sw::universal;

	//bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "cfloat log() failed: ";

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

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyLog< cfloat<8, 4, uint8_t> >(true), "cfloat<8,4>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2< cfloat<8, 4, uint8_t> >(true), "cfloat<8,4>", "log2");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10< cfloat <8, 4, uint8_t > >(true), "cfloat<8,4>", "log10");

#else

	std::cout << "classic floating-point cfloat log validation\n";


#if STRESS_TESTING
	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 2>(bReportIndividualTestCases, OPCODE_SQRT, 1000), "cfloat<64,2>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 3>(bReportIndividualTestCases, OPCODE_SQRT, 1000), "cfloat<64,3>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 4>(bReportIndividualTestCases, OPCODE_SQRT, 1000), "cfloat<64,4>", "log");


	nrOfFailedTestCases += ReportTestResult(VerifyLog<10, 1>(bReportIndividualTestCases), "cfloat<10,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<12, 1>(bReportIndividualTestCases), "cfloat<12,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<14, 1>(bReportIndividualTestCases), "cfloat<14,1>", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog<16, 1>(bReportIndividualTestCases), "cfloat<16,1>", "log");
	
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_arithmetic_exception& err) {
	std::cerr << "Uncaught cfloat arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_internal_exception& err) {
	std::cerr << "Uncaught cfloat internal exception: " << err.what() << std::endl;
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
