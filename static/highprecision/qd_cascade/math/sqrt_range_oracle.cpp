// sqrt_range_oracle.cpp: multi-component sqrt across the whole dynamic range
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The sqrt suites in the tree chain the function from 2.0 and 0.5 and check the result against
// std::sqrt. Neither end of the range is ever visited, which is how sqrt(maxpos) came to return inf
// or NaN in dd and qd, and sqrt(0) NaN in qd, while every suite stayed green (universal#1332).
//
// This one walks the exponent range end to end. It covers all five multi-component types, not just
// the two that were broken: the cascade types were fixed in universal#1331 and this is what keeps
// them fixed. It lives with the widest type for the same reason the trigonometry oracle does.
//
// The check is a residual rather than a comparison against a reference value. sqrt is irrational,
// so there is no exact reference to compare to -- but squaring the result is exact: a and r are
// sums of doubles, so both are dyadic rationals, and dyadic rationals are closed under
// multiplication. |r*r - a| <= 2^-budget * a is therefore decided in exact integer arithmetic, with
// no floating-point between the implementation and its verdict. Since r = sqrt(a)(1+e) gives
// r*r = a(1 + 2e + e^2), the residual reads one bit larger than the error in r itself.
//
// Measured worst residual over ~2,800 arguments per type, which is where the budgets come from:
//
//     dd          2^-104     dd_cascade  2^-106     td_cascade  2^-159
//     qd          2^-213     qd_cascade  2^-214
//
// The budgets sit four bits below each format's significand. A correct implementation clears them
// with room to spare; the implementation this test was written against delivered 52 bits in dd and
// 60 in qd, and returned 28 non-finite results.
#include <universal/utility/directives.hpp>
#include <string>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/dyadic_exact.hpp>

namespace {

	// exact dyadic value of a multi-component number: the sum of its components
	template<typename Scalar, unsigned NR_LIMBS>
	sw::universal::dyadic exact_value(const Scalar& v) {
		using namespace sw::universal;
		dyadic d;
		for (unsigned i = 0; i < NR_LIMBS; ++i) d = d + dyadic::from_double(v[static_cast<int>(i)]);
		return d;
	}

	bool magnitude_leq(const sw::universal::dyadic& a, const sw::universal::dyadic& b) {
		using namespace sw::universal;
		dyadic::bigint na, nb;
		int common{ 0 };
		dyadic_align(a, b, na, nb, common);
		return abs(na) <= abs(nb);
	}

	// |r*r - a| <= 2^-budgetBits * a, exactly
	bool residual_within(const sw::universal::dyadic& r, const sw::universal::dyadic& a, int budgetBits) {
		using namespace sw::universal;
		dyadic residual = r * r - a;
		if (residual.iszero()) return true;
		dyadic scaled(residual.numerator, residual.scale + budgetBits);
		return magnitude_leq(scaled, a);
	}

	// significandBits is what the format carries; the budget sits four bits below it
	template<typename Scalar, unsigned NR_LIMBS>
	int VerifySqrtRange(bool reportTestCases, int significandBits, int exponentStep, const std::string& tag) {
		using namespace sw::universal;
		using std::sqrt;
		const int budget = significandBits - 4;
		int nrOfFailedTests = 0;

		auto check = [&](const Scalar& a, const std::string& what) {
			Scalar r = sqrt(a);
			if (r.isnan() || r.isinf()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "  FAIL " << tag << " sqrt(" << what
				                               << ") is not finite\n";
				return;
			}
			if (a.iszero()) {
				if (!r.iszero()) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "  FAIL " << tag << " sqrt(0) is not zero\n";
				}
				return;
			}
			if (!residual_within(exact_value<Scalar, NR_LIMBS>(r), exact_value<Scalar, NR_LIMBS>(a), budget)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "  FAIL " << tag << " sqrt(" << what
				                               << ") : |r*r - a| exceeds 2^-" << budget << " * a\n";
			}
		};

		// the special values the range tests never reached
		check(Scalar(0.0), "zero");
		check(Scalar(SpecificValue::minpos), "minpos");
		check(Scalar(SpecificValue::maxpos), "maxpos");
		check(Scalar(1.7976931348623157e308), "the largest double");
		check(Scalar(1.0), "1.0");

		// and the range end to end
		for (int e = -1070; e <= 1020; e += exponentStep) {
			for (double m : { 1.0, 1.3, 1.7, 1.9999999 }) {
				Scalar a = ldexp(Scalar(m), e);
				if (a.iszero() || a.isinf()) continue;
				check(a, "2^" + std::to_string(e));
			}
		}
		return nrOfFailedTests;
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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "multi-component sqrt across the dynamic range";
	std::string test_tag    = "sqrt range";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	constexpr int QD_SIGNIFICAND = 212;
	nrOfFailedTestCases += VerifySqrtRange<qd, 4>(true, QD_SIGNIFICAND, 31, "qd");
	nrOfFailedTestCases += VerifySqrtRange<qd_cascade, 4>(true, QD_SIGNIFICAND, 31, "qd_cascade");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	constexpr int DD_SIGNIFICAND = 106;   // a double-double's significand
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRange<dd, 2>(reportTestCases, DD_SIGNIFICAND, 31, "dd"), test_tag, "dd");
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRange<dd_cascade, 2>(reportTestCases, DD_SIGNIFICAND, 31, "dd_cascade"), test_tag, "dd_cascade");
#endif

#if REGRESSION_LEVEL_2
	constexpr int TD_SIGNIFICAND = 159;   // a triple-double's significand
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRange<td_cascade, 3>(reportTestCases, TD_SIGNIFICAND, 31, "td_cascade"), test_tag, "td_cascade");
#endif

#if REGRESSION_LEVEL_3
	constexpr int QD_SIGNIFICAND = 212;   // a quad-double's significand
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRange<qd, 4>(reportTestCases, QD_SIGNIFICAND, 31, "qd"), test_tag, "qd");
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRange<qd_cascade, 4>(reportTestCases, QD_SIGNIFICAND, 31, "qd_cascade"), test_tag, "qd_cascade");
#endif

#if REGRESSION_LEVEL_4
	// every seventh exponent rather than every thirty-first
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRange<dd, 2>(reportTestCases, 106, 7, "dd"), test_tag, "dd, dense");
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRange<td_cascade, 3>(reportTestCases, 159, 7, "td_cascade"), test_tag, "td_cascade, dense");
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRange<qd, 4>(reportTestCases, 212, 7, "qd"), test_tag, "qd, dense");
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
