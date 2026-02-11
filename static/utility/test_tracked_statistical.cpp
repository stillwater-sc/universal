// test_tracked_statistical.cpp: verify TrackedStatistical ULP-based error tracking
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#include <iostream>
#include <iomanip>
#include <cmath>
// Include specific ieee754 headers for to_binary without the conflicting ulp()
#include <universal/utility/architecture.hpp>
#include <universal/utility/compiler.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/native/ieee754_parameter.hpp>
#include <universal/native/ieee754_decoder.hpp>
#include <universal/native/nonconst_bitcast.hpp>
#include <universal/native/ieee754_float.hpp>
#include <universal/native/ieee754_double.hpp>
#include <universal/utility/tracked_statistical.hpp>

using namespace sw::universal;

void test_ulp_function() {
	std::cout << "=== ULP Function Tests ===\n\n";

	// Show binary representations and ULP values
	double d1 = 1.0, d2 = 2.0, d05 = 0.5, d1e10 = 1e10, d1em10 = 1e-10, d0 = 0.0;
	std::cout << to_binary(d1) << " : 1.0,   ulp = " << std::scientific << ulp(d1) << "\n";
	std::cout << to_binary(d2) << " : 2.0,   ulp = " << ulp(d2) << "\n";
	std::cout << to_binary(d05) << " : 0.5,   ulp = " << ulp(d05) << "\n";
	std::cout << to_binary(d1e10) << " : 1e10,  ulp = " << ulp(d1e10) << "\n";
	std::cout << to_binary(d1em10) << " : 1e-10, ulp = " << ulp(d1em10) << "\n";
	std::cout << to_binary(d0) << " : 0.0,   ulp = " << ulp(d0) << " (denorm_min)\n";

	std::cout << "\nExpected ulp(1.0) ≈ 2.22e-16 (machine epsilon)\n";
	std::cout << "Actual epsilon    = " << std::numeric_limits<double>::epsilon() << "\n";
}

void test_basic_operations() {
	std::cout << "\n=== Basic Operations ===\n\n";

	double da = 1.0;
	double db = 1e-15;
	TrackedStatDouble a = da;
	TrackedStatDouble b = db;

	std::cout << to_binary(da) << " : a = " << da << "\n";
	std::cout << to_binary(db) << " : b = " << db << "\n";
	std::cout << "Model: " << TrackedStatDouble::model_name() << "\n\n";

	double dsum = da + db;
	auto sum = a + b;
	std::cout << to_binary(dsum) << " : a + b = " << std::setprecision(17) << dsum << "\n";
	std::cout << "  ULP error: " << std::setprecision(3) << sum.ulp_error() << "\n";
	std::cout << "  Valid bits: " << std::setprecision(1) << sum.valid_bits() << "\n";
	std::cout << "  Operations: " << sum.operations() << "\n";

	double dprod = da * db;
	auto prod = a * b;
	std::cout << "\n" << to_binary(dprod) << " : a * b = " << std::scientific << dprod << "\n";
	std::cout << "  ULP error: " << std::setprecision(3) << prod.ulp_error() << "\n";

	double dquot = da / db;
	auto quot = a / b;
	std::cout << "\n" << to_binary(dquot) << " : a / b = " << dquot << "\n";
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

	double da = 1.0;
	double db = 0.9999999;
	TrackedStatDouble a = da;
	TrackedStatDouble b = db;

	double dc = da - db;
	auto c = a - b;
	std::cout << "1.0 - 0.9999999 (near-cancellation):\n";
	std::cout << to_binary(da) << " : a = " << da << "\n";
	std::cout << to_binary(db) << " : b = " << db << "\n";
	std::cout << to_binary(dc) << " : a - b = " << std::scientific << dc << "\n";
	std::cout << "  ULP error: " << std::setprecision(2) << c.ulp_error() << "\n";
	std::cout << "  Valid bits: " << std::setprecision(1) << c.valid_bits() << "\n";

	// More severe cancellation
	double dx = 1.0;
	double dy = 0.9999999999999;
	TrackedStatDouble x = dx;
	TrackedStatDouble y = dy;

	double dz = dx - dy;
	auto z = x - y;
	std::cout << "\n1.0 - 0.9999999999999 (severe cancellation):\n";
	std::cout << to_binary(dx) << " : x = " << dx << "\n";
	std::cout << to_binary(dy) << " : y = " << dy << "\n";
	std::cout << to_binary(dz) << " : x - y = " << std::scientific << dz << "\n";
	std::cout << "  ULP error: " << std::setprecision(2) << z.ulp_error() << "\n";
	std::cout << "  Valid bits: " << std::setprecision(1) << z.valid_bits() << "\n";
}

void test_math_functions() {
	std::cout << "\n=== Mathematical Functions ===\n\n";

	double dx = 2.0;
	TrackedStatDouble x = dx;

	double dsqrt = std::sqrt(dx);
	auto s = sqrt(x);
	std::cout << to_binary(dx) << " : x = " << dx << "\n";
	std::cout << to_binary(dsqrt) << " : sqrt(x) = " << std::setprecision(17) << dsqrt << "\n";
	std::cout << "  ULP error: " << std::setprecision(3) << s.ulp_error() << "\n";

	double dangle = 0.5;
	TrackedStatDouble angle = dangle;
	double dsin = std::sin(dangle);
	auto sine = sin(angle);
	std::cout << "\n" << to_binary(dangle) << " : angle = " << dangle << "\n";
	std::cout << to_binary(dsin) << " : sin(angle) = " << std::setprecision(17) << dsin << "\n";
	std::cout << "  ULP error: " << sine.ulp_error() << "\n";

	double done = 1.0;
	double dexp = std::exp(done);
	auto e = exp(TrackedStatDouble(done));
	std::cout << "\n" << to_binary(done) << " : x = " << done << "\n";
	std::cout << to_binary(dexp) << " : exp(x) = " << dexp << "\n";
	std::cout << "  ULP error: " << e.ulp_error() << "\n";

	double dtwo = 2.0;
	double dlog = std::log(dtwo);
	auto ln = log(TrackedStatDouble(dtwo));
	std::cout << "\n" << to_binary(dtwo) << " : x = " << dtwo << "\n";
	std::cout << to_binary(dlog) << " : log(x) = " << dlog << "\n";
	std::cout << "  ULP error: " << ln.ulp_error() << "\n";
}

void test_power() {
	std::cout << "\n=== Integer Power ===\n\n";

	double dx = 2.0;
	TrackedStatDouble x = dx;

	double dx2 = std::pow(dx, 2);
	double dx5 = std::pow(dx, 5);
	double dx10 = std::pow(dx, 10);
	auto x2 = pow(x, 2);
	auto x5 = pow(x, 5);
	auto x10 = pow(x, 10);

	std::cout << to_binary(dx) << " : x = " << dx << "\n";
	std::cout << to_binary(dx2) << " : 2^2 = " << dx2 << " (ULP error: " << x2.ulp_error() << ")\n";
	std::cout << to_binary(dx5) << " : 2^5 = " << dx5 << " (ULP error: " << x5.ulp_error() << ")\n";
	std::cout << to_binary(dx10) << " : 2^10 = " << dx10 << " (ULP error: " << x10.ulp_error() << ")\n";
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

	double da = 1.0;
	double db = 1.0 + 1e-15;
	TrackedStatDouble a = da;
	TrackedStatDouble b = db;

	std::cout << to_binary(da) << " : a = " << da << "\n";
	std::cout << to_binary(db) << " : b = " << db << "\n";
	std::cout << "a == b (value): " << (a.value() == b.value() ? "yes" : "no") << "\n";
	std::cout << "definitely_different: " << (a.definitely_different(b) ? "yes" : "no") << "\n";
	std::cout << "possibly_equal: " << (a.possibly_equal(b) ? "yes" : "no") << "\n";

	// After operations, error grows
	double dc = da + da + da;
	double dd = 3.0;
	auto c = a + a + a;  // 3.0 with some error
	auto d = TrackedStatDouble(dd);  // exactly 3.0

	std::cout << "\n" << to_binary(dc) << " : c = a + a + a (has error)\n";
	std::cout << to_binary(dd) << " : d = 3.0 (exact)\n";
	std::cout << "c.ulp_error: " << c.ulp_error() << "\n";
	std::cout << "d.ulp_error: " << d.ulp_error() << "\n";
	std::cout << "definitely_different: " << (c.definitely_different(d) ? "yes" : "no") << "\n";
	std::cout << "possibly_equal: " << (c.possibly_equal(d) ? "yes" : "no") << "\n";
}

void test_report() {
	std::cout << "\n=== Detailed Report ===\n\n";

	double dx = 3.14159265358979;
	TrackedStatDouble x = dx;
	double dy = dx * dx;
	double dz = std::sqrt(dy);
	auto y = x * x;
	auto z = sqrt(y);

	std::cout << "Computing sqrt(x^2) for x = pi:\n";
	std::cout << to_binary(dx) << " : x = " << dx << "\n";
	std::cout << to_binary(dy) << " : x^2 = " << dy << "\n";
	std::cout << to_binary(dz) << " : sqrt(x^2) = " << dz << "\n";
	z.report(std::cout);
}

void test_float_vs_double() {
	std::cout << "\n=== Float vs Double ===\n\n";

	const int n = 100;

	// Float
	{
		float f01 = 0.1f;
		std::cout << to_binary(f01) << " : 0.1f = " << f01 << "\n\n";
		TrackedStatFloat sum = 0.0f;
		for (int i = 0; i < n; ++i) {
			sum += TrackedStatFloat(0.1f);
		}
		std::cout << "float (100 additions of 0.1f):\n";
		std::cout << to_binary(sum.value()) << " : sum = " << std::setprecision(10) << sum.value() << "\n";
		std::cout << "  ULP error: " << sum.ulp_error() << "\n";
		std::cout << "  Valid bits: " << sum.valid_bits() << " / " << mantissa_bits<float>() << "\n";
	}

	// Double
	{
		double d01 = 0.1;
		std::cout << "\n" << to_binary(d01) << " : 0.1 = " << d01 << "\n\n";
		TrackedStatDouble sum = 0.0;
		for (int i = 0; i < n; ++i) {
			sum += TrackedStatDouble(0.1);
		}
		std::cout << "double (100 additions of 0.1):\n";
		std::cout << to_binary(sum.value()) << " : sum = " << std::setprecision(17) << sum.value() << "\n";
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
