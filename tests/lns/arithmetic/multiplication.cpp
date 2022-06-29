// multiplication.cpp: test suite runner for multiplication arithmetic of fixed-sized, arbitrary precision logarithmic number system
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	//template<typename LnsType,
	//	std::enable_if_t<is_lns<LnsType>, LnsType> = 0
	//>
	template<typename LnsType>
	int ValidateMultiplication(bool reportTestCases) {
		constexpr size_t nbits = LnsType::nbits;
		constexpr size_t NR_ENCODINGS = (1ull << nbits);

		int nrOfFailedTestCases = 0;

		LnsType a, b, c, cref;
		for (size_t i = 0; i < NR_ENCODINGS; ++i) {
			a.setbits(i);
			double da = double(a);
			for (size_t j = 0; j < NR_ENCODINGS; ++j) {
				b.setbits(j);
				double db = double(b);

				double ref = da * db;
				c = a * b;
				cref = ref;
				if (c != cref) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "*", a, b, c, cref);
				}
				else {
					// if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, c, ref);
				}
			}
		}
		return nrOfFailedTestCases;
	}

} }


// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "lns multiplication validation";
	std::string test_tag    = "multiplication";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);

#if MANUAL_TESTING

	using LNS4_2 = lns<4, 2, std::uint8_t>;
	using LNS8_2 = lns<8, 2, std::uint8_t>;
	using LNS16_5 = lns<16, 5, std::uint16_t>;


	// generate individual testcases to hand trace/debug
//	TestCase<LNS16_5, double>(TestCaseOperator::MUL, INFINITY, INFINITY);
//	TestCase<LNS8_2, float>(TestCaseOperator::MUL, 0.5f, -0.5f);


	{
		constexpr double e = 2.71828182845904523536;
		lns<16, 5, std::uint16_t> a, b, c;
		a = 0.5; std::cout << a << '\n';
		a = e; std::cout << a << '\n';
		b = 1.0 / e;
		c = a * b;
		std::cout << double(c) << '\n';

		std::cout << "-----\n";
		a = 1.0f; b = 2.0f;
		c = a * b;
		std::cout << float(c) << '\n';
		a = 0.0f; b = 2.0f;
		c = a * b;
		std::cout << float(c) << '\n';
		a = 3.5f; b = 0.0f;
		c = a * b;
		std::cout << float(c) << '\n';
	}
	return 0;

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<LNS4_2>(reportTestCases), "lns<4,2>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
	using LNS8_2 = lns<8, 2, std::uint8_t>;

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<LNS8_2>(reportTestCases), "lns<8,2>", test_tag);
#endif

#if REGRESSION_LEVEL_2
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
