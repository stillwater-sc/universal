// addition.cpp: test suite runner for addition on arbitrary reals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// minimum set of include files to reflect source code dependencies
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

// generate specific test case that you can trace with the trace conditions in areal.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	sw::universal::areal<nbits, es> a, b, sum, ref;
	a = _a;
	b = _b;
	sum = a + b;
	// generate the reference
	Ty reference = _a + _b;
	ref = reference;

	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " + " << std::setw(nbits) << _b << " = " << std::setw(nbits) << reference << std::endl;
	std::cout << a << " + " << b << " = " << sum << " (reference: " << ref << ")   ";
	std::cout << to_binary(a, true) << " + " << to_binary(b, true) << " = " << to_binary(sum, true) << " (reference: " << to_binary(ref, true) << ")   ";
	std::cout << (ref == sum ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc > 1) {
		for (int i = 0; i < argc; ++i) {
			std::cout << argv[i] << ' ';
		}
		std::cout << std::endl;
	}
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, 8, double>(INFINITY, INFINITY);
	GenerateTestCase<8, 4, float>(0.5f, -0.5f);

	// manual exhaustive test
	//nrOfFailedTestCases += ReportTestResult(VerifyAddition< areal<8, 2, uint8_t> >("Manual Testing", true), "areal<8,2,uint8_t>", "addition");

	
	nrOfFailedTestCases = 0;

#else
	cout << "Arbitrary Real addition validation" << endl;

	bool bReportIndividualTestCases = false;
	std::string tag = "Addition failed: ";

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 2>(tag, bReportIndividualTestCases), "areal<8,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 4>(tag, bReportIndividualTestCases), "areal<8,4>", "addition");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<10, 4>(tag, bReportIndividualTestCases), "areal<10,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<16, 8>(tag, bReportIndividualTestCases), "areal<16,8>", "addition");
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_divide_by_zero& err) {
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
