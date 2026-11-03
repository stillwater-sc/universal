//  logic.cpp : test suite for logic operators on adaptive precision decimal rational numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <universal/number/erational/erational.hpp>
#include <universal/verification/test_suite.hpp>

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

// namespace scope, so constant-initialized: both digit vectors empty, not the runtime 0/1 (#1909)
sw::universal::erational constant_zero;

namespace {

	using sw::universal::erational;

	int check(bool ok, const std::string& what, bool reportTestCases) {
		if (!ok && reportTestCases) std::cerr << "FAIL: " << what << '\n';
		return ok ? 0 : 1;
	}

	// all six operators on (a, b) against the expected order: -1, 0 or +1
	int VerifyOrder(const erational& a, const erational& b, int order, const std::string& tag, bool reportTestCases) {
		int fails = 0;
		fails += check((a == b) == (order == 0), tag + " ==", reportTestCases);
		fails += check((a != b) == (order != 0), tag + " !=", reportTestCases);
		fails += check((a <  b) == (order <  0), tag + " <",  reportTestCases);
		fails += check((a >  b) == (order >  0), tag + " >",  reportTestCases);
		fails += check((a <= b) == (order <= 0), tag + " <=", reportTestCases);
		fails += check((a >= b) == (order >= 0), tag + " >=", reportTestCases);
		return fails;
	}

	// the erational-long and long-erational overloads against the expected order of (a, b)
	int VerifyOrderLong(const erational& a, long b, int order, const std::string& tag, bool reportTestCases) {
		int fails = 0;
		fails += check((a == b) == (order == 0), tag + " r==l", reportTestCases);
		fails += check((a != b) == (order != 0), tag + " r!=l", reportTestCases);
		fails += check((a <  b) == (order <  0), tag + " r<l",  reportTestCases);
		fails += check((a >  b) == (order >  0), tag + " r>l",  reportTestCases);
		fails += check((a <= b) == (order <= 0), tag + " r<=l", reportTestCases);
		fails += check((a >= b) == (order >= 0), tag + " r>=l", reportTestCases);
		fails += check((b == a) == (order == 0), tag + " l==r", reportTestCases);
		fails += check((b != a) == (order != 0), tag + " l!=r", reportTestCases);
		fails += check((b <  a) == (order >  0), tag + " l<r",  reportTestCases);
		fails += check((b >  a) == (order <  0), tag + " l>r",  reportTestCases);
		fails += check((b <= a) == (order >= 0), tag + " l<=r", reportTestCases);
		fails += check((b >= a) == (order <= 0), tag + " l>=r", reportTestCases);
		return fails;
	}

	// a NaN is unordered: every comparison is false except !=
	int VerifyUnordered(const erational& a, const erational& b, const std::string& tag, bool reportTestCases) {
		int fails = 0;
		fails += check(!(a == b), tag + " ==", reportTestCases);
		fails += check( (a != b), tag + " !=", reportTestCases);
		fails += check(!(a <  b), tag + " <",  reportTestCases);
		fails += check(!(a >  b), tag + " >",  reportTestCases);
		fails += check(!(a <= b), tag + " <=", reportTestCases);
		fails += check(!(a >= b), tag + " >=", reportTestCases);
		return fails;
	}

	// signed values in increasing order, each with its rank: equal values share a rank
	struct Ranked { erational v; int rank; std::string name; };

	std::vector<Ranked> OrderedValues() {
		// a zero with the sign flag set, as 0 * -3 leaves it (to_ieee754 gives -0.0). erational(-0.0)
		// would not do: convert_ieee754 returns a plain 0 for either IEEE zero.
		const erational negzero = erational(0) * erational(-3);
		return {
			{ erational(-7),         0, "-7"    },
			{ erational(-3, 2),      1, "-3/2"  },
			{ erational(3, -2),      1, "3/-2"  },
			{ erational(-1),         2, "-1"    },
			{ erational(-1, 2),      3, "-1/2"  },
			{ erational(-2, 4),      3, "-2/4"  },   // unreduced
			{ erational(0),          4, "0"     },
			{ negzero,               4, "0*-3"  },
			{ constant_zero,         4, "cz"    },
			{ erational(0, -5),      4, "0/-5"  },
			{ erational(1, 2),       5, "1/2"   },
			{ erational(2, 4),       5, "2/4"   },   // unreduced
			{ erational(1),          6, "1"     },
			{ erational(-3, -2),     7, "-3/-2" },
			{ erational(7),          8, "7"     },
		};
	}

}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "erational logic comparisons ";
	std::string test_tag    = "comparison";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	erational a(-7), b(7);
	std::cout << a << " == " << b << " : " << (a == b) << '\n';
	std::cout << a << " <  " << b << " : " << (a <  b) << '\n';
	std::cout << "constant zero == 0 : " << (constant_zero == erational(0)) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	{
		// the fixtures are what they claim: a sign-flagged zero, and a constant-initialized zero
		int fails = 0;
		const erational negzero = erational(0) * erational(-3);
		fails += check(negzero.iszero() && negzero.isneg(), "0 * -3 is a sign-flagged zero", reportTestCases);
		fails += check(std::signbit(double(negzero)), "0 * -3 converts to -0.0", reportTestCases);
		fails += check(constant_zero.is_constant_zero(), "namespace-scope erational is constant-initialized", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(fails, "erational", "zero fixtures");
	}
	{
		// every ordered pair, all six operators (#1908: the sign; #1909: the constant zero)
		const auto values = OrderedValues();
		int fails = 0;
		for (const auto& x : values) {
			for (const auto& y : values) {
				const int order = (x.rank < y.rank) ? -1 : ((x.rank > y.rank) ? 1 : 0);
				fails += VerifyOrder(x.v, y.v, order, x.name + " vs " + y.name, reportTestCases);
			}
		}
		nrOfFailedTestCases += ReportTestResult(fails, "erational", "ordered pairs");
	}
	{
		// the long overloads, both operand orders
		const auto values = OrderedValues();
		const std::vector<std::pair<long, int>> longs = { { -7L, 0 }, { -1L, 2 }, { 0L, 4 }, { 1L, 6 }, { 7L, 8 } };
		int fails = 0;
		for (const auto& x : values) {
			for (const auto& [l, rank] : longs) {
				const int order = (x.rank < rank) ? -1 : ((x.rank > rank) ? 1 : 0);
				fails += VerifyOrderLong(x.v, l, order, x.name + " vs " + std::to_string(l), reportTestCases);
			}
		}
		nrOfFailedTestCases += ReportTestResult(fails, "erational", "long overloads");
	}
	{
		// NaN is unordered, against itself, numbers, zeros and the constant zero
		erational nan;
		nan.setnan();
		erational inf_state(1, 0);    // n/0: never normalized, also unordered
		int fails = 0;
		for (const auto& x : OrderedValues()) {
			fails += VerifyUnordered(nan, x.v, "nan vs " + x.name, reportTestCases);
			fails += VerifyUnordered(x.v, nan, x.name + " vs nan", reportTestCases);
			fails += VerifyUnordered(inf_state, x.v, "1/0 vs " + x.name, reportTestCases);
		}
		fails += VerifyUnordered(nan, nan, "nan vs nan", reportTestCases);
		fails += check(nan != 0L && !(nan == 0L) && !(0L < nan) && !(0L >= nan), "nan vs long", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(fails, "erational", "NaN unordered");
	}
	{
		// the two-integer constructor: magnitudes in the digits, the sign in the flag
		int fails = 0;
		auto shape = [&](const erational& r, long long n, long long d, bool neg, const std::string& what) {
			fails += check(r.top() == edecimal(n) && r.bottom() == edecimal(d) && r.isneg() == neg, what, reportTestCases);
		};
		shape(erational(-3, 2),  3, 2, true,  "erational(-3, 2)");
		shape(erational(3, -2),  3, 2, true,  "erational(3, -2)");
		shape(erational(-3, -2), 3, 2, false, "erational(-3, -2)");
		shape(erational(0, -5),  0, 5, false, "erational(0, -5)");
		shape(erational(8, 18),  8, 18, false, "erational(8, 18) unreduced");
		shape(erational(7u, 2ull), 7, 2, false, "erational(7u, 2ull)");
		{
			const erational m(std::numeric_limits<std::int64_t>::min(), 1);
			fails += check(m.isneg() && m.top() == edecimal(9223372036854775808ull), "erational(INT64_MIN, 1)", reportTestCases);
		}
		nrOfFailedTestCases += ReportTestResult(fails, "erational", "erational(n, d)");
	}
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
catch (const sw::universal::erational_arithmetic_exception& err) {
	std::cerr << "Uncaught arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::erational_internal_exception& err) {
	std::cerr << "Uncaught internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
