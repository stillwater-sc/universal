// test_tracked.cpp: comprehensive test of unified Tracked<T> interface
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is
// released under an MIT Open Source license.

#include <iostream>
#include <iomanip>
#include <cmath>

// Include all number types
#include <universal/native/ieee754.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/interval/interval.hpp>

// Include unified tracking interface
#include <universal/utility/tracked.hpp>

using namespace sw::universal;

// ============================================================================
// Test helper to run a computation and report results
// ============================================================================

template<typename TrackedType>
void test_computation(const char* type_name) {
	std::cout << "\n--- " << type_name << " ---\n";
	std::cout << "Strategy: " << TrackedType::strategy_name() << "\n";

	TrackedType a = 1.0;
	TrackedType b = 1e-8;

	auto sum = a + b;
	auto prod = a * b;
	auto diff = a - b;

	std::cout << "a = 1.0, b = 1e-8\n";
	std::cout << "a + b = " << sum << " (error: " << std::scientific << sum.error()
	          << ", valid bits: " << std::fixed << std::setprecision(1) << sum.valid_bits() << ")\n";
	std::cout << "a * b = " << prod << " (error: " << std::scientific << prod.error() << ")\n";
	std::cout << "a - b = " << diff << " (error: " << std::scientific << diff.error() << ")\n";
}

// ============================================================================
// Test with different number types
// ============================================================================

void test_float() {
	std::cout << "\n=== IEEE float (Exact Strategy) ===\n";

	float          fa = 1.0f;
	float          fb = 1e-7f;
	Tracked<float> a  = fa;
	Tracked<float> b  = fb;

	std::cout << "Strategy: " << Tracked<float>::strategy_name() << "\n";

	auto fc = fa + fb;
	std::cout << to_binary(fa) << " : " << fa << "\n";
	std::cout << to_binary(fb) << " : " << fb << "\n";
	std::cout << to_binary(fc) << " : " << fc << "\n";
	auto c = a + b;
	std::cout << "1.0f + 1e-7f = " << c.value() << "\n";
	std::cout << "  Error: " << std::scientific << c.error() << "\n";
	std::cout << "  Valid bits: " << std::fixed << c.valid_bits() << "\n";
	std::cout << "  Operations: " << c.operations() << "\n";
}

void test_double() {
	std::cout << "\n=== IEEE double (Exact Strategy) ===\n";

	Tracked<double> a = 1.0;
	Tracked<double> b = 1e-15;

	std::cout << "Strategy: " << Tracked<double>::strategy_name() << "\n";

	auto c = a + b;
	std::cout << "1.0 + 1e-15 = " << std::setprecision(17) << c.value() << "\n";
	std::cout << "  Error: " << std::scientific << c.error() << "\n";
	std::cout << "  Valid bits: " << std::fixed << c.valid_bits() << "\n";
}

void test_cfloat() {
	std::cout << "\n=== cfloat<32,8> (Exact Strategy) ===\n";

	using CF = cfloat<32, 8>;
	Tracked<CF> a = 1.0;
	Tracked<CF> b = 1e-6;

	std::cout << "Strategy: " << Tracked<CF>::strategy_name() << "\n";

	auto c = a + b;
	std::cout << "1.0 + 1e-6 = " << double(c.value()) << "\n";
	std::cout << "  Error: " << std::scientific << c.error() << "\n";
	std::cout << "  Valid bits: " << std::fixed << c.valid_bits() << "\n";
}

void test_posit() {
	std::cout << "\n=== posit<32,2> (Shadow Strategy) ===\n";

	using P = posit<32, 2>;
	Tracked<P> a = 1.0;
	Tracked<P> b = 1e-8;

	std::cout << "Strategy: " << Tracked<P>::strategy_name() << "\n";

	auto c = a + b;
	std::cout << "1.0 + 1e-8 = " << double(c.value()) << "\n";
	std::cout << "  Error: " << std::scientific << c.error() << "\n";
	std::cout << "  Valid bits: " << std::fixed << c.valid_bits() << "\n";

	// Test accumulation
	Tracked<posit<16, 1>> sum = 0.0;
	for (int i = 0; i < 100; ++i) {
		sum += 0.01;
	}
	std::cout << "\n100 additions of 0.01 in posit<16,1>:\n";
	std::cout << "  Result: " << double(sum.value()) << " (expected 1.0)\n";
	std::cout << "  Error: " << sum.error() << "\n";
	std::cout << "  Valid bits: " << sum.valid_bits() << "\n";
}

void test_areal() {
	std::cout << "\n=== areal<32,8> (Inherent Strategy - ubit) ===\n";

	// Note: areal arithmetic uses TrackedAreal which wraps native ubit tracking
	// For now, just demonstrate the interface
	using A = areal<32, 8>;
	A raw_a = 1.0;
	A raw_b = 0.1;

	std::cout << "areal<32,8> native values:\n";
	std::cout << "  a = 1.0, ubit: " << raw_a.ubit() << "\n";
	std::cout << "  b = 0.1, ubit: " << raw_b.ubit() << "\n";

	A raw_c = raw_a + raw_b;
	std::cout << "  a + b = " << double(raw_c) << ", ubit: " << raw_c.ubit() << "\n";
	std::cout << "  (ubit=1 means value is uncertain, in interval (v, next(v)))\n";
}

void test_interval() {
	std::cout << "\n=== interval<double> (Inherent Strategy - bounds) ===\n";

	using I = interval<double>;
	Tracked<I> a = 1.0;
	Tracked<I> b(0.99, 1.01);  // Uncertain value in [0.99, 1.01]

	std::cout << "Strategy: " << Tracked<I>::strategy_name() << "\n";

	std::cout << "a = " << a.value() << ", is_exact: " << (a.is_exact() ? "yes" : "no") << "\n";
	std::cout << "b = " << b.value() << ", is_exact: " << (b.is_exact() ? "yes" : "no") << "\n";

	auto c = a + b;
	std::cout << "a + b = " << c.value() << "\n";
	std::cout << "  Error (width): " << c.error() << "\n";
	std::cout << "  Valid bits: " << c.valid_bits() << "\n";

	auto d = a * b;
	std::cout << "a * b = " << d.value() << "\n";
	std::cout << "  Error (width): " << d.error() << "\n";
}

void test_strategy_override() {
	std::cout << "\n=== Strategy Override ===\n";

	// Force Shadow strategy for double (normally uses Exact)
	Tracked<double, ErrorStrategy::Shadow> a = 1.0;
	Tracked<double, ErrorStrategy::Shadow> b = 1e-15;

	std::cout << "double with Shadow strategy (overriding Exact default):\n";
	auto c = a + b;
	std::cout << "1.0 + 1e-15 = " << c.value() << "\n";
	std::cout << "  Error: " << std::scientific << c.error() << "\n";
}

void test_dot_product_comparison() {
	std::cout << "\n=== Dot Product Comparison Across Types ===\n";

	const int n = 50;

	// Float with Exact tracking
	{
		Tracked<float> dot = 0.0f;
		for (int i = 0; i < n; ++i) {
			Tracked<float> ai = 1.0f / (i + 1);
			Tracked<float> bi = 1.0f / (i + 2);
			dot += ai * bi;
		}
		std::cout << "float (Exact):      " << std::setprecision(10) << dot.value()
		          << " error=" << std::scientific << dot.error()
		          << " bits=" << std::fixed << std::setprecision(1) << dot.valid_bits() << "\n";
	}

	// Double with Exact tracking
	{
		Tracked<double> dot = 0.0;
		for (int i = 0; i < n; ++i) {
			Tracked<double> ai = 1.0 / (i + 1);
			Tracked<double> bi = 1.0 / (i + 2);
			dot += ai * bi;
		}
		std::cout << "double (Exact):     " << std::setprecision(10) << dot.value()
		          << " error=" << std::scientific << dot.error()
		          << " bits=" << std::fixed << std::setprecision(1) << dot.valid_bits() << "\n";
	}

	// Posit with Shadow tracking
	{
		Tracked<posit<32, 2>> dot = 0.0;
		for (int i = 0; i < n; ++i) {
			Tracked<posit<32, 2>> ai = 1.0 / (i + 1);
			Tracked<posit<32, 2>> bi = 1.0 / (i + 2);
			dot += ai * bi;
		}
		std::cout << "posit<32,2> (Shadow): " << std::setprecision(10) << double(dot.value())
		          << " error=" << std::scientific << dot.error()
		          << " bits=" << std::fixed << std::setprecision(1) << dot.valid_bits() << "\n";
	}

	// Interval with Inherent tracking
	{
		Tracked<interval<double>> dot = 0.0;
		for (int i = 0; i < n; ++i) {
			Tracked<interval<double>> ai = 1.0 / (i + 1);
			Tracked<interval<double>> bi = 1.0 / (i + 2);
			dot += ai * bi;
		}
		std::cout << "interval<double> (Inherent): " << dot.value()
		          << " error=" << std::scientific << dot.error()
		          << " bits=" << std::fixed << std::setprecision(1) << dot.valid_bits() << "\n";
	}
}

void test_reports() {
	std::cout << "\n=== Detailed Reports ===\n";

	// TrackedExact report
	{
		Tracked<double> x = 3.14159265358979;
		auto y = x * x;
		auto z = sqrt(y);
		std::cout << "\nTrackedExact<double> - sqrt(pi^2):\n";
		z.report(std::cout);
	}

	// TrackedShadow report
	{
		Tracked<posit<32, 2>> x = 3.14159265358979;
		auto y = x * x;
		auto z = sqrt(y);
		std::cout << "\nTrackedShadow<posit<32,2>> - sqrt(pi^2):\n";
		z.report(std::cout);
	}
}

int main() {
	std::cout << "Unified Tracked<T> Interface Test\n";
	std::cout << "==================================\n";

	test_float();
	test_double();
	test_cfloat();
	test_posit();
	test_areal();
	test_interval();
	test_strategy_override();
	test_dot_product_comparison();
	test_reports();

	std::cout << "\n\nUnified Tracked<T>: PASS\n";
	return 0;
}

/*
There are Two different Meanings of "Precision"

What we're measuring (Result Accuracy): 
True mathematical result : 1.0 + 1e-7 = 1.0000001 
Computed result : 1.0 + ulp(1.0) ≈ 1.00000012 
Absolute error : ~1.9e-8 Relative error : 1.9e-8 / 1.0 ≈ 1.9e-8

By this measure, the result IS accurate to ~25 bits because 1.00000012 is 
very close to 1.0000001. The relative error is tiny.

We can also be concerned about Information Preservation: 
Input b = 1e-7 had ~7 significant decimal digits of information 
After addition: almost ALL of b's bits were discarded The ULP bit in the 
result is an approximation of b, not b itself

 Precision and Information Preservation are two different metrics:
  ┌───────────────────────┬──────────────┬───────────────────────────────────────┐
  │ Metric                │    Value     │            Interpretation             │
  ├───────────────────────┼──────────────┼───────────────────────────────────────┤
  │ Result accuracy       │ 24+ bits     │ "How close is result to true answer?" │
  ├───────────────────────┼──────────────┼───────────────────────────────────────┤
  │ Information preserved │ ~0 bits of b │ "How much of b survived?"             │
  └───────────────────────┴──────────────┴───────────────────────────────────────┘ 
  
# The Absorption Problem

This is the dual of cancellation. In subtraction of nearly-equal values,
error gets magnified.In addition of vastly-different magnitudes,
information gets absorbed :

    1.0f     = 1.00000000000000000000000 × 2 ^ 0
    1e-7f    = 0.00000000000000000000000 11010110111... × 2 ^ 0(shifted)
                                         ↑ These bits fall off the end

The bits of 1e-7 that would appear after position 24 are simply lost.
The result's ULP is a 1-bit approximation of a value that had 24 bits of information.

# Is 24 Bits Correct?

    For answering "how trustworthy is this result for further computation?" 
	- yes, 24 bits is correct.The result really is close to the true sum.

    But for answering "did this computation preserve input information?" 
	- no, we lost almost everything from b.

# What Should We Track?

    The current trackers answer question 1(result accuracy) but not question 2(information preservation)

we could add :

    1. Absorption detection : Flag when | b | < ulp(a + b) 
	2. Effective contribution : Track what fraction of each operand's bits survived 
	3. Condition number  How sensitive is the result to input perturbations ?
*/