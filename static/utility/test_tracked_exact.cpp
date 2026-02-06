// test_tracked_exact.cpp: verify TrackedExact error tracking
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#include <iostream>
#include <iomanip>
#include <cmath>
#include <universal/utility/tracked_exact.hpp>

using namespace sw::universal;

void test_basic_operations() {
	std::cout << "=== Basic Operations Test ===\n\n";

	TrackedExact<double> a = 1.0;
	TrackedExact<double> b = 1e-16;

	std::cout << "a = " << a.value() << " (error: " << a.error() << ")\n";
	std::cout << "b = " << b.value() << " (error: " << b.error() << ")\n";

	auto c = a + b;
	std::cout << "\na + b = " << c.value() << "\n";
	std::cout << "  Absolute error: " << c.error() << "\n";
	std::cout << "  Relative error: " << c.relative_error() << "\n";
	std::cout << "  Valid bits: " << c.valid_bits() << "\n";
	std::cout << "  Operations: " << c.operations() << "\n";
	std::cout << "  Is exact: " << (c.is_exact() ? "yes" : "no") << "\n";
}

void test_error_accumulation() {
	std::cout << "\n=== Error Accumulation Test ===\n\n";

	// Sum many small numbers - errors should accumulate
	TrackedExact<double> sum = 0.0;
	double small = 1e-10;
	int n = 1000;

	for (int i = 0; i < n; ++i) {
		sum += small;
	}

	std::cout << "Sum of " << n << " values of " << small << ":\n";
	std::cout << "  Value: " << sum.value() << "\n";
	std::cout << "  Expected: " << (n * small) << "\n";
	std::cout << "  Difference: " << std::abs(sum.value() - n * small) << "\n";
	std::cout << "  Tracked error: " << sum.error() << "\n";
	std::cout << "  Valid bits: " << sum.valid_bits() << "\n";
	std::cout << "  Operations: " << sum.operations() << "\n";
}

void test_multiplication_error() {
	std::cout << "\n=== Multiplication Error Test ===\n\n";

	TrackedExact<double> a = 1.0 + 1e-15;
	TrackedExact<double> b = 1.0 - 1e-15;

	auto c = a * b;  // Should be close to 1 - 1e-30

	std::cout << "a = " << std::setprecision(17) << a.value() << "\n";
	std::cout << "b = " << b.value() << "\n";
	std::cout << "a * b = " << c.value() << "\n";
	std::cout << "  Error: " << c.error() << "\n";
	std::cout << "  Expected: " << (1.0 - 1e-30) << "\n";
}

void test_cancellation() {
	std::cout << "\n=== Cancellation Test ===\n\n";

	// Catastrophic cancellation: (a + b) - b should equal a
	// but floating-point can lose precision
	TrackedExact<double> a = 1.0;
	TrackedExact<double> b = 1e16;

	auto c = a + b;
	auto d = c - b;  // Should be 1.0, but may not be

	std::cout << "a = " << a.value() << "\n";
	std::cout << "b = " << std::scientific << b.value() << "\n";
	std::cout << "(a + b) - b = " << std::fixed << d.value() << "\n";
	std::cout << "  Error from a: " << std::abs(d.value() - 1.0) << "\n";
	std::cout << "  Tracked error: " << std::scientific << d.error() << "\n";
	std::cout << "  Valid bits: " << std::fixed << d.valid_bits() << "\n";
}

void test_with_float() {
	std::cout << "\n=== Float Test ===\n\n";

	TrackedExact<float> a = 1.0f;
	TrackedExact<float> b = 1e-7f;

	auto c = a + b;
	std::cout << "float: a + b = " << c.value() << "\n";
	std::cout << "  Error: " << c.error() << "\n";
	std::cout << "  Valid bits: " << c.valid_bits() << "\n";
}

void test_dot_product() {
	std::cout << "\n=== Dot Product Error Tracking ===\n\n";

	// Compute dot product with error tracking
	const int n = 100;
	TrackedExact<double> dot = 0.0;

	for (int i = 0; i < n; ++i) {
		TrackedExact<double> ai = 1.0 / (i + 1);
		TrackedExact<double> bi = 1.0 / (i + 2);
		dot += ai * bi;
	}

	std::cout << "Dot product of 1/(i+1) * 1/(i+2) for i=0.." << (n-1) << ":\n";
	std::cout << "  Value: " << std::setprecision(15) << dot.value() << "\n";
	std::cout << "  Error bound: " << std::scientific << dot.error() << "\n";
	std::cout << "  Operations: " << dot.operations() << "\n";
	std::cout << "  Valid bits: " << std::fixed << dot.valid_bits() << "\n";
}

void test_report() {
	std::cout << "\n=== Report Test ===\n\n";

	TrackedExact<double> x = 3.14159265358979;
	auto y = x * x;
	auto z = sqrt(y);

	std::cout << "Computing sqrt(x^2) for x = pi:\n";
	z.report(std::cout);
}

int main() {
	std::cout << "TrackedExact Error Tracking Test\n";
	std::cout << "================================\n\n";

	test_basic_operations();
	test_error_accumulation();
	test_multiplication_error();
	test_cancellation();
	test_with_float();
	test_dot_product();
	test_report();

	std::cout << "\nTrackedExact: PASS\n";
	return 0;
}
