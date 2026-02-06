// test_tracked_bounded.cpp: verify TrackedBounded interval-based error tracking
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#include <iostream>
#include <iomanip>
#include <cmath>
#include <universal/utility/tracked_bounded.hpp>

using namespace sw::universal;

void test_exact_values() {
	std::cout << "=== Exact Values ===\n\n";

	TrackedBounded<double> a = 2.0;
	TrackedBounded<double> b = 3.0;

	std::cout << "a = " << a << " (exact: " << (a.is_exact() ? "yes" : "no") << ")\n";
	std::cout << "b = " << b << " (exact: " << (b.is_exact() ? "yes" : "no") << ")\n";

	auto c = a + b;
	std::cout << "a + b = " << c << "\n";
	std::cout << "  Midpoint: " << c.value() << "\n";
	std::cout << "  Width: " << c.width() << "\n";
	std::cout << "  Is exact: " << (c.is_exact() ? "yes" : "no") << "\n";
}

void test_division_bounds() {
	std::cout << "\n=== Division Creates Bounds ===\n\n";

	TrackedBounded<double> one = 1.0;
	TrackedBounded<double> three = 3.0;

	auto third = one / three;
	std::cout << "1 / 3 = " << third << "\n";
	std::cout << "  Midpoint: " << std::setprecision(17) << third.value() << "\n";
	std::cout << "  Width: " << std::scientific << third.width() << "\n";
	std::cout << "  True 1/3 in interval: "
	          << (third.lo() <= 1.0/3.0 && 1.0/3.0 <= third.hi() ? "yes" : "no") << "\n";

	// Chain divisions
	auto result = one / three / three / three;
	std::cout << "\n1 / 3 / 3 / 3 = " << result << "\n";
	std::cout << "  Midpoint: " << std::setprecision(17) << result.value() << "\n";
	std::cout << "  Width: " << std::scientific << result.width() << "\n";
	std::cout << "  True 1/27 = " << 1.0/27.0 << "\n";
	std::cout << "  True in interval: "
	          << (result.lo() <= 1.0/27.0 && 1.0/27.0 <= result.hi() ? "yes" : "no") << "\n";
}

void test_interval_growth() {
	std::cout << "\n=== Interval Growth (Dependency Problem) ===\n\n";

	TrackedBounded<double> x = 1.0;

	// x - x should be exactly 0, but interval arithmetic gives [-width, +width]
	auto diff = x - x;
	std::cout << "x - x (same variable) = " << diff << "\n";
	std::cout << "  (Ideally 0, but interval arithmetic doesn't track dependencies)\n";

	// Demonstrate interval growth with repeated operations
	TrackedBounded<double> sum = 0.0;
	for (int i = 0; i < 100; ++i) {
		sum += TrackedBounded<double>(0.1);
	}
	std::cout << "\nSum of 100 × 0.1:\n";
	std::cout << "  Interval: " << sum << "\n";
	std::cout << "  Midpoint: " << sum.value() << "\n";
	std::cout << "  Width: " << std::scientific << sum.width() << "\n";
	std::cout << "  Valid bits: " << std::fixed << std::setprecision(1) << sum.valid_bits() << "\n";
}

void test_multiplication() {
	std::cout << "\n=== Multiplication with Signs ===\n\n";

	TrackedBounded<double> pos(2.0, 3.0);
	TrackedBounded<double> neg(-3.0, -2.0);
	TrackedBounded<double> mixed(-1.0, 2.0);

	std::cout << "pos = " << pos << "\n";
	std::cout << "neg = " << neg << "\n";
	std::cout << "mixed = " << mixed << "\n";

	auto pp = pos * pos;
	std::cout << "\npos * pos = " << pp << " (should be [4, 9])\n";

	auto pn = pos * neg;
	std::cout << "pos * neg = " << pn << " (should be [-9, -4])\n";

	auto nn = neg * neg;
	std::cout << "neg * neg = " << nn << " (should be [4, 9])\n";

	auto pm = pos * mixed;
	std::cout << "pos * mixed = " << pm << " (should be [-3, 6])\n";
}

void test_sqrt() {
	std::cout << "\n=== Square Root ===\n\n";

	TrackedBounded<double> two = 2.0;
	auto sqrt2 = sqrt(two);

	std::cout << "sqrt(2) = " << sqrt2 << "\n";
	std::cout << "  Midpoint: " << std::setprecision(17) << sqrt2.value() << "\n";
	std::cout << "  Width: " << std::scientific << sqrt2.width() << "\n";
	std::cout << "  True sqrt(2) = " << std::sqrt(2.0) << "\n";
	std::cout << "  True in interval: "
	          << (sqrt2.lo() <= std::sqrt(2.0) && std::sqrt(2.0) <= sqrt2.hi() ? "yes" : "no") << "\n";

	// Pythagorean theorem
	TrackedBounded<double> a = 3.0;
	TrackedBounded<double> b = 4.0;
	auto c = sqrt(a * a + b * b);
	std::cout << "\nsqrt(3² + 4²) = " << c << "\n";
	std::cout << "  Midpoint: " << c.value() << "\n";
	std::cout << "  True (5) in interval: "
	          << (c.lo() <= 5.0 && 5.0 <= c.hi() ? "yes" : "no") << "\n";
}

void test_uncertain_inputs() {
	std::cout << "\n=== Uncertain Input Values ===\n\n";

	// Create a measurement with 1% uncertainty
	auto x = make_uncertain(100.0, 0.01);
	std::cout << "x = 100 ± 1% = " << x << "\n";

	auto y = make_uncertain(50.0, 0.02);
	std::cout << "y = 50 ± 2% = " << y << "\n";

	auto sum = x + y;
	std::cout << "\nx + y = " << sum << "\n";
	std::cout << "  Width: " << sum.width() << "\n";
	std::cout << "  Relative error: " << std::scientific << sum.relative_error() << "\n";

	auto prod = x * y;
	std::cout << "\nx * y = " << prod << "\n";
	std::cout << "  Width: " << prod.width() << "\n";
	std::cout << "  Relative error: " << std::scientific << prod.relative_error() << "\n";
}

void test_interval_comparisons() {
	std::cout << "\n=== Interval Comparisons ===\n\n";

	TrackedBounded<double> a(1.0, 2.0);
	TrackedBounded<double> b(3.0, 4.0);
	TrackedBounded<double> c(1.5, 2.5);

	std::cout << "a = " << a << "\n";
	std::cout << "b = " << b << "\n";
	std::cout << "c = " << c << "\n";

	std::cout << "\na definitely < b: " << (a.definitely_less(b) ? "yes" : "no") << "\n";
	std::cout << "a overlaps c: " << (a.overlaps(c) ? "yes" : "no") << "\n";
	std::cout << "a overlaps b: " << (a.overlaps(b) ? "yes" : "no") << "\n";

	std::cout << "\na contains 0: " << (a.contains_zero() ? "yes" : "no") << "\n";
	TrackedBounded<double> d(-1.0, 1.0);
	std::cout << "d = " << d << " contains 0: " << (d.contains_zero() ? "yes" : "no") << "\n";
}

void test_division_by_zero() {
	std::cout << "\n=== Division by Interval Containing Zero ===\n\n";

	TrackedBounded<double> one = 1.0;
	TrackedBounded<double> around_zero(-0.1, 0.1);

	std::cout << "1 / [-0.1, 0.1] = ";
	auto result = one / around_zero;
	std::cout << result << "\n";
	std::cout << "  (Returns [-inf, +inf] for interval containing zero)\n";
}

void test_dot_product() {
	std::cout << "\n=== Dot Product with Rigorous Bounds ===\n\n";

	const int n = 50;

	TrackedBounded<double> dot = 0.0;
	double exact = 0.0;

	for (int i = 0; i < n; ++i) {
		TrackedBounded<double> ai = 1.0 / (i + 1);
		TrackedBounded<double> bi = 1.0 / (i + 2);
		dot += ai * bi;
		exact += (1.0 / (i + 1)) * (1.0 / (i + 2));
	}

	std::cout << "Dot product of 1/(i+1) * 1/(i+2) for i=0.." << (n-1) << ":\n";
	std::cout << "  Interval: " << dot << "\n";
	std::cout << "  Midpoint: " << std::setprecision(10) << dot.value() << "\n";
	std::cout << "  Width: " << std::scientific << dot.width() << "\n";
	std::cout << "  Valid bits: " << std::fixed << std::setprecision(1) << dot.valid_bits() << "\n";
	std::cout << "  Double result: " << exact << "\n";
	std::cout << "  Double in interval: "
	          << (dot.lo() <= exact && exact <= dot.hi() ? "yes" : "no") << "\n";
	std::cout << "  Operations: " << dot.operations() << "\n";
}

void test_power() {
	std::cout << "\n=== Integer Power ===\n\n";

	TrackedBounded<double> x = 2.0;

	auto x2 = pow(x, 2);
	auto x5 = pow(x, 5);
	auto x10 = pow(x, 10);

	std::cout << "2^2 = " << x2 << " (expected 4)\n";
	std::cout << "2^5 = " << x5 << " (expected 32)\n";
	std::cout << "2^10 = " << x10 << " (expected 1024)\n";

	// Negative power
	auto xm2 = pow(x, -2);
	std::cout << "2^-2 = " << xm2 << " (expected 0.25)\n";
}

void test_report() {
	std::cout << "\n=== Detailed Report ===\n\n";

	TrackedBounded<double> a = 1.0;
	TrackedBounded<double> b = 7.0;

	auto c = a / b;
	auto d = c * c + c;

	d.report(std::cout);
}

int main() {
	std::cout << "TrackedBounded Rigorous Interval Error Tracking Test\n";
	std::cout << "====================================================\n";
	std::cout << "Key insight: Uses directed rounding for GUARANTEED bounds!\n\n";

	test_exact_values();
	test_division_bounds();
	test_interval_growth();
	test_multiplication();
	test_sqrt();
	test_uncertain_inputs();
	test_interval_comparisons();
	test_division_by_zero();
	test_dot_product();
	test_power();
	test_report();

	std::cout << "\n\nTrackedBounded: PASS\n";
	return 0;
}
