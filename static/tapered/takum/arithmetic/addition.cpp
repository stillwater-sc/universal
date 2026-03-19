// addition.cpp: exhaustive test suite for takum addition
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/takum/takum.hpp>
#include <universal/verification/test_suite.hpp>

// Exhaustive verification of addition for small takum configurations.
// For each pair (a, b), compute a + b via the takum type and via double,
// then re-encode the double result and check that the bit patterns match.
template<unsigned nbits>
int VerifyAddition(bool reportTestCases) {
	using namespace sw::universal;
	using Takum = takum<nbits>;
	int nrOfFailedTestCases = 0;
	const unsigned NR_VALUES = (1u << nbits);

	for (unsigned i = 0; i < NR_VALUES; ++i) {
		Takum a;
		a.setbits(i);
		if (a.isnar()) continue;

		for (unsigned j = 0; j < NR_VALUES; ++j) {
			Takum b;
			b.setbits(j);
			if (b.isnar()) continue;

			Takum result = a + b;
			double da = double(a);
			double db = double(b);
			double dsum = da + db;
			Takum expected(dsum);

			if (result.raw_bits() != expected.raw_bits()) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cerr << "FAIL: " << da << " + " << db
					          << " = " << double(result) << " (got " << to_binary(result)
					          << "), expected " << double(expected) << " (" << to_binary(expected) << ")\n";
				}
			}
		}
	}
	return nrOfFailedTestCases;
}

// Regression testing guards
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "takum addition validation";
	std::string test_tag    = "addition";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<6>(reportTestCases), "takum<6> addition", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8>(reportTestCases), "takum<8> addition", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<10>(reportTestCases), "takum<10> addition", test_tag);
#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
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
