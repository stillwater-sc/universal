// truncate.cpp: test suite runner for truncation functions trunc, round, floor, and ceil
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system library configuration
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/cfloat_math_test_suite.hpp>

template<typename TestType>
int VerifyFloor(bool reportTestCases) {
	using namespace sw::universal;
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_VALUES = (1ull << nbits);
	int nrOfFailedTestCases = 0;

	TestType a;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		TestType l1 = floor(a);
		// generate the reference
		float f = float(a);         // we can stay with floats as the state space NR_VALUES is always going to be small to be practical (nbits < 16)
		float l2 = std::floor(f);
		if (l1 != l2) {
			if (a.isnan() || l1.isnan()) continue;
			std::cout << to_binary(a) << " : " << a << '\n';
//			std::cout << to_binary(l1) << " : " << l1 << '\n';
			std::cout << "floor(" << f << ") = " << l2 << " vs result " << l1 << '\n';
			++nrOfFailedTestCases;
			if (reportTestCases) ReportOneInputFunctionError("floor", "floor", a, TestType(l1), TestType(l2));
		}
	}
	return nrOfFailedTestCases;
}

template<typename TestType>
int VerifyCeil(bool reportTestCases) {
	using namespace sw::universal;
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_VALUES = (1ull << nbits);
	int nrOfFailedTestCases = 0;

	TestType a;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		TestType l1 = ceil(a);
		// generate the reference
		float f = float(a);
		float l2 = std::ceil(f);
		if (l1 != l2) {
			if (a.isnan() || l1.isnan()) continue;
			std::cout << to_binary(a) << " : " << a << '\n';
//			std::cout << to_binary(l1) << " : " << l1 << '\n';
			std::cout << "ceil(" << f << ") = " << l2 << " vs result " << l1 << '\n';
			++nrOfFailedTestCases;
			if (reportTestCases) ReportOneInputFunctionError("ceil", "ceil", a, TestType(l1), TestType(l2));
		}
	}
	return nrOfFailedTestCases;
}

#define MANUAL_TESTING 1

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat<> mathlib truncation validation";
	std::string test_tag    = "truncation";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	nrOfFailedTestCases = ReportTestResult(VerifyFloor< cfloat<8, 2, uint8_t> >(reportTestCases), "floor", "cfloat<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCeil < cfloat<8, 2, uint8_t> >(reportTestCases), "ceil ", "cfloat<8,2>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures in manual testing mode
#else

	// clang and gcc fail due to NaN != NaN relationship

	nrOfFailedTestCases = ReportTestResult(VerifyFloor< cfloat<8, 2, uint8_t> >(reportTestCases), "floor", "cfloat<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCeil < cfloat<8, 2, uint8_t> >(reportTestCases), "ceil ", "cfloat<8,2>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
