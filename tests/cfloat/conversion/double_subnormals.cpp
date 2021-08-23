// double_subnormals.cpp: test suite runner for conversion tests of double subnormals to classic cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/number/cfloat/manipulators.hpp>  // for subnormals and color_print
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	subnormals<cfloat<8, 2, uint8_t>>();  // 1 block
	subnormals<cfloat<16, 5, uint8_t>>(); // 2 blocks
	subnormals<cfloat<32, 8, uint8_t>>(); // 4 blocks
	subnormals<cfloat<48, 11, uint16_t>>(); // 3 blocks
	subnormals<cfloat<64, 11, uint16_t>>(); // 4 blocks
	subnormals<cfloat<80, 11, uint16_t>>(); // 5 blocks
	
	nrOfFailedTestCases = 0;

#else
	cout << "subnormal validation" << endl;

	bool bReportIndividualTestCases = false;
	std::string tag = "double subnormal conversion failed: ";

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 2>(tag, bReportIndividualTestCases), "cfloat<8,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 4>(tag, bReportIndividualTestCases), "cfloat<8,4>", "addition");

#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_divide_by_zero& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
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
