// test_tracked_statistical.cpp: verify TrackedStatistical ULP-based error tracking
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#include <iostream>
#include <iomanip>
#include <cmath>
#include <universal/utility/tracked_statistical.hpp>

using namespace sw::universal;

void test_ulp_function() {
	std::cout << "=== ULP Function Tests ===\n\n";

	std::cout << "ulp(1.0)   = " << std::scientific << ulp(1.0) << "\n";
	std::cout << "ulp(2.0)   = " << ulp(2.0) << "\n";
	std::cout << "ulp(0.5)   = " << ulp(0.5) << "\n";
	std::cout << "ulp(1e10)  = " << ulp(1e10) << "\n";
	std::cout << "ulp(1e-10) = " << ulp(1e-10) << "\n";
	std::cout << "ulp(0.0)   = " << ulp(0.0) << " (denorm_min)\n";

	std::cout << "\nExpected ulp(1.0) ≈ 2.22e-16 (machine epsilon)\n";
	std::cout << "Actual epsilon    = " << std::numeric_limits<double>::epsilon() << "\n";
}

void test_basic_operations() {
	std::cout << "\n=== Basic Operations ===\n\n";

	TrackedStatDouble a = 1.0;
	TrackedStatDouble b = 1e-15;

	std::cout << "a = 1.0, b = 1e-15\n";
	std::cout << "Model: " << TrackedStatDouble::model_name() << "\n\n";

	auto sum = a + b;
	std::cout << "a + b = " << std::setprecision(17) << sum.value() << "\n";
	std::cout << "  ULP error: " << std::setprecision(3) << sum.ulp_error() << "\n";
	std::cout << "  Valid bits: " << std::setprecision(1) << sum.valid_bits() << "\n";
	std::cout << "  Operations: " << sum.operations() << "\n";

	auto prod = a * b;
	std::cout << "\na * b = " << std::scientific << prod.value() << "\n";
	std::cout << "  ULP error: " << std::setprecision(3) << prod.ulp_error() << "\n";

	auto quot = a / b;
	std::cout << "\na / b = " << quot.value() << "\n";
	std::cout << "  ULP error: " << std::setprecision(3) << quot.ulp_error() << "\n";
}

void test_error_accumulation() {
	std::cout << "\n=== Error Accumulation Comparison ===\n\n";

	const int n = 100;

	// Random walk model
	{
		TrackedStatistical<double, ErrorModel::RandomWalk> sum = 0.0;
		for (int i = 0; i < n; ++i) {
			sum += TrackedStatistical<double, ErrorModel::RandomWalk>(0.1);
		}
		std::cout << "100 additions (RandomWalk model):\n";
		std::cout << "  Value: " << std::setprecision(15) << sum.value() << "\n";
		std::cout << "  ULP error: " << std::setprecision(2) << sum.ulp_error() << "\n";
		std::cout << "  Expected sqrt(100) * 0.5 = " << std::sqrt(100.0) * 0.5 << " ULPs\n";
		std::cout << "  Valid bits: " << std::setprecision(1) << sum.valid_bits() << "\n";
	}

	// Linear model
	{
		TrackedStatistical<double, ErrorModel::Linear> sum = 0.0;
		for (int i = 0; i < n; ++i) {
			sum += TrackedStatistical<double, ErrorModel::Linear>(0.1);
		}
		std::cout << "\n100 additions (Linear model):\n";
		std::cout << "  Value: " << std::setprecision(15) << sum.value() << "\n";
		std::cout << "  ULP error: " << std::setprecision(2) << sum.ulp_error() << "\n";
		std::cout << "  Expected 100 * 0.5 = " << 100 * 0.5 << " ULPs\n";
		std::cout << "  Valid bits: " << std::setprecision(1) << sum.valid_bits() << "\n";
	}
}

void test_cancellation_detection() {
	std::cout << "\n=== Cancellation Detection ===\n\n";

	TrackedStatDouble a = 1.0;
	TrackedStatDouble b = 0.9999999;

	auto c = a - b;
	std::cout << "1.0 - 0.9999999 (near-cancellation):\n";
	std::cout << "  Value: " << std::scientific << c.value() << "\n";
	std::cout << "  ULP error: " << std::setprecision(2) << c.ulp_error() << "\n";
	std::cout << "  Valid bits: " << std::setprecision(1) << c.valid_bits() << "\n";

	// More severe cancellation
	TrackedStatDouble x = 1.0;
	TrackedStatDouble y = 0.9999999999999;

	auto z = x - y;
	std::cout << "\n1.0 - 0.9999999999999 (severe cancellation):\n";
	std::cout << "  Value: " << std::scientific << z.value() << "\n";
	std::cout << "  ULP error: " << std::setprecision(2) << z.ulp_error() << "\n";
	std::cout << "  Valid bits: " << std::setprecision(1) << z.valid_bits() << "\n";
}

void test_math_functions() {
	std::cout << "\n=== Mathematical Functions ===\n\n";

	TrackedStatDouble x = 2.0;

	auto s = sqrt(x);
	std::cout << "sqrt(2) = " << std::setprecision(17) << s.value() << "\n";
	std::cout << "  ULP error: " << std::setprecision(3) << s.ulp_error() << "\n";

	TrackedStatDouble angle = 0.5;
	auto sine = sin(angle);
	std::cout << "\nsin(0.5) = " << std::setprecision(17) << sine.value() << "\n";
	std::cout << "  ULP error: " << sine.ulp_error() << "\n";

	auto e = exp(TrackedStatDouble(1.0));
	std::cout << "\nexp(1) = " << e.value() << "\n";
	std::cout << "  ULP error: " << e.ulp_error() << "\n";

	auto ln = log(TrackedStatDouble(2.0));
	std::cout << "\nlog(2) = " << ln.value() << "\n";
	std::cout << "  ULP error: " << ln.ulp_error() << "\n";
}

void test_power() {
	std::cout << "\n=== Integer Power ===\n\n";

	TrackedStatDouble x = 2.0;

	auto x2 = pow(x, 2);
	auto x5 = pow(x, 5);
	auto x10 = pow(x, 10);

	std::cout << "2^2 = " << x2.value() << " (ULP error: " << x2.ulp_error() << ")\n";
	std::cout << "2^5 = " << x5.value() << " (ULP error: " << x5.ulp_error() << ")\n";
	std::cout << "2^10 = " << x10.value() << " (ULP error: " << x10.ulp_error() << ")\n";
}

void test_dot_product() {
	std::cout << "\n=== Dot Product Comparison ===\n\n";

	const int n = 100;

	// Random walk model
	{
		TrackedStatistical<double, ErrorModel::RandomWalk> dot = 0.0;
		for (int i = 0; i < n; ++i) {
			TrackedStatistical<double, ErrorModel::RandomWalk> ai = 1.0 / (i + 1);
			TrackedStatistical<double, ErrorModel::RandomWalk> bi = 1.0 / (i + 2);
			dot += ai * bi;
		}
		std::cout << "Dot product (RandomWalk model):\n";
		std::cout << "  Value: " << std::setprecision(10) << dot.value() << "\n";
		std::cout << "  ULP error: " << std::setprecision(2) << dot.ulp_error() << "\n";
		std::cout << "  Valid bits: " << std::setprecision(1) << dot.valid_bits() << "\n";
		std::cout << "  Operations: " << dot.operations() << "\n";
	}

	// Linear model
	{
		TrackedStatistical<double, ErrorModel::Linear> dot = 0.0;
		for (int i = 0; i < n; ++i) {
			TrackedStatistical<double, ErrorModel::Linear> ai = 1.0 / (i + 1);
			TrackedStatistical<double, ErrorModel::Linear> bi = 1.0 / (i + 2);
			dot += ai * bi;
		}
		std::cout << "\nDot product (Linear model):\n";
		std::cout << "  Value: " << std::setprecision(10) << dot.value() << "\n";
		std::cout << "  ULP error: " << std::setprecision(2) << dot.ulp_error() << "\n";
		std::cout << "  Valid bits: " << std::setprecision(1) << dot.valid_bits() << "\n";
		std::cout << "  Operations: " << dot.operations() << "\n";
	}
}

void test_validation() {
	std::cout << "\n=== Validation Against Shadow Computation ===\n\n";

	// Compute same thing with statistical tracking and shadow
	const int n = 50;
	TrackedStatDouble stat_sum = 0.0;
	long double shadow_sum = 0.0L;

	for (int i = 0; i < n; ++i) {
		stat_sum += TrackedStatDouble(0.1);
		shadow_sum += 0.1L;
	}

	auto validation = StatisticalValidation<double, ErrorModel::RandomWalk>::compute(
		stat_sum, static_cast<double>(shadow_sum));

	std::cout << "Sum of 50 × 0.1:\n";
	validation.report(std::cout);
}

void test_uncertain_comparison() {
	std::cout << "\n=== Uncertain Comparisons ===\n\n";

	TrackedStatDouble a = 1.0;
	TrackedStatDouble b = 1.0 + 1e-15;

	std::cout << "a = 1.0\n";
	std::cout << "b = 1.0 + 1e-15\n";
	std::cout << "a == b (value): " << (a.value() == b.value() ? "yes" : "no") << "\n";
	std::cout << "definitely_different: " << (a.definitely_different(b) ? "yes" : "no") << "\n";
	std::cout << "possibly_equal: " << (a.possibly_equal(b) ? "yes" : "no") << "\n";

	// After operations, error grows
	auto c = a + a + a;  // 3.0 with some error
	auto d = TrackedStatDouble(3.0);  // exactly 3.0

	std::cout << "\nc = a + a + a (has error)\n";
	std::cout << "d = 3.0 (exact)\n";
	std::cout << "c.ulp_error: " << c.ulp_error() << "\n";
	std::cout << "d.ulp_error: " << d.ulp_error() << "\n";
	std::cout << "definitely_different: " << (c.definitely_different(d) ? "yes" : "no") << "\n";
	std::cout << "possibly_equal: " << (c.possibly_equal(d) ? "yes" : "no") << "\n";
}

void test_report() {
	std::cout << "\n=== Detailed Report ===\n\n";

	TrackedStatDouble x = 3.14159265358979;
	auto y = x * x;
	auto z = sqrt(y);

	z.report(std::cout);
}

void test_float_vs_double() {
	std::cout << "\n=== Float vs Double ===\n\n";

	const int n = 100;

	// Float
	{
		TrackedStatFloat sum = 0.0f;
		for (int i = 0; i < n; ++i) {
			sum += TrackedStatFloat(0.1f);
		}
		std::cout << "float (100 additions of 0.1f):\n";
		std::cout << "  Value: " << std::setprecision(10) << sum.value() << "\n";
		std::cout << "  ULP error: " << sum.ulp_error() << "\n";
		std::cout << "  Valid bits: " << sum.valid_bits() << " / " << mantissa_bits<float>() << "\n";
	}

	// Double
	{
		TrackedStatDouble sum = 0.0;
		for (int i = 0; i < n; ++i) {
			sum += TrackedStatDouble(0.1);
		}
		std::cout << "\ndouble (100 additions of 0.1):\n";
		std::cout << "  Value: " << std::setprecision(17) << sum.value() << "\n";
		std::cout << "  ULP error: " << sum.ulp_error() << "\n";
		std::cout << "  Valid bits: " << sum.valid_bits() << " / " << mantissa_bits<double>() << "\n";
	}
}

int main() {
	std::cout << "TrackedStatistical ULP-Based Error Tracking Test\n";
	std::cout << "=================================================\n";
	std::cout << "Key insight: Fast approximate tracking using ULP statistics!\n\n";

	test_ulp_function();
	test_basic_operations();
	test_error_accumulation();
	test_cancellation_detection();
	test_math_functions();
	test_power();
	test_dot_product();
	test_validation();
	test_uncertain_comparison();
	test_report();
	test_float_vs_double();

	std::cout << "\n\nTrackedStatistical: PASS\n";
	return 0;
}
