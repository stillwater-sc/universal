// dbns_fdp.cpp: tests for dbns fused dot product via generalized quire
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// ============================================================================
// Quire Architecture for dbns<8, 4, uint8_t>
// ============================================================================
//
// dbns<8, 4>: 8-bit double-base number system
//   1 sign bit, 4 first-base (0.5) exponent bits, 3 second-base (3) exponent bits
//   Value = (-1)^sign * 2^(-a) * 3^b
//
// quire_traits<dbns<8,4>>:
//   exponent_bits = 7 (capped at 10)
//   range         = 2 * 128 = 256
//   radix_point   = 128
//   qbits         = 286
// ============================================================================
//
#include <universal/utility/directives.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/dbns/fdp.hpp>
#include <universal/verification/test_suite.hpp>
#include <vector>
#include <cmath>

namespace sw { namespace universal {

// ============================================================================
// TestQuireMul: verify dbns products via quire accumulation
// ============================================================================
int TestQuireMul() {
	int nrOfFailedTestCases = 0;

	using Scalar = dbns<8, 4, uint8_t>;

	// Product of 1.0 * 1.0 = 1.0
	{
		Scalar a(1.0), b(1.0);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		double expected = double(a) * double(b);
		if (std::abs(result - expected) > 0.5) {
			std::cerr << "FAIL: quire_mul(1, 1) expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Product of 2.0 * 3.0
	{
		Scalar a(2.0), b(3.0);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		double expected = double(a) * double(b);
		if (std::abs(result - expected) > 1.0) {
			std::cerr << "FAIL: quire_mul(2, 3) expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Negative product
	{
		Scalar a(-2.0), b(3.0);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		double expected = double(a) * double(b);
		if (std::abs(result - expected) > 1.0) {
			std::cerr << "FAIL: quire_mul(-2, 3) expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Zero product
	{
		Scalar a, b(3.0);
		a.setzero();
		auto product = quire_mul(a, b);
		if (!product.iszero()) {
			std::cerr << "FAIL: quire_mul(0, 3) should be zero\n";
			++nrOfFailedTestCases;
		}
	}

	// Accumulation of multiple products
	{
		quire<Scalar> q;
		Scalar a(2.0), b(1.0), c(3.0), d(1.0);
		q += quire_mul(a, b);
		q += quire_mul(c, d);
		double result = q.convert_to<double>();
		double expected = double(a) * double(b) + double(c) * double(d);
		if (std::abs(result - expected) > 1.0) {
			std::cerr << "FAIL: dbns accumulation expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestBasicFdp: dbns dot products
// ============================================================================
int TestBasicFdp() {
	int nrOfFailedTestCases = 0;

	using Scalar = dbns<8, 4, uint8_t>;

	// Simple dot product
	{
		std::vector<Scalar> x = { Scalar(1.0), Scalar(2.0), Scalar(3.0) };
		std::vector<Scalar> y = { Scalar(1.0), Scalar(1.0), Scalar(1.0) };
		Scalar result = fdp(x, y);
		double expected = 0.0;
		for (size_t i = 0; i < 3; ++i) expected += double(x[i]) * double(y[i]);
		if (std::abs(double(result) - expected) > 2.0) {
			std::cerr << "FAIL: basic fdp expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Cancellation
	{
		std::vector<Scalar> x = { Scalar(4.0), Scalar(-4.0) };
		std::vector<Scalar> y = { Scalar(1.0), Scalar(1.0) };
		Scalar result = fdp(x, y);
		double expected = double(x[0]) + double(x[1]);
		if (std::abs(double(result) - expected) > 1.0) {
			std::cerr << "FAIL: cancellation fdp expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestFdp1024: 1024-element dbns FDP tests
// ============================================================================
int TestFdp1024() {
	int nrOfFailedTestCases = 0;

	using Scalar = dbns<8, 4, uint8_t>;

	// Case 1: 1024 products of 1*1 = 1024
	// dbns has a sparse value grid, so the quire accumulates 1024 exactly
	// but quire_resolve rounds to the nearest representable dbns value
	{
		std::vector<Scalar> x(1024, Scalar(1.0));
		std::vector<Scalar> y(1024, Scalar(1.0));
		Scalar result = fdp(x, y);
		Scalar expected(1024.0);  // nearest dbns value to 1024
		if (double(result) != double(expected)) {
			std::cerr << "FAIL: fdp1024 case 1, expected " << double(expected)
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 2: cancellation — 512 pairs of (+1, -1) = 0
	{
		std::vector<Scalar> x(1024), y(1024, Scalar(1.0));
		for (int i = 0; i < 1024; i += 2) {
			x[i]     = Scalar(1.0);
			x[i + 1] = Scalar(-1.0);
		}
		Scalar result = fdp(x, y);
		if (std::abs(double(result)) > 1.0) {
			std::cerr << "FAIL: fdp1024 case 2 (cancel), got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Case 3: reproducibility
	{
		std::vector<Scalar> x(1024), y(1024);
		for (int i = 0; i < 1024; ++i) {
			x[i] = Scalar(double(1 + (i % 4)));
			y[i] = Scalar(double(1 + ((i + 2) % 3)));
		}
		Scalar result1 = fdp(x, y);
		std::vector<Scalar> xr(x.rbegin(), x.rend());
		std::vector<Scalar> yr(y.rbegin(), y.rend());
		Scalar result2 = fdp(xr, yr);
		if (double(result1) != double(result2)) {
			std::cerr << "FAIL: fdp1024 case 3 (reproducibility), "
			          << double(result1) << " != " << double(result2) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestLimbBoundary: carry/borrow tests
// ============================================================================
int TestLimbBoundary() {
	int nrOfFailedTestCases = 0;

	using Scalar = dbns<8, 4, uint8_t>;

	// Carry across boundary via repeated accumulation
	{
		quire<Scalar> q;
		Scalar one(1.0);
		for (int i = 0; i < 100; ++i) {
			q += quire_mul(one, one);
		}
		double result = q.convert_to<double>();
		if (std::abs(result - 100.0) > 2.0) {
			std::cerr << "FAIL: dbns limb carry, expected 100, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Exact cancellation
	{
		quire<Scalar> q;
		Scalar pos(4.0), neg(-4.0), one(1.0);
		for (int i = 0; i < 50; ++i) q += quire_mul(pos, one);
		for (int i = 0; i < 50; ++i) q += quire_mul(neg, one);
		if (!q.iszero()) {
			double result = q.convert_to<double>();
			std::cerr << "FAIL: dbns limb cancel, expected 0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestFdpStride: strided dot product
// ============================================================================
int TestFdpStride() {
	int nrOfFailedTestCases = 0;

	using Scalar = dbns<8, 4, uint8_t>;

	{
		std::vector<Scalar> x = { Scalar(2.0), Scalar(100.0), Scalar(3.0), Scalar(100.0) };
		std::vector<Scalar> y = { Scalar(1.0), Scalar(100.0), Scalar(1.0), Scalar(100.0) };
		Scalar result = fdp_stride(size_t(2), x, size_t(2), y, size_t(2));
		double expected = double(x[0]) * double(y[0]) + double(x[2]) * double(y[2]);
		if (std::abs(double(result) - expected) > 1.0) {
			std::cerr << "FAIL: fdp_stride expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestFdpQc: quire continuation
// ============================================================================
int TestFdpQc() {
	int nrOfFailedTestCases = 0;

	using Scalar = dbns<8, 4, uint8_t>;

	{
		quire<Scalar> q;
		std::vector<Scalar> x1 = { Scalar(2.0), Scalar(3.0) };
		std::vector<Scalar> y1 = { Scalar(1.0), Scalar(1.0) };
		fdp_qc(q, size_t(2), x1, size_t(1), y1, size_t(1));
		double expected = double(x1[0]) + double(x1[1]);
		double result = q.convert_to<double>();
		if (std::abs(result - expected) > 1.0) {
			std::cerr << "FAIL: fdp_qc expected " << expected
			          << ", got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// ============================================================================
// TestDifferentConfigs: various dbns configurations
// ============================================================================
int TestDifferentConfigs() {
	int nrOfFailedTestCases = 0;

	// dbns<9, 5> — wider first-base field
	{
		using Scalar = dbns<9, 5, uint16_t>;
		std::vector<Scalar> x = { Scalar(1.0), Scalar(2.0) };
		std::vector<Scalar> y = { Scalar(1.0), Scalar(1.0) };
		Scalar result = fdp(x, y);
		double expected = double(x[0]) + double(x[1]);
		if (std::abs(double(result) - expected) > 1.0) {
			std::cerr << "FAIL: dbns<9,5> fdp, expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	// dbns<10, 5> — balanced
	{
		using Scalar = dbns<10, 5, uint16_t>;
		std::vector<Scalar> x = { Scalar(4.0), Scalar(2.0), Scalar(1.0) };
		std::vector<Scalar> y = { Scalar(1.0), Scalar(1.0), Scalar(1.0) };
		Scalar result = fdp(x, y);
		double expected = double(x[0]) + double(x[1]) + double(x[2]);
		if (std::abs(double(result) - expected) > 2.0) {
			std::cerr << "FAIL: dbns<10,5> fdp, expected " << expected
			          << ", got " << double(result) << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "dbns fused dot product (FDP)";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';
	std::cout << std::string(60, '=') << '\n';

	nrOfFailedTestCases += ReportTestResult(TestQuireMul(), "dbns quire_mul", "unrounded product");
	nrOfFailedTestCases += ReportTestResult(TestBasicFdp(), "dbns fdp", "basic dot product");
	nrOfFailedTestCases += ReportTestResult(TestFdp1024(), "dbns fdp", "1024-element vectors");
	nrOfFailedTestCases += ReportTestResult(TestLimbBoundary(), "dbns quire", "limb-boundary carry/borrow");
	nrOfFailedTestCases += ReportTestResult(TestFdpStride(), "dbns fdp_stride", "strided dot product");
	nrOfFailedTestCases += ReportTestResult(TestFdpQc(), "dbns fdp_qc", "quire continuation");
	nrOfFailedTestCases += ReportTestResult(TestDifferentConfigs(), "dbns fdp", "different configs");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
