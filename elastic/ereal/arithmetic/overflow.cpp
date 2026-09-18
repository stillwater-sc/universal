// overflow.cpp: ereal arithmetic that overflows double range
//
// Two FINITE operands whose result lies beyond double's range came back as NaN in every
// limb -- DBL_MAX + DBL_MAX was [nan, nan], 1e200 * 1e200 was [nan, nan, nan] -- where
// IEEE double gives +/-inf (#1553). The special-value guards from #966 run on the
// operands, not the result, so the error-free transformation went ahead, rounded its
// leading term to inf, computed the error term as inf - inf = NaN, and renormalization
// spread it to every limb.
//
// The same fix found a second defect: division of a large DIVIDEND overflowed an
// intermediate even when the quotient is well inside the range, so DBL_MAX / 2 was NaN.
//
// Native double is the oracle throughout: for each expression the ereal result must be
// the same signed infinity, the same zero, or the same finite value double produces.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cfloat>
#include <climits>
#include <cmath>
#include <string>

#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	int expect_true(bool actual, const char* what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}


	using Real = ereal<>;

	// the result must be exactly what native double gives -- same infinity, same sign
	int expect_like_double(const Real& actual, double wanted, const char* what, bool reportTestCases) {
		const double got = double(actual);
		const bool same = (got == wanted) && (std::signbit(got) == std::signbit(wanted));
		const bool noNaN = !actual.isnan();
		if (same && noNaN) return 0;
		if (reportTestCases) {
			std::cout << "    FAIL " << what << ": got " << got << " (isnan=" << actual.isnan()
			          << "), native double gives " << wanted << '\n';
		}
		return 1;
	}

	int VerifyOverflowIsInfinite(bool reportTestCases) {
		int fails = 0;
		const Real max(DBL_MAX), big(1e200), two(2.0), tiny(1e-300);

		fails += expect_like_double(max + max, DBL_MAX + DBL_MAX, "DBL_MAX + DBL_MAX", reportTestCases);
		fails += expect_like_double(big * big, 1e200 * 1e200, "1e200 * 1e200", reportTestCases);
		fails += expect_like_double(max * two, DBL_MAX * 2.0, "DBL_MAX * 2", reportTestCases);
		fails += expect_like_double(big / tiny, 1e200 / 1e-300, "1e200 / 1e-300", reportTestCases);
		fails += expect_like_double(pown(two, 1024), std::ldexp(1.0, 1024), "2^1024", reportTestCases);
		fails += expect_like_double(pown(two, 2000), INFINITY, "2^2000", reportTestCases);

		// the in-place forms, including the double-operand overload
		{
			Real x(DBL_MAX);
			x += DBL_MAX;
			fails += expect_like_double(x, INFINITY, "x += DBL_MAX", reportTestCases);
			Real y(DBL_MAX);
			y *= 2.0;
			fails += expect_like_double(y, INFINITY, "y *= 2", reportTestCases);
		}

		// an overflowed result is a single infinite limb, not an expansion of NaNs
		{
			const Real r = max + max;
			fails += expect_true(r.isinf() && !r.isnan(), "overflow is infinite, not NaN", reportTestCases);
			fails += expect_true(r.limbs().size() == 1, "and occupies one limb", reportTestCases);
		}

		return fails;
	}

	int VerifyOverflowSign(bool reportTestCases) {
		int fails = 0;
		const Real max(DBL_MAX), min(-DBL_MAX), big(1e200), tiny(1e-300);

		fails += expect_like_double(min + min, -DBL_MAX + -DBL_MAX, "-DBL_MAX + -DBL_MAX", reportTestCases);
		fails += expect_like_double(max - min, DBL_MAX - -DBL_MAX, "DBL_MAX - -DBL_MAX", reportTestCases);
		fails += expect_like_double(min - max, -DBL_MAX - DBL_MAX, "-DBL_MAX - DBL_MAX", reportTestCases);
		fails += expect_like_double(max * Real(-2.0), DBL_MAX * -2.0, "DBL_MAX * -2", reportTestCases);
		fails += expect_like_double(Real(-1e200) * big, -1e200 * 1e200, "-1e200 * 1e200", reportTestCases);
		fails += expect_like_double(Real(-1e200) * Real(-1e200), 1e200 * 1e200, "-1e200 * -1e200", reportTestCases);
		fails += expect_like_double(Real(-1e200) / tiny, -1e200 / 1e-300, "-1e200 / 1e-300", reportTestCases);
		fails += expect_like_double(pown(Real(-2.0), 1025), -INFINITY, "(-2)^1025", reportTestCases);
		fails += expect_like_double(pown(Real(-2.0), 1026), INFINITY, "(-2)^1026", reportTestCases);

		return fails;
	}

	// the check must not fire on results that come close to the limit without crossing it
	int VerifyNearTheEdgeStaysFinite(bool reportTestCases) {
		int fails = 0;
		const Real max(DBL_MAX);

		fails += expect_like_double(max + Real(1.0), DBL_MAX + 1.0, "DBL_MAX + 1", reportTestCases);
		fails += expect_like_double(max - Real(1e300), DBL_MAX - 1e300, "DBL_MAX - 1e300", reportTestCases);
		fails += expect_like_double(Real(1e300) * Real(1e8), 1e300 * 1e8, "1e300 * 1e8", reportTestCases);
		fails += expect_like_double(pown(Real(2.0), 1023), std::ldexp(1.0, 1023), "2^1023", reportTestCases);
		fails += expect_like_double(max / Real(2.0), DBL_MAX / 2.0, "DBL_MAX / 2", reportTestCases);
		fails += expect_true(!(max + Real(1.0)).isinf(), "a sum at the limit is not infinite", reportTestCases);

		return fails;
	}

	// A large DIVIDEND overflowed an intermediate even when the quotient is in range: the
	// divisor is scaled into [0.5, 1), its reciprocal lies in (1, 2], and dividend *
	// reciprocal exceeded DBL_MAX. DBL_MAX / 2 and DBL_MAX / 1e10 were NaN (#1553).
	int VerifyLargeDividend(bool reportTestCases) {
		int fails = 0;
		const Real max(DBL_MAX);

		fails += expect_like_double(max / Real(2.0), DBL_MAX / 2.0, "DBL_MAX / 2", reportTestCases);
		fails += expect_like_double(max / Real(4.0), DBL_MAX / 4.0, "DBL_MAX / 4", reportTestCases);
		fails += expect_like_double(Real(1e308) / Real(2.0), 1e308 / 2.0, "1e308 / 2", reportTestCases);

		// a quotient that is not exact keeps its full precision: multiplying back by the
		// divisor recovers the dividend far beyond double precision
		for (double divisor : { 3.0, 1.5, 7.0, 1e10 }) {
			const Real q = max / Real(divisor);
			fails += expect_true(!q.isnan() && !q.isinf(), "a large quotient is finite", reportTestCases);
			const Real back = q * Real(divisor);
			const double residual = std::abs(double(back - max)) / DBL_MAX;
			if (!(residual < 1e-60)) {
				++fails;
				if (reportTestCases) {
					std::cout << "    FAIL (DBL_MAX / " << divisor << ") * " << divisor
					          << " misses DBL_MAX by a relative " << residual << '\n';
				}
			}
		}

		// a quotient that really does overflow is still infinite
		fails += expect_like_double(max / Real(0.5), DBL_MAX / 0.5, "DBL_MAX / 0.5", reportTestCases);

		return fails;
	}

	// an infinity that arises from overflow behaves as one downstream
	int VerifyOverflowPropagates(bool reportTestCases) {
		int fails = 0;
		const Real big(1e200), two(2.0);

		fails += expect_like_double(Real(1.0) / (big * big), 1.0 / (1e200 * 1e200),
			"1 / overflow is zero", reportTestCases);
		// 2^INT_MIN is 1 / 2^(2^31): the power overflows, and its reciprocal underflows to 0.
		// Before #1553 the overflow was NaN, so this was NaN too.
		fails += expect_like_double(pown(two, INT_MIN), 0.0, "2^INT_MIN", reportTestCases);
		fails += expect_like_double((big * big) + Real(1.0), INFINITY, "overflow + 1 stays infinite", reportTestCases);
		fails += expect_true(((big * big) - (big * big)).isnan(), "inf - inf is NaN, as in IEEE", reportTestCases);

		return fails;
	}

}  // anonymous namespace

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

	std::string test_suite  = "ereal overflow";
	std::string test_tag    = "ereal overflow";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyOverflowIsInfinite(reportTestCases), test_tag, "overflow is infinite");
	nrOfFailedTestCases += ReportTestResult(VerifyOverflowSign(reportTestCases), test_tag, "overflow sign");
	nrOfFailedTestCases += ReportTestResult(VerifyNearTheEdgeStaysFinite(reportTestCases), test_tag, "near the edge stays finite");
	nrOfFailedTestCases += ReportTestResult(VerifyOverflowPropagates(reportTestCases), test_tag, "overflow propagates");
	nrOfFailedTestCases += ReportTestResult(VerifyLargeDividend(reportTestCases), test_tag, "large dividend");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
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
