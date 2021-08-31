// pow.cpp: test suite runner for pow function
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default library configuration
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/fixpnt/manipulators.hpp>
#include <universal/verification/fixpnt_math_test_suite.hpp>

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::string tag = "fixpnt pow() function failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

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
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<2, 0>("Manual Testing", true), "fixpnt<2,0>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<3, 0>("Manual Testing", true), "fixpnt<3,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<3, 1>("Manual Testing", true), "fixpnt<3,1>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<4, 0>("Manual Testing", true), "fixpnt<4,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<4, 1>("Manual Testing", true), "fixpnt<4,1>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<5, 0>("Manual Testing", true), "fixpnt<5,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<5, 1>("Manual Testing", true), "fixpnt<5,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<5, 2>("Manual Testing", true), "fixpnt<5,2>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 0>("Manual Testing", true), "fixpnt<8,0>", "pow");	
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 1>("Manual Testing", true), "fixpnt<8,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 4>("Manual Testing", true), "fixpnt<8,4>", "pow");

	//nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<16, 1>("Manual Testing", true), "fixpnt<16,1>", "pow");

#else
	bool bReportIndividualTestCases = false;

	std::cout << "Integer power function\n";
	int a = 2;
	unsigned b = 32;
	std::cout << "2 ^ 32   = " << ipow(a, b) << '\n';
	std::cout << "2 ^ 32   = " << fastipow(a, uint8_t(b)) << '\n';

	int64_t c = 1024;
	uint8_t d = 2;
	std::cout << "1024 ^ 2 = " << ipow(c, d) << '\n';
	std::cout << "1M ^ 2   = " << ipow(ipow(c, d), d) << '\n';

	std::cout << "fixpnt pow() function validation\n";

	using FixedPoint = fixpnt<8, 2, Saturating, uint8_t>;
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< FixedPoint >(bReportIndividualTestCases), type_tag<FixedPoint>(), "pow");

#if STRESS_TESTING
	
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
//catch (const sw::universal::fixpnt_quire_exception& err) {
//	std::cerr << "Uncaught fixpnt quire exception: " << err.what() << std::endl;
//	return EXIT_FAILURE;
//}
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
