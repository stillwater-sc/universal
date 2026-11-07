// division.cpp: test suite runner for division arithmetic on SORNs
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/sorn/sorn.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// the special cases of SORN division; enclosure.cpp checks that every quotient encloses x / y
	template<typename SornType>
	int VerifyDivisionSpecialCases(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		auto check = [&](const SornType& got, const std::string& expected, const char* label) {
			std::stringstream s;
			s << got;
			if (s.str() != expected) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << label << " = " << s.str() << ", expected " << expected << '\n';
			}
		};
		const SornType a(1.25);                          // (1,1.5]
		const SornType b(0.75);                          // (0.5,1]
		const SornType straddle = SornType(-0.25) + SornType(0.25);   // [-0.5,0.5]
		const SornType zero(0);
		const SornType minpos(SpecificValue::minpos);    // (0,0.5]
		const SornType minneg(SpecificValue::minneg);    // [-0.5,0)

		check(a / b,        "(1,3]",       "(1,1.5] / (0.5,1]");
		check(a / -b,       "[-3,-1)",     "(1,1.5] / [-1,-0.5)");
		check(a / straddle, "[-inf,inf]",  "divisor straddling zero");
		check(a / zero,     "nan",         "division by the exact zero");
		check(zero / a,     "0",           "zero dividend");
		check(zero / zero,  "nan",         "0 / 0");
		check(a / minpos,   "(2,inf]",     "divisor with an open zero bound");
		check(a / minneg,   "[-inf,-2)",   "negative divisor with an open zero bound");
		check(a / 2,        "(0.5,1]",     "sorn / int");
		check(a / -2.0,     "[-1,-0.5)",   "sorn / negative double");
		check(a / 0.0f,     "nan",         "sorn / 0.0f");
		check(1 / a,        "(0.5,1]",     "int / sorn");
		check(3.0 / b,      "(2.5,inf]",   "double / sorn");   // [3, 6) saturates past 4
		return nrOfFailedTestCases;
	}

}}  // namespace sw::universal

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "sorn division validation";
	std::string test_tag    = "division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using LinSorn = sorn<0, 4, 8>;   // linear set up SORN datatype

#if MANUAL_TESTING

	const LinSorn a(1.25), b(0.75);
	std::cout << a << " / " << b << " = " << (a / b) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionSpecialCases<LinSorn>(reportTestCases), "sorn<0,4,8>", test_tag);
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
