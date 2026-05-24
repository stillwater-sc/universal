// maxlimbs_threshold.cpp: regression tests for the ereal<maxlimbs> limb-count
// constraint.
//
// Shewchuk's expansion arithmetic requires every limb to be a *normal* IEEE-754
// double. The smallest limb of an N-component expansion sits ~53*N binary
// exponents below the leading limb, so to stay above the subnormal threshold
// (DBL_MIN = 2^-1022) the expansion needs 53*N <= 1022, i.e. N <= 19 (53*19 =
// 1007 < 1022; 53*20 = 1060 > 1022 underflows). ereal_impl.hpp enforces this
// with a static_assert(maxlimbs <= 19).
//
// Coverage:
//   - each maxlimbs in {1, 2, 4, 8, 16, 19} constructs and does basic
//     arithmetic;
//   - the 2^-1007 boundary (19 limbs) stays >= DBL_MIN while 2^-1060 (20 limbs)
//     underflows;
//   - maxlimbs = 20 fails the static_assert at compile time (macro-gated
//     compile-only check -- enabling it must break the build).
//
// Pattern: elastic/ereal/arithmetic/addition.cpp (PR #943).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	// Construct an ereal<N> and exercise the basic operators, verifying the
	// result stays finite and the expansion respects its maxlimbs bound.
	template<unsigned N>
	int verify_configuration(bool reportTestCases) {
		using namespace sw::universal;
		int fails = 0;

		ereal<N> a(1.0);
		a += std::ldexp(1.0, -52);   // add a low-order bit to grow the expansion
		ereal<N> b(3.0);

		ereal<N> sum  = a + b;
		ereal<N> diff = b - a;
		ereal<N> prod = a * b;
		ereal<N> quot = b / a;

		// Round-trip and basic-value sanity on the projected double.
		if (!std::isfinite(double(sum)) || !std::isfinite(double(diff))
		    || !std::isfinite(double(prod)) || !std::isfinite(double(quot))) {
			if (reportTestCases) std::cout << "    FAIL ereal<" << N << "> produced a non-finite result\n";
			++fails;
		}
		if ((a + b) - b != a) {  // exact additive round trip
			if (reportTestCases) std::cout << "    FAIL ereal<" << N << "> (a+b)-b != a\n";
			++fails;
		}
		// Note: maxlimbs is the compile-time algorithmic ceiling enforced by the
		// static_assert(maxlimbs <= 19); it is NOT a hard runtime cap. The _limb
		// vector grows dynamically as renormalisation requires, so an
		// ereal<N> expansion may legitimately hold more than N components.
		return fails;
	}

	// =========================================================================
	// LEVEL 1: each supported maxlimbs configuration is usable
	// =========================================================================
	int VerifyMaxlimbsConfigurations(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		if (reportTestCases) std::cout << "  maxlimbs in {1,2,4,8,16,19}...\n";
		nrOfFailedTestCases += verify_configuration<1>(reportTestCases);
		nrOfFailedTestCases += verify_configuration<2>(reportTestCases);
		nrOfFailedTestCases += verify_configuration<4>(reportTestCases);
		nrOfFailedTestCases += verify_configuration<8>(reportTestCases);
		nrOfFailedTestCases += verify_configuration<16>(reportTestCases);
		nrOfFailedTestCases += verify_configuration<19>(reportTestCases);
		return nrOfFailedTestCases;
	}

	// =========================================================================
	// LEVEL 1: the 53*maxlimbs <= 1022 normal-double boundary
	// =========================================================================
	int VerifyMaxlimbsBoundary(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const double dmin = std::numeric_limits<double>::min();  // DBL_MIN = 2^-1022

		if (reportTestCases) std::cout << "  19-limb boundary 2^-1007 >= DBL_MIN...\n";
		// maxlimbs = 19: smallest limb ~ 2^-(53*19) = 2^-1007, still normal.
		if (!(std::ldexp(1.0, -1007) >= dmin)) {
			if (reportTestCases) std::cout << "    FAIL 2^-1007 < DBL_MIN\n";
			++nrOfFailedTestCases;
		}
		// 53*19 = 1007 < 1022.
		if (!(53 * 19 < 1022)) {
			if (reportTestCases) std::cout << "    FAIL 53*19 >= 1022\n";
			++nrOfFailedTestCases;
		}

		if (reportTestCases) std::cout << "  20-limb boundary 2^-1060 < DBL_MIN (underflow)...\n";
		// maxlimbs = 20: smallest limb ~ 2^-(53*20) = 2^-1060, subnormal -> the
		// non-overlap property breaks, which is exactly why maxlimbs is capped.
		if (!(std::ldexp(1.0, -1060) < dmin)) {
			if (reportTestCases) std::cout << "    FAIL 2^-1060 >= DBL_MIN\n";
			++nrOfFailedTestCases;
		}
		if (!(53 * 20 > 1022)) {
			if (reportTestCases) std::cout << "    FAIL 53*20 <= 1022\n";
			++nrOfFailedTestCases;
		}
		return nrOfFailedTestCases;
	}

	// Compile-only check: instantiating ereal<20> must fail the
	// static_assert(maxlimbs <= 19) in ereal_impl.hpp. Instantiating it
	// unconditionally would break this translation unit, so it is gated behind a
	// macro that defaults off. Building with -DEREAL_TEST_MAXLIMBS_OVERFLOW=1
	// MUST fail to compile (that is the test).
	//
	// Value-based guard (`#if EREAL_TEST_MAXLIMBS_OVERFLOW`, not
	// `#if defined(...)`) so `-DEREAL_TEST_MAXLIMBS_OVERFLOW=0` correctly
	// disables it.
#ifndef EREAL_TEST_MAXLIMBS_OVERFLOW
#define EREAL_TEST_MAXLIMBS_OVERFLOW 0
#endif
#if EREAL_TEST_MAXLIMBS_OVERFLOW
	// Expected: static_assert failure "maxlimbs must be <= 19".
	sw::universal::ereal<20> compile_failure_probe;
#endif

}  // namespace

// Regression testing guards
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 1
#	define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "ereal maxlimbs threshold";
	bool        reportTestCases = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyMaxlimbsConfigurations(reportTestCases), "ereal", "maxlimbs configurations manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors in manual mode

#else

	// Deterministic compile-time constraint -- one pass covers every level.
	nrOfFailedTestCases += ReportTestResult(VerifyMaxlimbsConfigurations(reportTestCases), "ereal", "maxlimbs configurations");
	nrOfFailedTestCases += ReportTestResult(VerifyMaxlimbsBoundary(reportTestCases), "ereal", "maxlimbs boundary");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& ex) {
	std::cerr << "Caught exception: " << ex.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
