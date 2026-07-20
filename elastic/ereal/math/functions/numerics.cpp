// numerics.cpp: test suite runner for numeric support functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>
#include <climits>
#include <limits>
#include <random>

namespace {
	using namespace sw::universal;

		// Verify copysign function
		template<typename Real>
		int VerifyCopysign(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: copysign(5.0, -3.0) == -5.0
			Real x(5.0), y(-3.0), expected(-5.0);
			Real result = copysign(x, y);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL copysign(5.0, -3.0) != -5.0\n";
				++nrOfFailedTestCases;
			}

			// Test: copysign(-5.0, 3.0) == 5.0
			x = -5.0; y = 3.0; expected = 5.0;
			result = copysign(x, y);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL copysign(-5.0, 3.0) != 5.0\n";
				++nrOfFailedTestCases;
			}

			// Test: copysign(5.0, 3.0) == 5.0 (both positive)
			x = 5.0; y = 3.0; expected = 5.0;
			result = copysign(x, y);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL copysign(5.0, 3.0) != 5.0\n";
				++nrOfFailedTestCases;
			}

			// Test: copysign(-5.0, -3.0) == -5.0 (both negative)
			x = -5.0; y = -3.0; expected = -5.0;
			result = copysign(x, y);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL copysign(-5.0, -3.0) != -5.0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify ldexp function
		template<typename Real>
		int VerifyLdexp(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: ldexp(1.0, 3) == 8.0 (1.0 * 2^3)
			Real x(1.0), expected(8.0);
			Real result = ldexp(x, 3);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL ldexp(1.0, 3) != 8.0\n";
				++nrOfFailedTestCases;
			}

			// Test: ldexp(1.0, -2) == 0.25 (1.0 * 2^-2)
			expected = 0.25;
			result = ldexp(x, -2);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL ldexp(1.0, -2) != 0.25\n";
				++nrOfFailedTestCases;
			}

			// Test: ldexp(1.0, 0) == 1.0
			expected = 1.0;
			result = ldexp(x, 0);
			if (result != expected) {
				if (reportTestCases) std::cout << "    FAIL ldexp(1.0, 0) != 1.0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify frexp function
		template<typename Real>
		int VerifyFrexp(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: frexp(8.0) == (0.5, 4)  because 8.0 = 0.5 * 2^4
			Real x(8.0), expected_mantissa(0.5);
			int exp, expected_exp = 4;
			Real mantissa = frexp(x, &exp);

			if (mantissa != expected_mantissa) {
				if (reportTestCases) std::cout << "    FAIL frexp(8.0) mantissa != 0.5\n";
				++nrOfFailedTestCases;
			}
			if (exp != expected_exp) {
				if (reportTestCases) std::cout << "    FAIL frexp(8.0) exponent != 4\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify frexp/ldexp roundtrip
		template<typename Real>
		int VerifyFrexpLdexpRoundtrip(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: ldexp(frexp(x, &e), e) == x
			Real x(6.0);
			int exp;
			Real mantissa = frexp(x, &exp);
			Real reconstructed = ldexp(mantissa, exp);

			if (reconstructed != x) {
				if (reportTestCases) std::cout << "    FAIL ldexp(frexp(6.0)) != 6.0\n";
				++nrOfFailedTestCases;
			}

			// Test with different value
			x = 100.0;
			mantissa = frexp(x, &exp);
			reconstructed = ldexp(mantissa, exp);

			if (reconstructed != x) {
				if (reportTestCases) std::cout << "    FAIL ldexp(frexp(100.0)) != 100.0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify scalbn, logb, ilogb, fma against std:: (double-range values)
		template<typename Real>
		int VerifyScalbnLogbFma(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			for (double v : {1.0, 2.0, 3.0, 0.75, 0.5, 100.0, -8.0, 1023.5, 0.1}) {
				for (int n : {-5, 0, 3, 20}) {
					if (double(scalbn(Real(v), n)) != std::scalbn(v, n)) {
						if (reportTestCases) std::cout << "    FAIL scalbn(" << v << "," << n << ")\n";
						++nrOfFailedTestCases;
					}
				}
				if (double(logb(Real(v))) != std::logb(v) || ilogb(Real(v)) != std::ilogb(v)) {
					if (reportTestCases) std::cout << "    FAIL logb/ilogb(" << v << ") got "
					                               << double(logb(Real(v))) << "/" << ilogb(Real(v)) << "\n";
					++nrOfFailedTestCases;
				}
			}

			// fma is EXACT: (2^30+1)*(2^30-1) = 2^60 - 1, representable in ereal but
			// not in double. Build the expected value independently of the product
			// (scalbn/subtract), so this is not a tautology against x*y+z.
			Real x30(1073741825.0), y30(1073741823.0);      // 2^30 + 1, 2^30 - 1
			Real pm1 = scalbn(Real(1.0), 60) - Real(1.0);   // 2^60 - 1
			if (fma(x30, y30, Real(0.0)) != pm1) {
				if (reportTestCases) std::cout << "    FAIL fma not exact (2^60-1)\n";
				++nrOfFailedTestCases;
			}
			if (fma(x30, y30, Real(2.0)) != pm1 + Real(2.0)) {   // z folded in exactly
				if (reportTestCases) std::cout << "    FAIL fma z addend\n";
				++nrOfFailedTestCases;
			}

			// ilogb / logb special values
			Real zero(0.0), inf(std::numeric_limits<double>::infinity()), nan(std::numeric_limits<double>::quiet_NaN());
			if (ilogb(zero) != FP_ILOGB0 || ilogb(inf) != INT_MAX || ilogb(nan) != FP_ILOGBNAN) {
				if (reportTestCases) std::cout << "    FAIL ilogb special values\n";
				++nrOfFailedTestCases;
			}
			if (!logb(zero).isinf() || !logb(nan).isnan()) {
				if (reportTestCases) std::cout << "    FAIL logb special values\n";
				++nrOfFailedTestCases;
			}
			return nrOfFailedTestCases;
		}

		// Property fuzzer: copysign sign/magnitude and frexp/ldexp round trips
		// over random finite values.
		template<typename Real>
		int VerifyNumericsFuzz(bool reportTestCases, unsigned nrIterations) {
			int nrOfFailedTestCases = 0;
			std::mt19937_64 rng(0xC1A55'1FFEULL);
			std::uniform_real_distribution<double> dist(-1.0e6, 1.0e6);
			std::uniform_int_distribution<int> kdist(-40, 40);
			for (unsigned i = 0; i < nrIterations; ++i) {
				double d = dist(rng);
				Real x(d), y(dist(rng));
				// copysign: magnitude of x, sign of y
				Real cs = copysign(x, y);
				if (abs(cs) != abs(x) || signbit(cs) != signbit(y)) {
					if (reportTestCases) std::cout << "    FAIL copysign at d=" << d << '\n';
					++nrOfFailedTestCases;
				}
				// frexp/ldexp round trip: ldexp(frexp(x, &e), e) == x exactly
				int e;
				Real m = frexp(x, &e);
				if (ldexp(m, e) != x) {
					if (reportTestCases) std::cout << "    FAIL frexp/ldexp roundtrip at d=" << d << '\n';
					++nrOfFailedTestCases;
				}
				// ldexp by a power of two then its inverse is exact
				int k = kdist(rng);
				if (ldexp(ldexp(x, k), -k) != x) {
					if (reportTestCases) std::cout << "    FAIL ldexp shift roundtrip at d=" << d << " k=" << k << '\n';
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

	std::string test_suite  = "ereal mathlib numeric support function validation";
	std::string test_tag    = "numerics";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of numeric functions:\n";
	std::cout << "copysign(5.0, -3.0) = " << double(copysign(ereal<>(5.0), ereal<>(-3.0))) << " (expected: -5.0)\n";
	std::cout << "ldexp(1.0, 3) = " << double(ldexp(ereal<>(1.0), 3)) << " (expected: 8.0)\n";

	int exp;
	ereal<> mantissa = frexp(ereal<>(8.0), &exp);
	std::cout << "frexp(8.0) = (" << double(mantissa) << ", " << exp << ") (expected: (0.5, 4))\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 1 function: copysign
	test_tag = "copysign";
	nrOfFailedTestCases += ReportTestResult(VerifyCopysign<ereal<>>(reportTestCases), "copysign(ereal)", test_tag);

	// Phase 2 functions: ldexp, frexp
	test_tag = "ldexp";
	nrOfFailedTestCases += ReportTestResult(VerifyLdexp<ereal<>>(reportTestCases), "ldexp(ereal)", test_tag);

	test_tag = "frexp";
	nrOfFailedTestCases += ReportTestResult(VerifyFrexp<ereal<>>(reportTestCases), "frexp(ereal)", test_tag);

	test_tag = "frexp/ldexp roundtrip";
	nrOfFailedTestCases += ReportTestResult(VerifyFrexpLdexpRoundtrip<ereal<>>(reportTestCases), "frexp/ldexp roundtrip", test_tag);

	test_tag = "scalbn/logb/ilogb/fma";
	nrOfFailedTestCases += ReportTestResult(VerifyScalbnLogbFma<ereal<>>(reportTestCases), "scalbn/logb/ilogb/fma(ereal)", test_tag);
#endif

#if REGRESSION_LEVEL_2
	test_tag = "numerics fuzz";
	nrOfFailedTestCases += ReportTestResult(VerifyNumericsFuzz<ereal<>>(reportTestCases, 50), "numerics(ereal) fuzz", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// Future: Precision validation
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
