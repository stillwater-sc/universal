// division.cpp: test suite runner for division arithmetic on SORNs
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/sorn/sorn.hpp>
#include <universal/verification/test_suite.hpp>    // there is a generic VerifyDivision here

namespace sw { namespace universal {

	//template<typename SornType,
	//	std::enable_if_t<is_sorn<SornType>, SornType> = 0
	//>
	template<typename SornType>
	int VerifyDivision_(bool reportTestCases) {
		constexpr size_t nbits = SornType::nbits;
		// using value_type = typename SornType::value_type;

		SornType s;
		std::cerr << "SORN type : " << type_tag(s) << " : nbits = " << nbits << '\n';

		constexpr size_t NR_ENCODINGS = 16; // (1ull << nbits);

		int nrOfFailedTestCases = 0;

		SornType a, b, c, cref;
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
					if (c.isnan() && cref.isnan()) continue; // NaN non-equivalence
					++nrOfFailedTestCases;
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "+", a, b, c, cref);
					//std::cout << "ref  : " << to_binary(ref) << " : " << ref << '\n';
					//std::cout << "cref : " << std::setw(68) << to_binary(cref) << " : " << cref << '\n';
				}
				else {
					if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, c, ref);
				}
				if (nrOfFailedTestCases > 24) return nrOfFailedTestCases;
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

	std::string test_suite  = "sorn division validation";
	std::string test_tag    = "division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	
//	using FloatSorn = sorn<0, 4, 8 >; // set up SORN datatype (linear);

//	TestCase< FloatSorn, float>(TestCaseOperator::ADD, 0.5f, -0.5f);

//	nrOfFailedTestCases += ReportTestResult(VerifyDivision_<FloatSorn>(reportTestCases), "sorn<float>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
	using FloatSorn = sorn<0, 4, 8 >; // set up SORN datatype (linear);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<FloatSorn>(reportTestCases), "sorn<float>", test_tag);
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
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught unexpected universal internal exception: " << err.what() << std::endl;
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
