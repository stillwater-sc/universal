// function_truncate.cpp: test suite runner for truncation functions trunc, round, floor, and ceil
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
// use default number system library configuration
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

namespace sw { namespace universal {

	template<size_t nbits, size_t es>
	int VerifyFloor(bool reportTestCases) {
		using namespace sw::universal;
		constexpr size_t NR_VALUES = (1 << nbits);
		int nrOfFailedTestCases = 0;

		posit<nbits, es> p;
			for (size_t i = 0; i < NR_VALUES; ++i) {
				p.setbits(i);
				long l1 = long(sw::universal::floor(p));
				// generate the reference
				float f = float(p);
				if (!std::isfinite(f)) continue;
				long l2 = long(std::floor(f));
				if (l1 != l2) {
				++nrOfFailedTestCases;
				if (reportTestCases) 
					ReportOneInputFunctionError("floor", "floor",
						p, posit<nbits, es>(l1), posit<nbits, es>(l2));
			}
		}
		return nrOfFailedTestCases;
	}

	template<size_t nbits, size_t es>
	int VerifyCeil(bool reportTestCases) {
		using namespace sw::universal;
		constexpr size_t NR_VALUES = (1 << nbits);
		int nrOfFailedTestCases = 0;

		posit<nbits, es> p;
			for (size_t i = 0; i < NR_VALUES; ++i) {
				p.setbits(i);
				long l1 = long(sw::universal::ceil(p));
				// generate the reference
				float f = float(p);
				if (!std::isfinite(f)) continue;
				long l2 = long(std::ceil(f));
				if (l1 != l2) {
				++nrOfFailedTestCases;
				if (reportTestCases)
					ReportOneInputFunctionError("ceil", "ceil",
						p, posit<nbits, es>(l1), posit<nbits, es>(l2));
			}
		}
		return nrOfFailedTestCases;
	}

	template<size_t nbits, size_t es>
	int VerifyTrunc(bool reportTestCases) {
		using namespace sw::universal;
		constexpr size_t NR_VALUES = (1 << nbits);
		int nrOfFailedTestCases = 0;

		posit<nbits, es> p;
			for (size_t i = 0; i < NR_VALUES; ++i) {
				p.setbits(i);
				long l1 = long(sw::universal::trunc(p));
				// generate the reference
				float f = float(p);
				if (!std::isfinite(f)) continue;
				long l2 = long(std::trunc(f));
				if (l1 != l2) {
				++nrOfFailedTestCases;
				if (reportTestCases)
					ReportOneInputFunctionError("trunc", "trunc",
						p, posit<nbits, es>(l1), posit<nbits, es>(l2));
			}
		}
		return nrOfFailedTestCases;
	}

	template<size_t nbits, size_t es>
	int VerifyRound(bool reportTestCases) {
		using namespace sw::universal;
		constexpr size_t NR_VALUES = (1 << nbits);
		int nrOfFailedTestCases = 0;

		posit<nbits, es> p;
			for (size_t i = 0; i < NR_VALUES; ++i) {
				p.setbits(i);
				long l1 = long(sw::universal::round(p));
				// generate the reference
				float f = float(p);
				if (!std::isfinite(f)) continue;
				long l2 = long(std::round(f));
				if (l1 != l2) {
				++nrOfFailedTestCases;
				if (reportTestCases)
					ReportOneInputFunctionError("round", "round",
						p, posit<nbits, es>(l1), posit<nbits, es>(l2));
			}
		}
		return nrOfFailedTestCases;
	}

} }  // namespace sw::universal


int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "posit truncate function validation";
	std::string test_tag    = "truncate failed: ";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	nrOfFailedTestCases = ReportTestResult(VerifyTrunc<4, 0>(reportTestCases), "trunc", "posit<4,0>()");
	nrOfFailedTestCases = ReportTestResult(VerifyRound<4, 0>(reportTestCases), "round", "posit<4,0>()");
	nrOfFailedTestCases = ReportTestResult(VerifyFloor<4, 0>(reportTestCases), "floor", "posit<4,0>()");
	nrOfFailedTestCases = ReportTestResult(VerifyCeil<4, 0>(reportTestCases), "ceil", "posit<4,0>()");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases = ReportTestResult(VerifyTrunc<6, 2>(reportTestCases), "trunc", "posit<6,2>()");
	nrOfFailedTestCases = ReportTestResult(VerifyRound<6, 2>(reportTestCases), "round", "posit<6,2>()");
	nrOfFailedTestCases = ReportTestResult(VerifyFloor<6, 2>(reportTestCases), "floor", "posit<6,2>()");
	nrOfFailedTestCases = ReportTestResult(VerifyCeil<6, 2>(reportTestCases), "ceil", "posit<6,2>()");


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
