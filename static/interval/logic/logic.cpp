// logic.cpp: test suite for interval comparison and logic operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/interval/interval.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

// Regression testing guards
#define MANUAL_TESTING 0
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

namespace sw { namespace universal {

template<typename Scalar>
int VerifyIntervalEquality(bool reportTestCases) {
	using Interval = interval<Scalar>;
	int nrOfFailedTestCases = 0;

	// Test equality
	{
		Interval a(Scalar(1), Scalar(2));
		Interval b(Scalar(1), Scalar(2));
		if (!(a == b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " == " << b << " should be true\n";
		}
	}

	// Test inequality
	{
		Interval a(Scalar(1), Scalar(2));
		Interval b(Scalar(1), Scalar(3));
		if (!(a != b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " != " << b << " should be true\n";
		}
	}

	// Test equality with same bounds but different order should still be equal
	{
		Interval a(Scalar(1), Scalar(2));
		Interval b(Scalar(2), Scalar(1));  // constructor should swap
		if (!(a == b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " == " << b << " should be true (after auto-swap)\n";
		}
	}

	return nrOfFailedTestCases;
}

template<typename Scalar>
int VerifyIntervalOrdering(bool reportTestCases) {
	using Interval = interval<Scalar>;
	int nrOfFailedTestCases = 0;

	// Test less than (entire interval before)
	{
		Interval a(Scalar(1), Scalar(2));
		Interval b(Scalar(3), Scalar(4));
		if (!(a < b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " < " << b << " should be true\n";
		}
	}

	// Test greater than
	{
		Interval a(Scalar(5), Scalar(6));
		Interval b(Scalar(1), Scalar(2));
		if (!(a > b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " > " << b << " should be true\n";
		}
	}

	// Test overlapping intervals - neither < nor > should be true
	{
		Interval a(Scalar(1), Scalar(3));
		Interval b(Scalar(2), Scalar(4));
		if (a < b || a > b) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: overlapping intervals " << a << " and " << b << " should not be ordered\n";
		}
	}

	// Test <= and >=
	{
		Interval a(Scalar(1), Scalar(2));
		Interval b(Scalar(3), Scalar(4));
		if (!(a <= b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " <= " << b << " should be true\n";
		}
		if (!(b >= a)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << b << " >= " << a << " should be true\n";
		}
	}

	return nrOfFailedTestCases;
}

template<typename Scalar>
int VerifyIntervalPredicates(bool reportTestCases) {
	using Interval = interval<Scalar>;
	int nrOfFailedTestCases = 0;

	// Test iszero
	{
		Interval a;
		a.setzero();
		if (!a.iszero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " iszero() should be true\n";
		}
	}

	// Test isdegenerate
	{
		Interval a(Scalar(3));
		if (!a.isdegenerate()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " isdegenerate() should be true\n";
		}

		Interval b(Scalar(1), Scalar(2));
		if (b.isdegenerate()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << b << " isdegenerate() should be false\n";
		}
	}

	// Test contains_zero
	{
		Interval a(Scalar(-1), Scalar(1));
		if (!a.contains_zero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " contains_zero() should be true\n";
		}

		Interval b(Scalar(1), Scalar(2));
		if (b.contains_zero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << b << " contains_zero() should be false\n";
		}
	}

	// Test contains
	{
		Interval a(Scalar(1), Scalar(5));
		if (!a.contains(Scalar(3))) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " contains(3) should be true\n";
		}
		if (a.contains(Scalar(6))) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " contains(6) should be false\n";
		}
	}

	// Test ispos and isneg
	{
		Interval a(Scalar(1), Scalar(2));
		if (!a.ispos()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " ispos() should be true\n";
		}

		Interval b(Scalar(-2), Scalar(-1));
		if (!b.isneg()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << b << " isneg() should be true\n";
		}

		Interval c(Scalar(-1), Scalar(1));
		if (c.ispos() || c.isneg()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << c << " should be neither pos nor neg\n";
		}
	}

	return nrOfFailedTestCases;
}

template<typename Scalar>
int VerifyIntervalContainment(bool reportTestCases) {
	using Interval = interval<Scalar>;
	int nrOfFailedTestCases = 0;

	// Test subset_of
	{
		Interval a(Scalar(2), Scalar(3));
		Interval b(Scalar(1), Scalar(4));
		if (!a.subset_of(b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " subset_of " << b << " should be true\n";
		}
		if (b.subset_of(a)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << b << " subset_of " << a << " should be false\n";
		}
	}

	// Test proper_subset_of
	{
		Interval a(Scalar(2), Scalar(3));
		Interval b(Scalar(1), Scalar(4));
		if (!a.proper_subset_of(b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " proper_subset_of " << b << " should be true\n";
		}

		Interval c(Scalar(1), Scalar(4));
		if (c.proper_subset_of(b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << c << " proper_subset_of " << b << " should be false (equal)\n";
		}
	}

	// Test overlaps
	{
		Interval a(Scalar(1), Scalar(3));
		Interval b(Scalar(2), Scalar(4));
		if (!a.overlaps(b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " overlaps " << b << " should be true\n";
		}

		Interval c(Scalar(5), Scalar(6));
		if (a.overlaps(c)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << a << " overlaps " << c << " should be false\n";
		}
	}

	return nrOfFailedTestCases;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "interval logic validation";
	std::string test_tag    = "logic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual testing

#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	std::cout << "Equality tests (float)\n";
	nrOfFailedTestCases += VerifyIntervalEquality<float>(reportTestCases);
	std::cout << "Ordering tests (float)\n";
	nrOfFailedTestCases += VerifyIntervalOrdering<float>(reportTestCases);
#endif

#if REGRESSION_LEVEL_2
	std::cout << "Predicate tests (float)\n";
	nrOfFailedTestCases += VerifyIntervalPredicates<float>(reportTestCases);
	std::cout << "Containment tests (float)\n";
	nrOfFailedTestCases += VerifyIntervalContainment<float>(reportTestCases);
#endif

#if REGRESSION_LEVEL_3
	std::cout << "Equality tests (double)\n";
	nrOfFailedTestCases += VerifyIntervalEquality<double>(reportTestCases);
	std::cout << "Ordering tests (double)\n";
	nrOfFailedTestCases += VerifyIntervalOrdering<double>(reportTestCases);
	std::cout << "Predicate tests (double)\n";
	nrOfFailedTestCases += VerifyIntervalPredicates<double>(reportTestCases);
	std::cout << "Containment tests (double)\n";
	nrOfFailedTestCases += VerifyIntervalContainment<double>(reportTestCases);
#endif

#if REGRESSION_LEVEL_4
	std::cout << "Equality tests (cfloat<16,5>)\n";
	nrOfFailedTestCases += VerifyIntervalEquality<cfloat<16, 5, uint16_t>>(reportTestCases);
	std::cout << "Ordering tests (cfloat<16,5>)\n";
	nrOfFailedTestCases += VerifyIntervalOrdering<cfloat<16, 5, uint16_t>>(reportTestCases);
#endif

#endif // MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
