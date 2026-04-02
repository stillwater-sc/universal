// api.cpp: API tests for the bisection number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>

#include <universal/number/bisection/bisection.hpp>
#include <universal/verification/test_suite.hpp>

/// Exhaustive encode/decode round-trip test.
/// For a p-bit bisection type, enumerate all 2^p encodings, decode
/// each to double, re-encode the double, and verify the encoding
/// matches the original.
template<typename BisectionType>
int VerifyRoundTrip(bool reportTestCases) {
	constexpr unsigned p = BisectionType::nbits;
	constexpr int64_t NR_ENCODINGS = int64_t(1) << p;
	int nrOfFailedTests = 0;

	for (int64_t i = 0; i < NR_ENCODINGS; ++i) {
		BisectionType a;
		a.setbits(static_cast<uint64_t>(i));

		if (a.isnan()) continue;

		double d = double(a);
		BisectionType b(d);

		if (a != b) {
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests <= 10) {
				std::cerr << "FAIL round-trip: encoding " << i
				          << " -> " << d
				          << " -> " << sw::universal::to_binary(b)
				          << " (expected " << sw::universal::to_binary(a) << ")\n";
			}
		}
	}
	return nrOfFailedTests;
}

/// Verify monotonicity: for all consecutive encodings, the decoded
/// values must be non-decreasing (strictly increasing for non-NaN).
template<typename BisectionType>
int VerifyMonotonicity(bool reportTestCases) {
	constexpr unsigned p = BisectionType::nbits;
	constexpr int64_t NR_ENCODINGS = int64_t(1) << p;
	int nrOfFailedTests = 0;

	double prev = -std::numeric_limits<double>::infinity();
	for (int64_t i = 0; i < NR_ENCODINGS; ++i) {
		BisectionType a;
		a.setbits(static_cast<uint64_t>(i));
		if (a.isnan()) { prev = -std::numeric_limits<double>::infinity(); continue; }

		double d = double(a);
		if (d < prev) {
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests <= 10) {
				std::cerr << "FAIL monotonicity: encoding " << i
				          << " = " << d << " < prev " << prev << "\n";
			}
		}
		prev = d;
	}
	return nrOfFailedTests;
}

/// Verify zero encodes to the all-zeros bit pattern.
template<typename BisectionType>
int VerifyZero(bool reportTestCases) {
	BisectionType z(0.0);
	if (!z.iszero() || z.getbits() != 0) {
		if (reportTestCases) {
			std::cerr << "FAIL zero: 0.0 encoded as " << sw::universal::to_binary(z)
			          << " (bits=" << z.getbits() << ")\n";
		}
		return 1;
	}
	return 0;
}

/// Verify negation symmetry: encode(x) = -encode(-x) for all encodings.
template<typename BisectionType>
int VerifyBisectionNegation(bool reportTestCases) {
	constexpr unsigned p = BisectionType::nbits;
	constexpr int64_t NR_ENCODINGS = int64_t(1) << p;
	int nrOfFailedTests = 0;

	for (int64_t i = 0; i < NR_ENCODINGS; ++i) {
		BisectionType a;
		a.setbits(static_cast<uint64_t>(i));
		if (a.isnan() || a.iszero()) continue;

		double d = double(a);
		BisectionType neg_a(-d);
		BisectionType neg_direct = -a;

		if (neg_a != neg_direct) {
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests <= 10) {
				std::cerr << "FAIL negation: -(" << d << ") encode mismatch: "
				          << sw::universal::to_binary(neg_a) << " vs "
				          << sw::universal::to_binary(neg_direct) << "\n";
			}
		}
	}
	return nrOfFailedTests;
}

// Regression testing guards
#define MANUAL_TESTING 0
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

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "bisection number system API validation";
	std::string test_tag   = "bisection";
	bool reportTestCases   = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		// Quick smoke test: Posit(1) bisection at 8 bits
		using BP = bisection_posit<8, 1>;
		std::cout << "bisection_posit<8,1> values:\n";
		for (int i = 0; i < 256; ++i) {
			BP a;
			a.setbits(i);
			double d = double(a);
			std::cout << "  " << std::setw(3) << i << ": "
			          << to_binary(a) << " = " << d << "\n";
		}
	}
	{
		// Unary 6-bit
		using BU = bisection_unary<6>;
		std::cout << "\nbisection_unary<6> values:\n";
		for (int i = 0; i < 64; ++i) {
			BU a;
			a.setbits(i);
			std::cout << "  " << std::setw(2) << i << ": "
			          << to_binary(a) << " = " << double(a) << "\n";
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else //!MANUAL_TESTING

#if REGRESSION_LEVEL_1

	// ── Round-trip: encode -> decode -> re-encode for small types ──

	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<bisection_unary<6>>(reportTestCases),
		test_tag, "bisection_unary<6> round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<bisection_unary<8>>(reportTestCases),
		test_tag, "bisection_unary<8> round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<bisection_posit<6, 0>>(reportTestCases),
		test_tag, "bisection_posit<6,0> round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<bisection_posit<8, 0>>(reportTestCases),
		test_tag, "bisection_posit<8,0> round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<bisection_posit<8, 1>>(reportTestCases),
		test_tag, "bisection_posit<8,1> round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<bisection_elias_gamma<6>>(reportTestCases),
		test_tag, "bisection_elias_gamma<6> round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<bisection_elias_gamma<8>>(reportTestCases),
		test_tag, "bisection_elias_gamma<8> round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<bisection_fibonacci<6>>(reportTestCases),
		test_tag, "bisection_fibonacci<6> round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<bisection_fibonacci<8>>(reportTestCases),
		test_tag, "bisection_fibonacci<8> round-trip");

	// ── Monotonicity ─────────────────────────────────────────────

	nrOfFailedTestCases += ReportTestResult(
		VerifyMonotonicity<bisection_unary<8>>(reportTestCases),
		test_tag, "bisection_unary<8> monotonicity");
	nrOfFailedTestCases += ReportTestResult(
		VerifyMonotonicity<bisection_posit<8, 1>>(reportTestCases),
		test_tag, "bisection_posit<8,1> monotonicity");
	nrOfFailedTestCases += ReportTestResult(
		VerifyMonotonicity<bisection_elias_gamma<8>>(reportTestCases),
		test_tag, "bisection_elias_gamma<8> monotonicity");
	nrOfFailedTestCases += ReportTestResult(
		VerifyMonotonicity<bisection_fibonacci<8>>(reportTestCases),
		test_tag, "bisection_fibonacci<8> monotonicity");

	// ── Zero encoding ────────────────────────────────────────────

	nrOfFailedTestCases += ReportTestResult(
		VerifyZero<bisection_unary<8>>(reportTestCases),
		test_tag, "bisection_unary<8> zero");
	nrOfFailedTestCases += ReportTestResult(
		VerifyZero<bisection_posit<8, 1>>(reportTestCases),
		test_tag, "bisection_posit<8,1> zero");
	nrOfFailedTestCases += ReportTestResult(
		VerifyZero<bisection_elias_gamma<8>>(reportTestCases),
		test_tag, "bisection_elias_gamma<8> zero");

	// ── Negation symmetry ────────────────────────────────────────

	nrOfFailedTestCases += ReportTestResult(
		VerifyBisectionNegation<bisection_unary<8>>(reportTestCases),
		test_tag, "bisection_unary<8> negation");
	nrOfFailedTestCases += ReportTestResult(
		VerifyBisectionNegation<bisection_posit<8, 1>>(reportTestCases),
		test_tag, "bisection_posit<8,1> negation");
	nrOfFailedTestCases += ReportTestResult(
		VerifyBisectionNegation<bisection_elias_gamma<8>>(reportTestCases),
		test_tag, "bisection_elias_gamma<8> negation");

#endif

#if REGRESSION_LEVEL_2

	// Larger types: 10 and 12 bit
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<bisection_posit<10, 1>>(reportTestCases),
		test_tag, "bisection_posit<10,1> round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<bisection_unary<10>>(reportTestCases),
		test_tag, "bisection_unary<10> round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<bisection_elias_gamma<10>>(reportTestCases),
		test_tag, "bisection_elias_gamma<10> round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyMonotonicity<bisection_posit<12, 1>>(reportTestCases),
		test_tag, "bisection_posit<12,1> monotonicity");

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
