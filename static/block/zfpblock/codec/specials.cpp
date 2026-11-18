// specials.cpp: zfpblock with subnormal, inf and NaN values, and with rates out of range
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// #2097:
//   - a block of subnormal magnitudes wrapped its biased exponent: float {1e-40, ...} decoded as 1.16e37,
//     double 1e-310 as 3.2e306. The block exponent is clamped at 1 - ebias, as the ZFP reference does.
//   - inf and NaN went through an undefined integer cast: {inf, 1, 2, 3} decoded as {-8, 1, 2, 3}. As
//     decided, such a block is stored raw (exponent field 0), and decodes bit for bit; a fixed rate that
//     cannot hold the raw block decodes it as all NaN.
//   - a fixed rate below the header zeroed every block, and a negative or NaN rate was undefined. Such a
//     rate clamps to the header + 4 bits; a rate past the raw size is capped there.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <vector>
#define ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/zfpblock/zfpblock.hpp>
#include <universal/number/zfpblock/zfparray.hpp>
#include <universal/verification/test_suite.hpp>

#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace {

// a block of subnormal magnitudes decodes near its values, at full precision
template<typename Real, unsigned Dim>
int VerifySubnormalBlock(bool reportTestCases) {
	using namespace sw::universal;
	constexpr size_t N = zfp_block_size<Dim>::value;
	const Real tiny = std::numeric_limits<Real>::denorm_min();
	Real x[N], y[N];
	for (size_t i = 0; i < N; ++i) x[i] = tiny * static_cast<Real>(1000 + 37 * i) * ((i % 3 == 0) ? Real(-1) : Real(1));
	zfpblock<Real, Dim> b;
	b.compress_fixed_precision(x, zfp_type_traits<Real>::precision_bits);
	b.decompress(y);
	double err = 0, mag = 0;
	for (size_t i = 0; i < N; ++i) {
		err = std::isfinite(y[i]) ? std::fmax(err, std::fabs(double(y[i]) - double(x[i]))) : std::numeric_limits<double>::infinity();   // a NaN fails
		mag = std::fmax(mag, std::fabs(double(x[i])));
	}
	if (!(err <= mag)) {   // the values are 2^-149-scale; before the fix they came back near 2^120
		if (reportTestCases) std::cerr << "FAIL: subnormal block (Dim " << Dim << "): error " << err << " against magnitude " << mag << '\n';
		return 1;
	}
	return 0;
}

// a block with inf and NaN round-trips bit for bit in every lossy mode with room for the raw block
template<typename Real, unsigned Dim>
int VerifyNonfiniteBlock(bool reportTestCases) {
	using namespace sw::universal;
	using UInt = typename zfp_type_traits<Real>::UInt;
	constexpr size_t N = zfp_block_size<Dim>::value;
	int fails = 0;
	Real x[N], y[N];
	for (size_t i = 0; i < N; ++i) x[i] = static_cast<Real>(i) + Real(0.5);
	x[0] = std::numeric_limits<Real>::infinity();
	x[N - 1] = -std::numeric_limits<Real>::infinity();
	x[1] = std::numeric_limits<Real>::quiet_NaN();
	auto check = [&](zfpblock<Real, Dim>& b, const char* mode) {
		b.decompress(y);
		for (size_t i = 0; i < N; ++i) {
			if (std::bit_cast<UInt>(y[i]) != std::bit_cast<UInt>(x[i])) {
				if (reportTestCases) std::cerr << "FAIL: non-finite block (Dim " << Dim << ", " << mode << ") value " << i << ": " << y[i] << '\n';
				++fails;
				return;
			}
		}
	};
	zfpblock<Real, Dim> p, a, r, f;
	p.compress_fixed_precision(x, 12);
	check(p, "fixed precision");
	a.compress_fixed_accuracy(x, 1e-2);
	check(a, "fixed accuracy");
	r.compress_reversible(x);
	check(r, "reversible");
	f.compress_fixed_rate(x, 1e6);    // capped at the raw size: room for it
	check(f, "fixed rate, full");
	// a fixed rate too small for the raw block: all NaN
	zfpblock<Real, Dim> s;
	s.compress_fixed_rate(x, 4.0);
	s.decompress(y);
	for (size_t i = 0; i < N; ++i) {
		if (!std::isnan(y[i])) {
			if (reportTestCases) std::cerr << "FAIL: non-finite block at rate 4 (Dim " << Dim << ") is not all NaN\n";
			++fails;
			break;
		}
	}
	return fails;
}

// rates below the floor, negative, NaN and past the raw size are all well defined
template<typename Real, unsigned Dim>
int VerifyRates(bool reportTestCases) {
	using namespace sw::universal;
	constexpr size_t N = zfp_block_size<Dim>::value;
	constexpr size_t floor_bits = 1u + zfp_type_traits<Real>::ebits + 4u;
	constexpr size_t raw_bits = zfp_raw_block_bits<Real, Dim>();
	int fails = 0;
	Real x[N], y[N];
	for (size_t i = 0; i < N; ++i) x[i] = static_cast<Real>(1.0 + double(i));
	for (double rate : { 0.0, 0.1, -3.0, std::numeric_limits<double>::quiet_NaN(), -std::numeric_limits<double>::infinity() }) {   // all below the floor
		zfpblock<Real, Dim> b;
		const size_t bits = b.compress_fixed_rate(x, rate);
		b.decompress(y);
		bool finite = true;
		for (size_t i = 0; i < N; ++i) finite = finite && std::isfinite(y[i]);
		if (bits != floor_bits || !finite) {
			if (reportTestCases) std::cerr << "FAIL: rate " << rate << " (Dim " << Dim << "): " << bits << " bits, expected the floor " << floor_bits << '\n';
			++fails;
		}
	}
	for (double rate : { 1e6, std::numeric_limits<double>::infinity() }) {
		zfpblock<Real, Dim> b;
		const size_t bits = b.compress_fixed_rate(x, rate);
		if (bits != raw_bits) {
			if (reportTestCases) std::cerr << "FAIL: rate " << rate << " (Dim " << Dim << "): " << bits << " bits, expected the cap " << raw_bits << '\n';
			++fails;
		}
	}
	return fails;
}

// zfparray: an index past the array throws std::out_of_range instead of writing past the store
int VerifyArrayBounds(bool reportTestCases) {
	using namespace sw::universal;
	int fails = 0;
	zfparray<float, 1> a(8, 16.0);
	a.set(7, 1.0f);
	bool threwSet = false, threwGet = false;
	try { a.set(100, 1.0f); } catch (const std::out_of_range&) { threwSet = true; }
	try { (void)a(8); } catch (const std::out_of_range&) { threwGet = true; }
	if (!threwSet || !threwGet || a(7) != 1.0f) {
		if (reportTestCases) std::cerr << "FAIL: zfparray bounds: set threw " << threwSet << ", get threw " << threwGet << '\n';
		++fails;
	}
	// a rate below the floor clamps for an array too: the block is not zeroed
	zfparray<float, 1> low(4, 0.5);
	const float src[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
	low.compress(src);
	if (low.bytes_per_block() != (1u + 8u + 4u + 7u) / 8u) {
		if (reportTestCases) std::cerr << "FAIL: zfparray rate floor: " << low.bytes_per_block() << " bytes per block\n";
		++fails;
	}
	return fails;
}

// zfparray beyond <float, 1> (#2133): double and the 2D / 3D block shapes, with a partial last block.
// At the full rate (8 sizeof(Real) bits per value) the round trip is within a few units of the type's
// epsilon of the data; at a quarter of it the error is larger but bounded. The store holds
// ceil(n / BLOCK_SIZE) blocks of bytes_per_block() each, and set / get go through the block cache.
template<typename Real, unsigned Dim>
int VerifyArrayShape(bool reportTestCases) {
	using namespace sw::universal;
	using A = zfparray<Real, Dim>;
	int fails = 0;
	auto expect = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL: zfparray<" << (sizeof(Real) == 4 ? "float" : "double") << ", " << Dim << ">: " << what << '\n'; }
	};
	const size_t n = 3 * A::BLOCK_SIZE + 5;   // a partial last block
	std::vector<Real> src(n), out(n);
	for (size_t i = 0; i < n; ++i) src[i] = Real(std::sin(0.1 * static_cast<double>(i)) + 2.0);   // smooth, in [1, 3]
	auto maxError = [&](double rate) {
		A a(n, rate, src.data());
		a.decompress(out.data());
		double e = 0.0;
		for (size_t i = 0; i < n; ++i) {
			const double d = std::fabs(static_cast<double>(out[i]) - static_cast<double>(src[i]));
			if (!std::isfinite(d)) return std::numeric_limits<double>::infinity();   // a NaN or inf output fails
			e = std::max(e, d);
		}
		return e;
	};
	const double fullRate = 8.0 * sizeof(Real);
	const double eps = static_cast<double>(std::numeric_limits<Real>::epsilon());
	const double full = maxError(fullRate);
	expect(full <= 4.0 * eps * 3.0, "full-rate round trip within a few epsilon");
	const double quarter = maxError(fullRate / 4.0);
	expect(quarter >= full && quarter > 0.0 && quarter < 1.0, "quarter-rate error larger than full rate, nonzero, and bounded");
	A a(n, fullRate, src.data());
	const size_t blocks = (n + A::BLOCK_SIZE - 1) / A::BLOCK_SIZE;
	expect(a.size() == n && a.compressed_bytes() == blocks * a.bytes_per_block(), "storage is ceil(n / BLOCK_SIZE) blocks");
	// element access through the cache, across block boundaries and into the partial block
	double getError = 0.0;
	for (size_t i = 0; i < n; ++i) {
		const double d = std::fabs(static_cast<double>(a(i)) - static_cast<double>(src[i]));
		getError = std::isfinite(d) ? std::max(getError, d) : std::numeric_limits<double>::infinity();
		if (!std::isfinite(d)) break;
	}
	expect(getError <= 4.0 * eps * 3.0, "operator() reads the decompressed values");
	a.set(0, Real(7.25));
	a.set(n - 1, Real(-1.5));
	a.flush();
	expect(a(0) == Real(7.25) && a(n - 1) == Real(-1.5), "set then get, first and last element");
	bool threw = false;
	try { a.set(n, Real(1)); } catch (const std::out_of_range&) { threw = true; }
	expect(threw, "set past the end throws");
	return fails;
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

	std::string test_suite  = "zfpblock subnormal, inf, NaN and rate edges";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	float x[4] = { 1e-40f, 2e-40f, 3e-40f, 4e-40f }, y[4];
	zfpblock<float, 1> b;
	b.compress_fixed_precision(x, 32);
	b.decompress(y);
	std::cout << "1e-40 -> " << y[0] << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult((VerifySubnormalBlock<float, 1>(reportTestCases)),  "float 1D",  "subnormal block");
	nrOfFailedTestCases += ReportTestResult((VerifySubnormalBlock<float, 3>(reportTestCases)),  "float 3D",  "subnormal block");
	nrOfFailedTestCases += ReportTestResult((VerifySubnormalBlock<double, 2>(reportTestCases)), "double 2D", "subnormal block");
	nrOfFailedTestCases += ReportTestResult((VerifyNonfiniteBlock<float, 1>(reportTestCases)),  "float 1D",  "inf / NaN block");
	nrOfFailedTestCases += ReportTestResult((VerifyNonfiniteBlock<float, 2>(reportTestCases)),  "float 2D",  "inf / NaN block");
	nrOfFailedTestCases += ReportTestResult((VerifyNonfiniteBlock<double, 3>(reportTestCases)), "double 3D", "inf / NaN block");
	nrOfFailedTestCases += ReportTestResult((VerifyRates<float, 1>(reportTestCases)),           "float 1D",  "rate edges");
	nrOfFailedTestCases += ReportTestResult((VerifyRates<double, 3>(reportTestCases)),          "double 3D", "rate edges");
	nrOfFailedTestCases += ReportTestResult(VerifyArrayBounds(reportTestCases),                 "zfparray",  "bounds and rate floor");
	nrOfFailedTestCases += ReportTestResult(VerifyArrayShape<double, 1>(reportTestCases), "zfparray<double,1>", "round trip, storage, access");
	nrOfFailedTestCases += ReportTestResult(VerifyArrayShape<float , 2>(reportTestCases), "zfparray<float,2>",  "round trip, storage, access");
	nrOfFailedTestCases += ReportTestResult(VerifyArrayShape<double, 2>(reportTestCases), "zfparray<double,2>", "round trip, storage, access");
	nrOfFailedTestCases += ReportTestResult(VerifyArrayShape<float , 3>(reportTestCases), "zfparray<float,3>",  "round trip, storage, access");
	nrOfFailedTestCases += ReportTestResult(VerifyArrayShape<double, 3>(reportTestCases), "zfparray<double,3>", "round trip, storage, access");
#endif

#if REGRESSION_LEVEL_2
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
