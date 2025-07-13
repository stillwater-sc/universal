// subtraction.cpp: test suite runner for subtraction arithmetic on fixed-sized, arbitrary precision logarithmic number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
//#include <universal/verification/test_suite.hpp>    // there is a generic VerifySubtraction there: we need a trait to break template match
// in the mean time: explicity bring in the dependencies to get the test running
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_case.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

	template<typename LnsType, std::enable_if_t<is_lns<LnsType>, bool> = true>
	int VerifySubtraction(bool reportTestCases) {
		constexpr size_t nbits = LnsType::nbits;
		//constexpr size_t rbits = LnsType::rbits;
		//constexpr Behavior behavior = LnsType::behavior;
		//using bt = typename LnsType::BlockType;
		constexpr size_t NR_ENCODINGS = (1ull << nbits);

		int nrOfFailedTestCases = 0;

		LnsType a{}, b{}, c{}, cref{};
		for (size_t i = 0; i < NR_ENCODINGS; ++i) {
			a.setbits(i);
			double da = double(a);
			for (size_t j = 0; j < NR_ENCODINGS; ++j) {
				b.setbits(j);
				double db = double(b);

				double ref = da - db;
				if (reportTestCases && !isInRange<LnsType>(ref)) {
					std::cerr << da << " * " << db << " = " << ref << " which is not in range " << range(a) << '\n';
				}
				c = a - b;
				cref = ref;
				//std::cout << "ref  : " << to_binary(ref) << " : " << ref << '\n';
				//std::cout << "cref : " << std::setw(68) << to_binary(cref) << " : " << cref << '\n';
				if (c != cref) {
					if (c.isnan() && cref.isnan()) continue; // NaN non-equivalence
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

} }


// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
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

	std::string test_suite  = "lns subtraction validation";
	std::string test_tag    = "subtraction";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using LNS4_1_sat = lns<4, 1,std::uint8_t>;
	using LNS4_2_sat = lns<4, 2,std::uint8_t>;
	using LNS5_2_sat = lns<5, 2,std::uint8_t>;
	using LNS8_3_sat = lns<8, 3,std::uint8_t>;
	using LNS9_4_sat = lns<9, 4,std::uint8_t>;
	using LNS16_5_sat = lns<16, 5,std::uint16_t>;

	// generate individual testcases to hand trace/debug
	TestCase< LNS16_5_sat, double>(TestCaseOperator::SUB, INFINITY, INFINITY);
	TestCase< LNS8_3_sat, float>(TestCaseOperator::SUB, 0.5f, -0.5f);

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<LNS8_3_sat>(reportTestCases), "lns<8,2,uint8_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	using LNS4_1_sat = lns<4, 1,std::uint8_t>;
	using LNS4_2_sat = lns<4, 2,std::uint8_t>;
	using LNS5_2_sat = lns<5, 2,std::uint8_t>;
	using LNS8_3_sat = lns<8, 3,std::uint8_t>;

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<LNS4_1_sat>(reportTestCases), "lns<4,1, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<LNS4_2_sat>(reportTestCases), "lns<4,2, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<LNS5_2_sat>(reportTestCases), "lns<5,2, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<LNS8_3_sat>(reportTestCases), "lns<8,3, uint8_t>", test_tag);

#endif

#if REGRESSION_LEVEL_2
	using LNS9_4_sat = lns<9, 4,std::uint8_t>;
	using LNS10_4_sat = lns<10, 4,std::uint8_t>;

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<LNS9_4_sat>(reportTestCases), "lns<9,4, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<LNS10_4_sat>(reportTestCases), "lns<10,4, uint8_t>", test_tag);

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
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
