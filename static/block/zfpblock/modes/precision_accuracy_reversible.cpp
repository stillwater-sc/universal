// precision_accuracy_reversible.cpp: ZFP's fixed-precision, fixed-accuracy and reversible modes keep their promises
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// #2097:
//   - the bit-plane coder skipped planes k >= maxprec, so it coded the BOTTOM maxprec planes: a fixed
//     precision below full dropped every significant bit. It codes the top maxprec planes, as ZFP does.
//   - fixed_accuracy ignored its tolerance; each block now codes emax - floor(log2(tol)) + 2 (Dim + 1)
//     planes, ZFP's rule, which keeps every error within the tolerance.
//   - reversible was the lossy pipeline at full precision: 0.1f came back as 0.099999994. A block the
//     pipeline does not reproduce bit for bit is stored raw.
//   - the non-rate modes were padded to their worst case (137 bits for 4 floats); only a fixed rate pads.
// The references are the input itself: a bound on the error for precision and accuracy, equality of the
// bit patterns for reversible.
#include <universal/utility/directives.hpp>
#define ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/zfpblock/zfpblock.hpp>
#include <universal/verification/test_suite.hpp>

#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>

namespace {

// a random block whose values span `spread` binades around 2^centre
template<typename Real, size_t N>
void random_block(std::mt19937_64& rng, Real* x, int centre, int spread) {
	std::uniform_real_distribution<double> m(-1.0, 1.0);
	for (size_t i = 0; i < N; ++i) x[i] = static_cast<Real>(std::ldexp(m(rng), centre - static_cast<int>(rng() % static_cast<unsigned>(spread + 1))));
}

// the exponent ZFP gives the block: frexp's exponent of the largest magnitude
template<typename Real, size_t N>
int block_exponent(const Real* x) {
	int emax = std::numeric_limits<int>::min();
	for (size_t i = 0; i < N; ++i) {
		if (x[i] == Real(0)) continue;
		int e = 0;
		(void)std::frexp(x[i], &e);
		if (e > emax) emax = e;
	}
	return emax;
}

// fixed precision p: every error is within 2^(emax - p + 2 (Dim + 1)), the bound ZFP's accuracy rule
// inverts; and more planes never cost fewer bits
template<typename Real, unsigned Dim>
int VerifyFixedPrecision(unsigned samples, bool reportTestCases) {
	using namespace sw::universal;
	constexpr size_t N = zfp_block_size<Dim>::value;
	constexpr unsigned full = zfp_type_traits<Real>::precision_bits;
	int fails = 0;
	std::mt19937_64 rng(2097);
	for (unsigned k = 0; k < samples; ++k) {
		Real x[N], y[N];
		random_block<Real, N>(rng, x, static_cast<int>(rng() % 40) - 20, 6);
		const int emax = block_exponent<Real, N>(x);
		size_t previous = 0;
		for (unsigned p : { 8u, 16u, full / 2u + 4u, full }) {
			zfpblock<Real, Dim> b;
			const size_t bits = b.compress_fixed_precision(x, p);
			b.decompress(y);
			double err = 0;
			for (size_t i = 0; i < N; ++i) err = std::fmax(err, std::fabs(double(y[i]) - double(x[i])));
			const double bound = std::ldexp(1.0, emax - static_cast<int>(p) + 2 * static_cast<int>(Dim + 1));
			if (err > bound || bits < previous) {
				if (reportTestCases && fails < 5) std::cerr << "FAIL: fixed precision " << p << " (Dim " << Dim << "): error " << err << " > " << bound << " or " << bits << " bits < " << previous << '\n';
				++fails;
			}
			previous = bits;
		}
	}
	return fails;
}

// fixed accuracy: every error within the tolerance, and a looser tolerance costs strictly fewer bits
// (the values sit above every tolerance, so each one codes a different number of planes; ignoring the
// tolerance coded every plane, with the same size for all three)
template<typename Real, unsigned Dim>
int VerifyFixedAccuracy(unsigned samples, bool reportTestCases) {
	using namespace sw::universal;
	constexpr size_t N = zfp_block_size<Dim>::value;
	int fails = 0;
	std::mt19937_64 rng(2098);
	for (unsigned k = 0; k < samples; ++k) {
		Real x[N], y[N];
		random_block<Real, N>(rng, x, 2 + static_cast<int>(rng() % 8), 1);   // |x| in [2^0, 2^10)
		size_t previous = 0;
		// float's 24 bits hold |x| < 2^10 to about 1e-4: its tolerances stop at 1e-3
		const double tols[3] = { 1e-1, std::is_same_v<Real, float> ? 1e-2 : 1e-3, std::is_same_v<Real, float> ? 1e-3 : 1e-5 };
		for (double tol : tols) {
			zfpblock<Real, Dim> b;
			const size_t bits = b.compress_fixed_accuracy(x, tol);
			b.decompress(y);
			double err = 0;
			for (size_t i = 0; i < N; ++i) err = std::fmax(err, std::fabs(double(y[i]) - double(x[i])));
			// a value below the tolerance may round to zero: still within it
			if (err > tol || bits <= previous) {
				if (reportTestCases && fails < 5) std::cerr << "FAIL: fixed accuracy " << tol << " (Dim " << Dim << "): error " << err << ", " << bits << " bits\n";
				++fails;
			}
			previous = bits;
		}
	}
	return fails;
}

// reversible: every block comes back bit for bit, -0, subnormals, a wide exponent spread and NaN payloads
// included
template<typename Real, unsigned Dim>
int VerifyReversible(unsigned samples, bool reportTestCases) {
	using namespace sw::universal;
	using UInt = typename zfp_type_traits<Real>::UInt;
	constexpr size_t N = zfp_block_size<Dim>::value;
	int fails = 0;
	std::mt19937_64 rng(2099);
	for (unsigned k = 0; k < samples; ++k) {
		Real x[N], y[N];
		switch (k % 4) {
		case 0: random_block<Real, N>(rng, x, 0, 2); break;                                                       // smooth
		case 1: random_block<Real, N>(rng, x, 0, 60); break;                                                      // wide spread
		case 2: for (size_t i = 0; i < N; ++i) x[i] = static_cast<Real>(0.1 * double(i + 1)); break;              // non-dyadic
		default:                                                                                                  // arbitrary bit patterns
			for (size_t i = 0; i < N; ++i) x[i] = std::bit_cast<Real>(static_cast<UInt>(rng()));
			x[0] = -Real(0);
			break;
		}
		zfpblock<Real, Dim> b;
		b.compress_reversible(x);
		b.decompress(y);
		for (size_t i = 0; i < N; ++i) {
			if (std::bit_cast<UInt>(y[i]) != std::bit_cast<UInt>(x[i])) {
				if (reportTestCases && fails < 5) std::cerr << "FAIL: reversible (Dim " << Dim << ", case " << k % 4 << ") value " << i << ": " << x[i] << " -> " << y[i] << '\n';
				++fails;
				break;
			}
		}
	}
	return fails;
}

// only a fixed rate pads: a non-rate block of 4 floats at 16 planes is smaller than the raw 137 bits
int VerifyNoPadding(bool reportTestCases) {
	using namespace sw::universal;
	const float x[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
	zfpblock<float, 1> p, a;
	const size_t pb = p.compress_fixed_precision(x, 16);
	const size_t ab = a.compress_fixed_accuracy(x, 1e-2);
	const bool ok = pb < 137 && ab < 137;
	if (!ok && reportTestCases) std::cerr << "FAIL: non-rate modes are padded: " << pb << " / " << ab << " bits\n";
	return ok ? 0 : 1;
}

}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
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

	std::string test_suite  = "zfpblock fixed-precision, fixed-accuracy and reversible modes";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	float x[4] = { 0.1f, 0.2f, 0.3f, 0.4f }, y[4];
	zfpblock<float, 1> b;
	b.compress_reversible(x);
	b.decompress(y);
	std::cout << "reversible 0.1f -> " << y[0] << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult((VerifyFixedPrecision<float, 1>(200, reportTestCases)),  "float 1D",  "fixed precision");
	nrOfFailedTestCases += ReportTestResult((VerifyFixedPrecision<float, 2>(100, reportTestCases)),  "float 2D",  "fixed precision");
	nrOfFailedTestCases += ReportTestResult((VerifyFixedPrecision<double, 3>(50, reportTestCases)),  "double 3D", "fixed precision");
	nrOfFailedTestCases += ReportTestResult((VerifyFixedAccuracy<float, 1>(200, reportTestCases)),   "float 1D",  "fixed accuracy");
	nrOfFailedTestCases += ReportTestResult((VerifyFixedAccuracy<float, 3>(50, reportTestCases)),    "float 3D",  "fixed accuracy");
	nrOfFailedTestCases += ReportTestResult((VerifyFixedAccuracy<double, 2>(100, reportTestCases)),  "double 2D", "fixed accuracy");
	nrOfFailedTestCases += ReportTestResult((VerifyReversible<float, 1>(400, reportTestCases)),      "float 1D",  "reversible");
	nrOfFailedTestCases += ReportTestResult((VerifyReversible<float, 3>(100, reportTestCases)),      "float 3D",  "reversible");
	nrOfFailedTestCases += ReportTestResult((VerifyReversible<double, 2>(200, reportTestCases)),     "double 2D", "reversible");
	nrOfFailedTestCases += ReportTestResult(VerifyNoPadding(reportTestCases),                        "float 1D",  "non-rate size");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyFixedPrecision<double, 1>(2000, reportTestCases)), "double 1D", "fixed precision");
	nrOfFailedTestCases += ReportTestResult((VerifyFixedAccuracy<double, 3>(500, reportTestCases)),   "double 3D", "fixed accuracy");
	nrOfFailedTestCases += ReportTestResult((VerifyReversible<double, 3>(500, reportTestCases)),      "double 3D", "reversible");
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
