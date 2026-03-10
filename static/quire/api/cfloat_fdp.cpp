// cfloat_fdp.cpp: tests for cfloat fused dot product via generalized quire
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/fdp.hpp>
#include <universal/verification/test_suite.hpp>
#include <vector>
#include <cmath>

namespace sw { namespace universal {

// Test quire_mul produces correct unrounded products
int TestQuireMul() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	{
		Scalar a(3.0f), b(5.0f);
		auto product = quire_mul(a, b);
		// product should represent 15.0 exactly
		if (product.iszero() || product.isnan() || product.isinf()) {
			std::cerr << "FAIL: quire_mul(3, 5) should be normal\n";
			++nrOfFailedTestCases;
		}
		// accumulate in quire and extract
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != 15.0) {
			std::cerr << "FAIL: quire_mul(3, 5) should give 15.0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Test with negative operands
	{
		Scalar a(-2.0f), b(7.0f);
		auto product = quire_mul(a, b);
		quire<Scalar> q;
		q += product;
		double result = q.convert_to<double>();
		if (result != -14.0) {
			std::cerr << "FAIL: quire_mul(-2, 7) should give -14.0, got " << result << '\n';
			++nrOfFailedTestCases;
		}
	}

	// Test with zero
	{
		Scalar a(0.0f), b(42.0f);
		auto product = quire_mul(a, b);
		if (!product.iszero()) {
			std::cerr << "FAIL: quire_mul(0, 42) should be zero\n";
			++nrOfFailedTestCases;
		}
	}

	// Test with NaN
	{
		Scalar a, b(1.0f);
		a.setnan();
		auto product = quire_mul(a, b);
		if (!product.isnan()) {
			std::cerr << "FAIL: quire_mul(NaN, 1) should be NaN\n";
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// Test basic fdp with simple vectors
int TestBasicFdp() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// dot product: [1, 2, 3, 4] . [5, 6, 7, 8] = 5 + 12 + 21 + 32 = 70
	std::vector<Scalar> a = { Scalar(1.0f), Scalar(2.0f), Scalar(3.0f), Scalar(4.0f) };
	std::vector<Scalar> b = { Scalar(5.0f), Scalar(6.0f), Scalar(7.0f), Scalar(8.0f) };

	Scalar result = fdp(a, b);
	double d = double(result);
	if (d != 70.0) {
		std::cerr << "FAIL: fdp([1,2,3,4], [5,6,7,8]) should be 70.0, got " << d << '\n';
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test fdp_stride
int TestFdpStride() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// stride-2 dot product: x[0]*y[0] + x[2]*y[2] = 1*5 + 3*7 = 26
	std::vector<Scalar> x = { Scalar(1.0f), Scalar(2.0f), Scalar(3.0f), Scalar(4.0f) };
	std::vector<Scalar> y = { Scalar(5.0f), Scalar(6.0f), Scalar(7.0f), Scalar(8.0f) };

	Scalar result = fdp_stride(size_t(4), x, size_t(2), y, size_t(2));
	double d = double(result);
	if (d != 26.0) {
		std::cerr << "FAIL: fdp_stride(4, x, 2, y, 2) should be 26.0, got " << d << '\n';
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test fdp_qc (quire continuation)
int TestFdpQc() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	std::vector<Scalar> x = { Scalar(1.0f), Scalar(2.0f), Scalar(3.0f) };
	std::vector<Scalar> y = { Scalar(4.0f), Scalar(5.0f), Scalar(6.0f) };
	// expected: 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32

	quire<Scalar> q;
	fdp_qc(q, size_t(3), x, size_t(1), y, size_t(1));

	double result = q.convert_to<double>();
	if (result != 32.0) {
		std::cerr << "FAIL: fdp_qc should give 32.0, got " << result << '\n';
		++nrOfFailedTestCases;
	}

	// Continue accumulating (quire continuation)
	std::vector<Scalar> x2 = { Scalar(1.0f), Scalar(1.0f) };
	std::vector<Scalar> y2 = { Scalar(8.0f), Scalar(10.0f) };
	// expected: 32 + 8 + 10 = 50
	fdp_qc(q, size_t(2), x2, size_t(1), y2, size_t(1));

	result = q.convert_to<double>();
	if (result != 50.0) {
		std::cerr << "FAIL: fdp_qc continuation should give 50.0, got " << result << '\n';
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test FDP with cancellation-heavy vectors
// The key advantage of FDP: it avoids catastrophic cancellation
int TestCancellationHeavy() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// Classic ill-conditioned dot product:
	// a = [1e7, 1, -1e7, 1]
	// b = [1,   1,  1,   1]
	// exact result: 1e7 + 1 - 1e7 + 1 = 2.0
	// naive fp32 sum may lose the small terms
	std::vector<Scalar> a = { Scalar(1e7f), Scalar(1.0f), Scalar(-1e7f), Scalar(1.0f) };
	std::vector<Scalar> b = { Scalar(1.0f), Scalar(1.0f), Scalar(1.0f),  Scalar(1.0f) };

	Scalar result = fdp(a, b);
	double d = double(result);
	if (d != 2.0) {
		std::cerr << "FAIL: cancellation-heavy fdp should be 2.0, got " << d << '\n';
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test FDP accuracy vs long double reference
int TestFdpAccuracy() {
	int nrOfFailedTestCases = 0;

	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	// Build vectors where naive fp32 accumulation would lose precision
	constexpr size_t N = 100;
	std::vector<Scalar> x(N), y(N);
	long double reference = 0.0L;
	for (size_t i = 0; i < N; ++i) {
		float xv = static_cast<float>(i + 1) * 0.1f;
		float yv = static_cast<float>(N - i) * 0.1f;
		x[i] = Scalar(xv);
		y[i] = Scalar(yv);
		// compute reference using long double from the cfloat-rounded values
		reference += static_cast<long double>(float(x[i])) * static_cast<long double>(float(y[i]));
	}

	Scalar fdp_result = fdp(x, y);
	double fdp_d = double(fdp_result);

	// also compute naive fp32 dot product for comparison
	float naive = 0.0f;
	for (size_t i = 0; i < N; ++i) {
		naive += float(x[i]) * float(y[i]);
	}

	// FDP result should be at least as close to the reference as naive
	double fdp_error = std::abs(fdp_d - static_cast<double>(reference));
	double naive_error = std::abs(static_cast<double>(naive) - static_cast<double>(reference));

	std::cout << "  reference (long double): " << static_cast<double>(reference) << '\n';
	std::cout << "  fdp result:             " << fdp_d << '\n';
	std::cout << "  naive fp32 result:      " << naive << '\n';
	std::cout << "  fdp error:              " << fdp_error << '\n';
	std::cout << "  naive error:            " << naive_error << '\n';

	if (fdp_error > naive_error + 1e-6) {
		std::cerr << "FAIL: FDP should be at least as accurate as naive accumulation\n";
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test with different cfloat configurations
int TestDifferentConfigs() {
	int nrOfFailedTestCases = 0;

	// fp16-equivalent
	{
		using Scalar = cfloat<16, 5, uint16_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(2.0f), Scalar(3.0f) };
		std::vector<Scalar> y = { Scalar(4.0f), Scalar(5.0f) };
		// 2*4 + 3*5 = 8 + 15 = 23
		Scalar result = fdp(x, y);
		double d = double(result);
		if (d != 23.0) {
			std::cerr << "FAIL: fp16 fdp should be 23.0, got " << d << '\n';
			++nrOfFailedTestCases;
		}
	}

	// fp8-equivalent (quarter precision)
	// cfloat<8,2> has fbits=5, which gives quire_traits mbits=6, escale=22
	// Use small values that are exactly representable
	{
		using Scalar = cfloat<8, 4, uint8_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(1.0f), Scalar(2.0f) };
		std::vector<Scalar> y = { Scalar(1.0f), Scalar(1.0f) };
		// 1*1 + 2*1 = 3
		Scalar result = fdp(x, y);
		double d = double(result);
		if (d != 3.0) {
			std::cerr << "FAIL: fp8 fdp should be 3.0, got " << d << '\n';
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "cfloat fused dot product (FDP)";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';
	std::cout << std::string(60, '=') << '\n';

	nrOfFailedTestCases += ReportTestResult(TestQuireMul(), "cfloat quire_mul", "unrounded product");
	nrOfFailedTestCases += ReportTestResult(TestBasicFdp(), "cfloat fdp", "basic dot product");
	nrOfFailedTestCases += ReportTestResult(TestFdpStride(), "cfloat fdp_stride", "strided dot product");
	nrOfFailedTestCases += ReportTestResult(TestFdpQc(), "cfloat fdp_qc", "quire continuation");
	nrOfFailedTestCases += ReportTestResult(TestCancellationHeavy(), "cfloat fdp", "cancellation-heavy");
	nrOfFailedTestCases += ReportTestResult(TestFdpAccuracy(), "cfloat fdp", "accuracy vs long double");
	nrOfFailedTestCases += ReportTestResult(TestDifferentConfigs(), "cfloat fdp", "different configs");

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
