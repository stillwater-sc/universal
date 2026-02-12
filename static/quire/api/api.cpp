// api.cpp: api tests for quire (super-accumulator) with posit scalar type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit arithmetic
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/test_suite.hpp>

/*
 * The quire is a super-accumulator that enables exact accumulation of
 * products for implementing fused dot product (FDP) operations.
 *
 * Key features:
 * - Fixed-point accumulator sized to hold the full dynamic range of posit products
 * - Supports += and -= for accumulating posit values or unrounded products
 * - Can convert back to posit with a single rounding operation
 * - Default capacity of 30 bits allows ~2^30 accumulations of maxpos^2
 */

namespace sw { namespace universal {

	// Demonstrate basic quire construction and properties
	void TestQuireConstruction() {
		std::cout << "Quire construction and properties\n";

		// Create a quire for posit<16,1>
		quire<16, 1> q16;
		std::cout << "quire<16,1> properties:\n";
		std::cout << "  total bits: " << q16.total_bits() << '\n';
		std::cout << "  max scale: " << q16.max_scale() << '\n';
		std::cout << "  min scale: " << q16.min_scale() << '\n';
		std::cout << "  capacity range: " << q16.capacity_range() << '\n';
		std::cout << "  is zero: " << (q16.iszero() ? "yes" : "no") << '\n';
		std::cout << '\n';

		// Create a quire for posit<32,2>
		quire<32, 2> q32;
		std::cout << "quire<32,2> properties:\n";
		std::cout << "  total bits: " << q32.total_bits() << '\n';
		std::cout << "  max scale: " << q32.max_scale() << '\n';
		std::cout << "  min scale: " << q32.min_scale() << '\n';
		std::cout << "  capacity range: " << q32.capacity_range() << '\n';
		std::cout << '\n';
	}

	// Demonstrate quire assignment from various types
	void TestQuireAssignment() {
		std::cout << "Quire assignment operations\n";

		quire<16, 1> q;

		// Assignment from native integers
		q = 1;
		std::cout << "q = 1: " << q << '\n';

		q = 100;
		std::cout << "q = 100: " << q << '\n';

		q = -50;
		std::cout << "q = -50: " << q << '\n';

		// Assignment from native floating-point
		q = 3.14159;
		std::cout << "q = 3.14159: " << q << '\n';

		// Assignment from posit
		posit<16, 1> p(2.5);
		q = p;
		std::cout << "q = posit(2.5): " << q << '\n';

		// Reset to zero
		q.clear();
		std::cout << "q.clear(): " << q << " is zero: " << (q.iszero() ? "yes" : "no") << '\n';
		std::cout << '\n';
	}

	// Demonstrate quire accumulation
	void TestQuireAccumulation() {
		std::cout << "Quire accumulation (+=, -=)\n";

		quire<16, 1> q;
		posit<16, 1> p;

		// Accumulate posit values
		p = 1.0;
		q += p;
		std::cout << "q += 1.0: " << q << '\n';

		p = 2.0;
		q += p;
		std::cout << "q += 2.0: " << q << '\n';

		p = 3.0;
		q += p;
		std::cout << "q += 3.0: " << q << '\n';

		// Subtract
		p = 1.5;
		q -= p;
		std::cout << "q -= 1.5: " << q << '\n';

		std::cout << '\n';
	}

	// Demonstrate fused dot product using quire
	void TestFusedDotProduct() {
		std::cout << "Fused Dot Product (FDP) example\n";

		using Posit = posit<16, 1>;
		using Quire = quire<16, 1>;

		// Two vectors to compute dot product
		Posit a[] = { Posit(1.0), Posit(2.0), Posit(3.0), Posit(4.0) };
		Posit b[] = { Posit(0.5), Posit(1.5), Posit(2.5), Posit(3.5) };
		constexpr int n = 4;

		// Traditional dot product (with rounding at each step)
		Posit traditional_sum(0);
		for (int i = 0; i < n; ++i) {
			traditional_sum += a[i] * b[i];
		}

		// FDP using quire (exact accumulation, single rounding at end)
		Quire q;
		for (int i = 0; i < n; ++i) {
			q += quire_mul(a[i], b[i]);  // unrounded product
		}
		Posit fdp_sum = q.convert_to<Posit>();

		std::cout << "Vector a: [1.0, 2.0, 3.0, 4.0]\n";
		std::cout << "Vector b: [0.5, 1.5, 2.5, 3.5]\n";
		std::cout << "Traditional dot product: " << traditional_sum << '\n';
		std::cout << "FDP using quire: " << fdp_sum << '\n';
		std::cout << "Expected: 1*0.5 + 2*1.5 + 3*2.5 + 4*3.5 = 0.5 + 3 + 7.5 + 14 = 25\n";
		std::cout << '\n';
	}

	// Demonstrate conversion back to posit
	void TestQuireToPositConversion() {
		std::cout << "Quire to Posit conversion\n";

		using Posit = posit<16, 1>;
		using Quire = quire<16, 1>;

		Quire q;
		Posit p;

		// Accumulate some values
		q = 10;
		q += Posit(5.5);
		q += Posit(2.25);

		// Convert back to posit
		p = q.convert_to<Posit>();
		std::cout << "Quire value: " << q << '\n';
		std::cout << "Converted to posit: " << p << '\n';
		std::cout << "Expected: 10 + 5.5 + 2.25 = 17.75\n";
		std::cout << '\n';
	}

}} // namespace sw::universal

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

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "quire API demonstration";
	std::string test_tag = "api";
	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	TestQuireConstruction();
	TestQuireAssignment();
	TestQuireAccumulation();
	TestFusedDotProduct();
	TestQuireToPositConversion();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	// Run demonstrations
	TestQuireConstruction();
	TestQuireAssignment();
	TestQuireAccumulation();
	TestFusedDotProduct();
	TestQuireToPositConversion();

	// Basic verification tests
	{
		using Posit = posit<16, 1>;
		using Quire = quire<16, 1>;

		// Test that a fresh quire is zero
		Quire q;
		if (!q.iszero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: fresh quire should be zero\n";
		}

		// Test assignment and accumulation
		q = 10;
		q += Posit(5);
		Posit result = q.convert_to<Posit>();
		if (double(result) != 15.0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 10 + 5 should be 15, got " << result << '\n';
		}

		// Test clear
		q.clear();
		if (!q.iszero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: quire should be zero after clear()\n";
		}

		// Test negative accumulation
		q = 10;
		q -= Posit(3);
		result = q.convert_to<Posit>();
		if (double(result) != 7.0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 10 - 3 should be 7, got " << result << '\n';
		}
	}

	nrOfFailedTestCases += ReportTestResult(0, "quire<16,1>", "api demonstration");
#endif

#if REGRESSION_LEVEL_2
	// Test with posit<32,2>
	{
		using Posit = posit<32, 2>;
		using Quire = quire<32, 2>;

		Quire q;
		q = 1000;
		q += Posit(500.5);
		Posit result = q.convert_to<Posit>();
		if (std::abs(double(result) - 1500.5) > 0.001) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: posit<32,2> quire test\n";
		}
	}
	nrOfFailedTestCases += ReportTestResult(0, "quire<32,2>", "api verification");
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Caught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Caught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
