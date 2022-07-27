// addition.cpp: test suite runner for addition arithmetic on fixed-sized, arbitrary precision logarithmic number system
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// minimum set of include files to reflect source code dependencies
#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	//template<typename LnsType,
	//	std::enable_if_t<is_lns<LnsType>, LnsType> = 0
	//>
	template<typename LnsType>
	int ValidateAddition(bool reportTestCases) {
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

				double ref = da + db;
				c = a + b;
				cref = ref;
				//std::cout << "ref  : " << to_binary(ref) << " : " << ref << '\n';
				//std::cout << "cref : " << std::setw(68) << to_binary(cref) << " : " << cref << '\n';
				if (c != cref) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "+", a, b, c, cref);
					//std::cout << "ref  : " << to_binary(ref) << " : " << ref << '\n';
					//std::cout << "cref : " << std::setw(68) << to_binary(cref) << " : " << cref << '\n';
				}
				else {
					if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, c, ref);
				}
				if (nrOfFailedTestCases > 0) return 25;
			}
		}
		return nrOfFailedTestCases;
	}

} }  // namespace sw::universal


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

	std::string test_suite  = "lns addition validation";
	std::string test_tag    = "addition";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using LNS4_1 = lns<4, 1, Saturating, std::uint8_t>;
	using LNS4_2 = lns<4, 2, Saturating, std::uint8_t>;
	using LNS5_2 = lns<5, 2, Saturating, std::uint8_t>;
	using LNS8_3 = lns<8, 3, Saturating, std::uint8_t>;
	using LNS9_4 = lns<9, 4, Saturating, std::uint8_t>;
	using LNS16_5 = lns<16, 5, Saturating, std::uint16_t>;

	// generate individual testcases to hand trace/debug
	TestCase< LNS16_5, double>(TestCaseOperator::ADD, INFINITY, INFINITY);
	TestCase< LNS8_3, float>(TestCaseOperator::ADD, 0.5f, -0.5f);

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<LNS4_2>(reportTestCases), "lns<4,2,uint8_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
	using LNS4_1 = lns<4, 1, Saturating, std::uint8_t>;
	using LNS4_2 = lns<4, 2, Saturating, std::uint8_t>;
	using LNS5_2 = lns<5, 2, Saturating, std::uint8_t>;
	using LNS8_3 = lns<8, 3, Saturating, std::uint8_t>;
	using LNS9_4 = lns<9, 4, Saturating, std::uint8_t>;
	using LNS10_4 = lns<10, 4, Saturating, std::uint8_t>;

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<LNS4_1>(reportTestCases), "lns<4,1,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<LNS4_2>(reportTestCases), "lns<4,2,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<LNS5_2>(reportTestCases), "lns<5,2,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<LNS8_3>(reportTestCases), "lns<8,3,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<LNS9_4>(reportTestCases), "lns<9,4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<LNS10_4>(reportTestCases), "lns<10,4,uint8_t>", test_tag);
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
