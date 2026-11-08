// special_values.cpp: efloat comparisons with NaN, +-inf and +-0 (#2005)
//
// Locks in the IEEE-754 ordering of the special values:
//   - every comparison with a NaN is false, except != which is true; NaN == NaN is false
//   - -0 == +0, and neither is less than the other
//   - -inf < every finite value < +inf, and inf == inf with the same sign
//
//   operators   == != < > <= >=, efloat against efloat, and efloat against double on either side
//   values      quiet NaN, +-inf, +-0, +-1, +-0.5, +-1e300, and efloat's own min() and max(),
//               which lie past double's range
//   oracle      the same comparison of the doubles, which C++ evaluates by IEEE-754; for min()
//               and max(), the ordering stated directly
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using sw::universal::efloat;

	// the special and ordinary values, as doubles
	std::vector<double> values() {
		const double inf = std::numeric_limits<double>::infinity();
		return { std::numeric_limits<double>::quiet_NaN(), inf, -inf, 0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 1e300, -1e300 };
	}

	std::string label(double v) {
		if (v != v) return "nan";
		if (v == 0.0) return std::signbit(v) ? "-0" : "+0";
		return std::to_string(v);
	}

	// the six operators on a and b, as bits: == != < > <= >=
	template<typename A, typename B>
	unsigned relations(const A& a, const B& b) {
		return (a == b ? 1u : 0u) | (a != b ? 2u : 0u) | (a < b ? 4u : 0u) | (a > b ? 8u : 0u) | (a <= b ? 16u : 0u) | (a >= b ? 32u : 0u);
	}

	// efloat vs efloat, efloat vs double and double vs efloat, against double vs double
	template<unsigned nlimbs>
	int VerifySpecialValueComparisons(bool reportTestCases) {
		using E = efloat<nlimbs>;
		int nrOfFailedTests = 0;
		const std::vector<double> v = values();
		for (double a : v) {
			for (double b : v) {
				E ea(a), eb(b);
				if (std::signbit(a) != (ea.sign() == -1) || (a != a) != ea.isnan()) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: efloat<" << nlimbs << ">(" << label(a) << ") lost its sign or NaN\n";
				}
				const unsigned expected = relations(a, b);
				const unsigned ee = relations(ea, eb);
				const unsigned ed = relations(ea, b);
				const unsigned de = relations(a, eb);
				if (ee != expected || ed != expected || de != expected) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: efloat<" << nlimbs << "> " << label(a) << " vs " << label(b) << ": expected " << expected
					                               << " efloat/efloat " << ee << " efloat/double " << ed << " double/efloat " << de << '\n';
				}
			}
		}
		return nrOfFailedTests;
	}

	// min() and max() against the special values: the ordering is
	// -inf < -max() < -1 < -min() < -0 == +0 < min() < 1 < max() < +inf, and NaN is unordered
	template<unsigned nlimbs>
	int VerifyRangeEnds(bool reportTestCases) {
		using E = efloat<nlimbs>;
		int nrOfFailedTests = 0;
		const E inf = std::numeric_limits<E>::infinity();
		const E nan = std::numeric_limits<E>::quiet_NaN();
		const E zero(0.0), negzero(-0.0), one(1.0);
		const E mx = std::numeric_limits<E>::max(), mn = std::numeric_limits<E>::min();
		const std::vector<E> ordered = { -inf, -mx, -one, -mn, negzero, mn, one, mx, inf };
		for (std::size_t i = 0; i < ordered.size(); ++i) {
			for (std::size_t j = 0; j < ordered.size(); ++j) {
				// -0 at index 4 stands for both zeros: +0 is checked separately below
				const unsigned expected = (i == j) ? (1u | 16u | 32u) : (i < j ? (2u | 4u | 16u) : (2u | 8u | 32u));
				const unsigned got = relations(ordered[i], ordered[j]);
				if (got != expected) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: efloat<" << nlimbs << "> ordered[" << i << "] vs ordered[" << j << "]: " << got << " (want " << expected << ")\n";
				}
				const unsigned with_nan = relations(ordered[i], nan) | relations(nan, ordered[i]);
				if (with_nan != 2u) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: efloat<" << nlimbs << "> ordered[" << i << "] vs nan: " << with_nan << '\n';
				}
			}
			// +0 sits where -0 does
			const unsigned z = relations(ordered[i], zero), nz = relations(ordered[i], negzero);
			if (z != nz) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: efloat<" << nlimbs << "> ordered[" << i << "] vs +0 " << z << " and -0 " << nz << '\n';
			}
		}
		if (relations(nan, nan) != 2u) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: efloat<" << nlimbs << "> nan vs nan\n";
		}
		return nrOfFailedTests;
	}

}  // namespace

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

	std::string test_suite  = "efloat comparisons with NaN, inf and signed zero";
	std::string test_tag    = "special values";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifySpecialValueComparisons<4>(true), "efloat<4>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValueComparisons<1>(reportTestCases), "efloat<1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValueComparisons<4>(reportTestCases), "efloat<4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRangeEnds<4>(reportTestCases), "efloat<4>", "min() and max()");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValueComparisons<32>(reportTestCases), "efloat<32>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRangeEnds<32>(reportTestCases), "efloat<32>", "min() and max()");
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
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
