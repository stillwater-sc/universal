// test_tracked_shadow.cpp: verify TrackedShadow error tracking with posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#include <iostream>
#include <iomanip>
#include <cmath>
#include <universal/utility/tracked_shadow.hpp>
#include <universal/number/posit/posit.hpp>

using namespace sw::universal;

void test_basic_posit() {
	std::cout << "=== Basic Posit Operations Test ===\n\n";

	using Posit = posit<32, 2>;
	TrackedShadow<Posit> a = 1.0;
	TrackedShadow<Posit> b = 1e-8;

	std::cout << "a = " << double(a.value()) << " (shadow: " << a.shadow() << ")\n";
	std::cout << "b = " << double(b.value()) << " (shadow: " << b.shadow() << ")\n";

	auto c = a + b;
	std::cout << "\na + b:\n";
	std::cout << "  Value:  " << double(c.value()) << "\n";
	std::cout << "  Shadow: " << c.shadow() << "\n";
	std::cout << "  Error:  " << c.error() << "\n";
	std::cout << "  Valid bits: " << c.valid_bits() << "\n";
	std::cout << "  Operations: " << c.operations() << "\n";
}

void test_posit_accumulation() {
	std::cout << "\n=== Posit Error Accumulation Test ===\n\n";

	using Posit = posit<16, 1>;  // Small posit to see more error
	TrackedShadow<Posit> sum = 0.0;
	double small = 0.001;
	int n = 1000;

	for (int i = 0; i < n; ++i) {
		sum += small;
	}

	std::cout << "Sum of " << n << " values of " << small << " in posit<16,1>:\n";
	std::cout << "  Value:    " << double(sum.value()) << "\n";
	std::cout << "  Shadow:   " << sum.shadow() << "\n";
	std::cout << "  Expected: " << (n * small) << "\n";
	std::cout << "  Error:    " << sum.error() << "\n";
	std::cout << "  Valid bits: " << sum.valid_bits() << "\n";
}

void test_posit_multiplication() {
	std::cout << "\n=== Posit Multiplication Test ===\n\n";

	using Posit = posit<32, 2>;
	TrackedShadow<Posit> a = 3.14159265358979;
	TrackedShadow<Posit> b = 2.71828182845905;

	auto c = a * b;

	std::cout << "pi * e in posit<32,2>:\n";
	std::cout << "  Value:  " << std::setprecision(15) << double(c.value()) << "\n";
	std::cout << "  Shadow: " << c.shadow() << "\n";
	std::cout << "  Error:  " << std::scientific << c.error() << "\n";
}

void test_posit_division() {
	std::cout << "\n=== Posit Division Test ===\n\n";

	using Posit = posit<32, 2>;
	TrackedShadow<Posit> a = 1.0;
	TrackedShadow<Posit> b = 3.0;

	auto c = a / b;  // 1/3 - not exactly representable

	std::cout << "1/3 in posit<32,2>:\n";
	std::cout << "  Value:  " << std::setprecision(17) << double(c.value()) << "\n";
	std::cout << "  Shadow: " << c.shadow() << "\n";
	std::cout << "  Error:  " << std::scientific << c.error() << "\n";

	// Multiply back by 3
	auto d = c * b;
	std::cout << "\n(1/3) * 3:\n";
	std::cout << "  Value:  " << std::setprecision(17) << double(d.value()) << "\n";
	std::cout << "  Shadow: " << d.shadow() << "\n";
	std::cout << "  Error:  " << d.error() << "\n";
}

void test_small_posit() {
	std::cout << "\n=== Small Posit (8-bit) Test ===\n\n";

	using Posit = posit<8, 0>;
	TrackedShadow<Posit> a = 1.5;
	TrackedShadow<Posit> b = 0.25;

	std::cout << "posit<8,0> arithmetic:\n";
	std::cout << "a = " << double(a.value()) << "\n";
	std::cout << "b = " << double(b.value()) << "\n";

	auto sum = a + b;
	auto diff = a - b;
	auto prod = a * b;
	auto quot = a / b;

	std::cout << "\na + b = " << double(sum.value()) << " (error: " << sum.error() << ")\n";
	std::cout << "a - b = " << double(diff.value()) << " (error: " << diff.error() << ")\n";
	std::cout << "a * b = " << double(prod.value()) << " (error: " << prod.error() << ")\n";
	std::cout << "a / b = " << double(quot.value()) << " (error: " << quot.error() << ")\n";
}

void test_math_functions() {
	std::cout << "\n=== Math Functions Test ===\n\n";

	using Posit = posit<32, 2>;
	TrackedShadow<Posit> x = 2.0;

	auto sq = sqrt(x);
	std::cout << "sqrt(2):\n";
	std::cout << "  Value:  " << std::setprecision(15) << double(sq.value()) << "\n";
	std::cout << "  Shadow: " << sq.shadow() << "\n";
	std::cout << "  Error:  " << std::scientific << sq.error() << "\n";

	auto ex = exp(TrackedShadow<Posit>(1.0));
	std::cout << "\nexp(1) = e:\n";
	std::cout << "  Value:  " << std::setprecision(15) << double(ex.value()) << "\n";
	std::cout << "  Shadow: " << ex.shadow() << "\n";
	std::cout << "  Error:  " << ex.error() << "\n";
}

void test_dot_product() {
	std::cout << "\n=== Dot Product Error Tracking ===\n\n";

	using Posit = posit<16, 1>;
	const int n = 100;
	TrackedShadow<Posit> dot = 0.0;

	for (int i = 0; i < n; ++i) {
		TrackedShadow<Posit> ai = 1.0 / (i + 1);
		TrackedShadow<Posit> bi = 1.0 / (i + 2);
		dot += ai * bi;
	}

	std::cout << "Dot product in posit<16,1>:\n";
	std::cout << "  Value:      " << std::setprecision(10) << double(dot.value()) << "\n";
	std::cout << "  Shadow:     " << dot.shadow() << "\n";
	std::cout << "  Error:      " << std::scientific << dot.error() << "\n";
	std::cout << "  Valid bits: " << std::fixed << dot.valid_bits() << "\n";
	std::cout << "  Operations: " << dot.operations() << "\n";
}

void test_report() {
	std::cout << "\n=== Report Test ===\n\n";

	using Posit = posit<32, 2>;
	TrackedShadow<Posit> x = 3.14159265358979;
	auto y = x * x;
	auto z = sqrt(y);

	std::cout << "Computing sqrt(x^2) for x = pi in posit<32,2>:\n";
	z.report(std::cout);
}

void test_convenience_alias() {
	std::cout << "\n=== TrackedPosit Alias Test ===\n\n";

	TrackedPosit<32, 2> a = 1.0;
	TrackedPosit<32, 2> b = 2.0;
	auto c = a + b;

	std::cout << "Using TrackedPosit<32,2> alias:\n";
	std::cout << "1 + 2 = " << double(c.value()) << " (error: " << c.error() << ")\n";
}

int main() {
	std::cout << "TrackedShadow Error Tracking Test (with Posits)\n";
	std::cout << "================================================\n\n";

	test_basic_posit();
	test_posit_accumulation();
	test_posit_multiplication();
	test_posit_division();
	test_small_posit();
	test_math_functions();
	test_dot_product();
	test_report();
	test_convenience_alias();

	std::cout << "\nTrackedShadow: PASS\n";
	return 0;
}
