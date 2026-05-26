// classify.cpp: test suite runner for classification functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>
#include <limits>
#include <random>

namespace {
	using namespace sw::universal;

		// Verify isfinite function
		template<typename Real>
		int VerifyIsFinite(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: isfinite(2.0) == true
			Real x(2.0);
			if (!isfinite(x)) {
				if (reportTestCases) std::cout << "    FAIL isfinite(2.0) != true\n";
				++nrOfFailedTestCases;
			}

			// Test: isfinite(-1.0) == true
			x = -1.0;
			if (!isfinite(x)) {
				if (reportTestCases) std::cout << "    FAIL isfinite(-1.0) != true\n";
				++nrOfFailedTestCases;
			}

			// Test: isfinite(0.0) == true
			Real zero(0.0);
			if (!isfinite(zero)) {
				if (reportTestCases) std::cout << "    FAIL isfinite(0.0) != true\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify isnan function
		template<typename Real>
		int VerifyIsNan(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: isnan(2.0) == false (normal values are not NaN)
			Real x(2.0);
			if (isnan(x)) {
				if (reportTestCases) std::cout << "    FAIL isnan(2.0) != false\n";
				++nrOfFailedTestCases;
			}

			// Special value: isnan(NaN) == true
			Real nan(std::numeric_limits<double>::quiet_NaN());
			if (!isnan(nan)) {
				if (reportTestCases) std::cout << "    FAIL isnan(NaN) != true\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify isinf function
		template<typename Real>
		int VerifyIsInf(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: isinf(2.0) == false (normal values are not infinite)
			Real x(2.0);
			if (isinf(x)) {
				if (reportTestCases) std::cout << "    FAIL isinf(2.0) != false\n";
				++nrOfFailedTestCases;
			}

			// Special values: isinf(+/-Inf) == true and isfinite(Inf) == false
			Real pinf(std::numeric_limits<double>::infinity());
			Real ninf(-std::numeric_limits<double>::infinity());
			if (!isinf(pinf) || !isinf(ninf)) {
				if (reportTestCases) std::cout << "    FAIL isinf(+/-Inf) != true\n";
				++nrOfFailedTestCases;
			}
			if (isfinite(pinf)) {
				if (reportTestCases) std::cout << "    FAIL isfinite(Inf) != false\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify isnormal function
		template<typename Real>
		int VerifyIsNormal(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: isnormal(2.0) == true
			Real x(2.0);
			if (!isnormal(x)) {
				if (reportTestCases) std::cout << "    FAIL isnormal(2.0) != true\n";
				++nrOfFailedTestCases;
			}

			// Test: isnormal(-1.0) == true
			x = -1.0;
			if (!isnormal(x)) {
				if (reportTestCases) std::cout << "    FAIL isnormal(-1.0) != true\n";
				++nrOfFailedTestCases;
			}

			// Test: isnormal(0.0) == false (zero is not normal)
			Real zero(0.0);
			if (isnormal(zero)) {
				if (reportTestCases) std::cout << "    FAIL isnormal(0.0) != false\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify signbit function
		template<typename Real>
		int VerifySignBit(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: signbit(2.0) == false
			Real pos(2.0);
			if (signbit(pos)) {
				if (reportTestCases) std::cout << "    FAIL signbit(2.0) != false\n";
				++nrOfFailedTestCases;
			}

			// Test: signbit(-1.0) == true
			Real neg(-1.0);
			if (!signbit(neg)) {
				if (reportTestCases) std::cout << "    FAIL signbit(-1.0) != true\n";
				++nrOfFailedTestCases;
			}

			// Test: signbit(0.0) == false
			Real zero(0.0);
			if (signbit(zero)) {
				if (reportTestCases) std::cout << "    FAIL signbit(0.0) != false\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify fpclassify function
		template<typename Real>
		int VerifyFpClassify(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: fpclassify(2.0) == FP_NORMAL
			Real normal(2.0);
			if (fpclassify(normal) != FP_NORMAL) {
				if (reportTestCases) std::cout << "    FAIL fpclassify(2.0) != FP_NORMAL\n";
				++nrOfFailedTestCases;
			}

			// Test: fpclassify(0.0) == FP_ZERO
			Real zero(0.0);
			if (fpclassify(zero) != FP_ZERO) {
				if (reportTestCases) std::cout << "    FAIL fpclassify(0.0) != FP_ZERO\n";
				++nrOfFailedTestCases;
			}

			// Special values: fpclassify(Inf) == FP_INFINITE, fpclassify(NaN) == FP_NAN
			Real inf(std::numeric_limits<double>::infinity());
			if (fpclassify(inf) != FP_INFINITE) {
				if (reportTestCases) std::cout << "    FAIL fpclassify(Inf) != FP_INFINITE\n";
				++nrOfFailedTestCases;
			}
			Real nan(std::numeric_limits<double>::quiet_NaN());
			if (fpclassify(nan) != FP_NAN) {
				if (reportTestCases) std::cout << "    FAIL fpclassify(NaN) != FP_NAN\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Property fuzzer: over random finite values, the classification
		// predicates are mutually consistent and signbit matches the projection.
		template<typename Real>
		int VerifyClassifyFuzz(bool reportTestCases, unsigned nrIterations) {
			int nrOfFailedTestCases = 0;
			std::mt19937_64 rng(0xC1A55'1FFEULL);
			std::uniform_real_distribution<double> dist(-1.0e6, 1.0e6);
			for (unsigned i = 0; i < nrIterations; ++i) {
				double d = dist(rng);
				Real x(d);
				// finite normal/zero inputs: finite and not nan/inf
				if (!isfinite(x) || isnan(x) || isinf(x)) {
					if (reportTestCases) std::cout << "    FAIL classify consistency at d=" << d << '\n';
					++nrOfFailedTestCases;
				}
				// signbit agrees with the double projection
				if (signbit(x) != std::signbit(d)) {
					if (reportTestCases) std::cout << "    FAIL signbit mismatch at d=" << d << '\n';
					++nrOfFailedTestCases;
				}
				// exactly one of {normal, zero} holds for a finite value
				bool normal = isnormal(x);
				bool zero = (d == 0.0);
				if (normal == zero) {
					if (reportTestCases) std::cout << "    FAIL normal/zero partition at d=" << d << '\n';
					++nrOfFailedTestCases;
				}
			}
			return nrOfFailedTestCases;
		}

}  // anonymous namespace

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal mathlib classification function validation";
	std::string test_tag    = "classification";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of classification functions:\n";
	ereal<> x(2.0);
	std::cout << "isfinite(2.0) = " << isfinite(x) << " (expected: 1)\n";
	std::cout << "isnan(2.0) = " << isnan(x) << " (expected: 0)\n";
	std::cout << "isnormal(2.0) = " << isnormal(x) << " (expected: 1)\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 1 functions: classification
	test_tag = "isfinite";
	nrOfFailedTestCases += ReportTestResult(VerifyIsFinite<ereal<>>(reportTestCases), "isfinite(ereal)", test_tag);

	test_tag = "isnan";
	nrOfFailedTestCases += ReportTestResult(VerifyIsNan<ereal<>>(reportTestCases), "isnan(ereal)", test_tag);

	test_tag = "isinf";
	nrOfFailedTestCases += ReportTestResult(VerifyIsInf<ereal<>>(reportTestCases), "isinf(ereal)", test_tag);

	test_tag = "isnormal";
	nrOfFailedTestCases += ReportTestResult(VerifyIsNormal<ereal<>>(reportTestCases), "isnormal(ereal)", test_tag);

	test_tag = "signbit";
	nrOfFailedTestCases += ReportTestResult(VerifySignBit<ereal<>>(reportTestCases), "signbit(ereal)", test_tag);

	test_tag = "fpclassify";
	nrOfFailedTestCases += ReportTestResult(VerifyFpClassify<ereal<>>(reportTestCases), "fpclassify(ereal)", test_tag);
#endif

#if REGRESSION_LEVEL_2
	test_tag = "classify fuzz";
	nrOfFailedTestCases += ReportTestResult(VerifyClassifyFuzz<ereal<>>(reportTestCases, 50), "classify(ereal) fuzz", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// Future: Edge cases
#endif

#if REGRESSION_LEVEL_4
	// Future: Stress tests
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
