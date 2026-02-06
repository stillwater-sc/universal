// test_tracked_lns.cpp: verify TrackedLNS specialized error tracking
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#include <iostream>
#include <iomanip>
#include <cmath>
#include <universal/utility/tracked_lns.hpp>
#include <universal/number/lns/lns.hpp>

using namespace sw::universal;

void test_multiplication_exact() {
	std::cout << "=== Multiplication is EXACT in LNS ===\n\n";

	using LNS = lns<32, 8>;
	TrackedLNS<LNS> a = 2.0;
	TrackedLNS<LNS> b = 3.0;

	std::cout << "a = " << double(a.value()) << "\n";
	std::cout << "b = " << double(b.value()) << "\n";

	// Chain of multiplications - should be EXACT
	auto c = a * b;
	auto d = c * a;
	auto e = d * b;

	std::cout << "\nChain: a * b * a * b = " << double(e.value()) << "\n";
	std::cout << "  Expected: " << (2.0 * 3.0 * 2.0 * 3.0) << "\n";
	std::cout << "  Error: " << e.error() << "\n";
	std::cout << "  Multiplications: " << e.multiplications() << " (EXACT)\n";
	std::cout << "  Additions: " << e.additions() << " (error source)\n";
	std::cout << "  Is exact: " << (e.is_exact() ? "yes" : "no") << "\n";
}

void test_addition_error() {
	std::cout << "\n=== Addition Introduces Error ===\n\n";

	using LNS = lns<16, 5>;  // Smaller LNS to see more error
	TrackedLNS<LNS> a = 1.0;
	TrackedLNS<LNS> b = 0.001;

	auto c = a + b;
	std::cout << "1.0 + 0.001 in lns<16,5>:\n";
	std::cout << "  Value: " << double(c.value()) << "\n";
	std::cout << "  Shadow: " << c.shadow() << "\n";
	std::cout << "  Error: " << std::scientific << c.error() << "\n";
	std::cout << "  Additions: " << c.additions() << "\n";
	std::cout << "  Multiplications: " << c.multiplications() << "\n";
}

void test_mixed_operations() {
	std::cout << "\n=== Mixed Operations ===\n\n";

	using LNS = lns<32, 8>;
	TrackedLNS<LNS> a = 2.0;
	TrackedLNS<LNS> b = 3.0;
	TrackedLNS<LNS> c = 0.5;

	// (a * b) + c - mix of exact and inexact
	auto result = (a * b) + c;

	std::cout << "(2 * 3) + 0.5 = " << double(result.value()) << "\n";
	std::cout << "  Expected: " << (2.0 * 3.0 + 0.5) << "\n";
	std::cout << "  Error: " << std::scientific << result.error() << "\n";
	std::cout << "  Multiplications: " << result.multiplications() << " (EXACT)\n";
	std::cout << "  Additions: " << result.additions() << " (error source)\n";
	std::cout << "  Exact ops ratio: "
	          << std::fixed << std::setprecision(1)
	          << (100.0 * result.exact_operations() / result.operations()) << "%\n";
}

void test_division_exact() {
	std::cout << "\n=== Division is EXACT in LNS ===\n\n";

	using LNS = lns<32, 8>;
	TrackedLNS<LNS> a = 12.0;
	TrackedLNS<LNS> b = 4.0;

	auto c = a / b;
	std::cout << "12 / 4 = " << double(c.value()) << "\n";
	std::cout << "  Error: " << c.error() << "\n";
	std::cout << "  Divisions: " << c.divisions() << " (EXACT)\n";

	// Multiple divisions
	auto d = c / b;  // 12/4/4 = 0.75
	std::cout << "(12 / 4) / 4 = " << double(d.value()) << "\n";
	std::cout << "  Error: " << d.error() << "\n";
	std::cout << "  Divisions: " << d.divisions() << " (EXACT)\n";
}

void test_cancellation() {
	std::cout << "\n=== Cancellation Detection ===\n\n";

	using LNS = lns<32, 8>;
	TrackedLNS<LNS> a = 1.0;
	TrackedLNS<LNS> b = 0.95;

	// Near-cancellation: a - b when a ≈ b
	auto c = a - b;
	std::cout << "1.0 - 0.95 (near-cancellation):\n";
	std::cout << "  Value: " << double(c.value()) << "\n";
	std::cout << "  Error: " << std::scientific << c.error() << "\n";
	std::cout << "  Cancellations detected: " << c.cancellations() << "\n";

	// Severe cancellation
	TrackedLNS<LNS> x = 1.0;
	TrackedLNS<LNS> y = 0.999;
	auto z = x - y;
	std::cout << "\n1.0 - 0.999 (severe cancellation):\n";
	std::cout << "  Value: " << double(z.value()) << "\n";
	std::cout << "  Error: " << std::scientific << z.error() << "\n";
	std::cout << "  Valid bits: " << std::fixed << z.valid_bits() << "\n";
	std::cout << "  Cancellations detected: " << z.cancellations() << "\n";
}

void test_power_exact() {
	std::cout << "\n=== Power (Integer) is EXACT ===\n\n";

	using LNS = lns<32, 8>;
	TrackedLNS<LNS> a = 2.0;

	auto a2 = pow(a, 2);
	auto a3 = pow(a, 3);
	auto a10 = pow(a, 10);

	std::cout << "2^2 = " << double(a2.value()) << " (error: " << a2.error() << ")\n";
	std::cout << "2^3 = " << double(a3.value()) << " (error: " << a3.error() << ")\n";
	std::cout << "2^10 = " << double(a10.value()) << " (error: " << a10.error() << ")\n";
	std::cout << "  Expected 2^10: " << (1 << 10) << "\n";
	std::cout << "  Multiplications for 2^10: " << a10.multiplications() << "\n";
	std::cout << "  Is exact: " << (a10.is_exact() ? "yes" : "no") << "\n";
}

void test_dot_product() {
	std::cout << "\n=== Dot Product Analysis ===\n\n";

	using LNS = lns<32, 8>;
	const int n = 100;

	TrackedLNS<LNS> dot = 0.0;
	for (int i = 0; i < n; ++i) {
		TrackedLNS<LNS> ai = 1.0 / (i + 1);
		TrackedLNS<LNS> bi = 1.0 / (i + 2);
		dot += ai * bi;  // mult is exact, add introduces error
	}

	std::cout << "Dot product of 1/(i+1) * 1/(i+2) for i=0.." << (n-1) << ":\n";
	std::cout << "  Value: " << std::setprecision(10) << double(dot.value()) << "\n";
	std::cout << "  Shadow: " << dot.shadow() << "\n";
	std::cout << "  Error: " << std::scientific << dot.error() << "\n";
	std::cout << "  Valid bits: " << std::fixed << dot.valid_bits() << "\n";
	std::cout << "\n  Operation breakdown:\n";
	std::cout << "    Multiplications: " << dot.multiplications() << " (EXACT - no error)\n";
	std::cout << "    Additions: " << dot.additions() << " (ERROR SOURCE)\n";
	std::cout << "    Exact ops: " << std::setprecision(1)
	          << (100.0 * dot.exact_operations() / dot.operations()) << "%\n";
}

void test_multiply_heavy() {
	std::cout << "\n=== Multiply-Heavy Algorithm (LNS Sweet Spot) ===\n\n";

	using LNS = lns<32, 8>;

	// Compute product of 1.01^100 (compound interest)
	TrackedLNS<LNS> result = 1.0;
	TrackedLNS<LNS> factor = 1.01;

	for (int i = 0; i < 100; ++i) {
		result = result * factor;  // All EXACT!
	}

	std::cout << "1.01^100 (compound interest):\n";
	std::cout << "  Value: " << std::setprecision(15) << double(result.value()) << "\n";
	std::cout << "  Expected: " << std::pow(1.01, 100) << "\n";
	std::cout << "  Error: " << std::scientific << result.error() << "\n";
	std::cout << "  Multiplications: " << result.multiplications() << " (ALL EXACT!)\n";
	std::cout << "  Additions: " << result.additions() << "\n";
	std::cout << "  Is exact: " << (result.is_exact() ? "yes" : "no") << "\n";
}

void test_report() {
	std::cout << "\n=== Detailed Report ===\n\n";

	using LNS = lns<32, 8>;
	TrackedLNS<LNS> a = 3.0;
	TrackedLNS<LNS> b = 4.0;

	// Pythagorean: sqrt(a^2 + b^2)
	auto a2 = a * a;        // EXACT
	auto b2 = b * b;        // EXACT
	auto sum = a2 + b2;     // Error here
	auto result = sqrt(sum);  // Error here too

	std::cout << "Pythagorean: sqrt(3^2 + 4^2) = " << double(result.value()) << "\n";
	std::cout << "Expected: 5.0\n\n";
	result.report(std::cout);
}

int main() {
	std::cout << "TrackedLNS Specialized Error Tracking Test\n";
	std::cout << "==========================================\n";
	std::cout << "Key insight: In LNS, multiplication/division are EXACT!\n";
	std::cout << "Only addition/subtraction introduce error.\n\n";

	test_multiplication_exact();
	test_addition_error();
	test_mixed_operations();
	test_division_exact();
	test_cancellation();
	test_power_exact();
	test_dot_product();
	test_multiply_heavy();
	test_report();

	std::cout << "\n\nTrackedLNS: PASS\n";
	return 0;
}
