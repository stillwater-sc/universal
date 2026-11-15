// ocp_scaling.cpp: mxblock::quantize picks the OCP MX v1.0 scale and rounds the elements to nearest even
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// OCP MX v1.0, section 6.3: X = 2^(floor(log2(amax)) - emax_elem), and each element is
// x_i / X rounded to the element type, to nearest with ties to even.  The reference below
// computes that in double, with its own table of emax values and its own exponent extraction
// (std::frexp), so it shares nothing with quantize() but the element codec:
//   - mxint8's emax is 6: an int8 code's largest power of two is 2^6.  With 7, every block's
//     largest code fell in [128, 256) and was clamped to 127 (#2083).
//   - int8 codes round ties to even.  std::round rounded 2.5 to 3 (#2083).
//   - floor(log2(amax)) is exact.  A float log2 rounds up just below a power of two.
//   - the scale exponent is clamped to e8m0's range, 2^-127 .. 2^127.  A reciprocal scale was
//     inf below 2^-127, so an mxfp8 block {2^-126, 0, 0} became {inf, NaN, NaN} (#2083).
//   - a NaN input sets the scale to NaN, so the whole block dequantizes to NaN.  An int8
//     element cast NaN to int, which is undefined.
#include <universal/utility/directives.hpp>
#include <universal/number/mxfloat/mxfloat.hpp>
#include <universal/verification/test_suite.hpp>

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

namespace {

// emax of each element type, OCP MX v1.0 Table 1 (int8 read as an integer code)
template<typename Elem> constexpr int ref_emax();
template<> constexpr int ref_emax<sw::universal::e2m1>()            { return 2; }
template<> constexpr int ref_emax<sw::universal::e2m3>()            { return 2; }
template<> constexpr int ref_emax<sw::universal::e3m2>()            { return 4; }
template<> constexpr int ref_emax<sw::universal::e4m3_saturating>() { return 8; }
template<> constexpr int ref_emax<sw::universal::e5m2>()            { return 15; }
template<> constexpr int ref_emax<std::int8_t>()                    { return 6; }

template<typename Elem>
unsigned element_bits(const Elem& e) {
	if constexpr (std::is_integral_v<Elem>) return static_cast<std::uint8_t>(e);
	else return static_cast<unsigned>(e.bits());
}

// the element the spec gives for x / 2^scale_exp, computed exactly in double
template<typename Elem>
unsigned reference_element(float x, int scale_exp) {
	const double q = std::ldexp(static_cast<double>(x), -scale_exp);   // exact
	if constexpr (std::is_integral_v<Elem>) {
		double r = std::nearbyint(q);                                    // ties to even
		if (r > 127.0) r = 127.0;
		if (r < -128.0) r = -128.0;
		return static_cast<std::uint8_t>(static_cast<std::int8_t>(r));
	}
	else {
		Elem e;
		e.from_float(static_cast<float>(q));
		return static_cast<unsigned>(e.bits());
	}
}

// quantize one block and compare its scale and elements with the reference
template<typename Block>
int VerifyBlock(const std::vector<float>& x, const std::string& tag, bool reportTestCases) {
	using Elem = std::remove_cv_t<std::remove_reference_t<decltype(std::declval<Block>().element(0))>>;
	Block b;
	b.quantize(x.data(), x.size());

	float amax = 0.0f;
	for (float v : x) amax = std::fabs(v) > amax ? std::fabs(v) : amax;
	int scale_exp = 0;
	if (amax == 0.0f) {
		scale_exp = -ref_emax<Elem>();
	}
	else {
		int e = 0;
		(void)std::frexp(static_cast<double>(amax), &e);                 // amax = m * 2^e, m in [0.5, 1)
		scale_exp = (e - 1) - ref_emax<Elem>();
		if (scale_exp < -127) scale_exp = -127;
		if (scale_exp > 127) scale_exp = 127;
	}

	int fails = 0;
	if (b.scale().bits() != static_cast<unsigned>(scale_exp + 127)) {
		if (reportTestCases) std::cerr << "FAIL " << tag << ": amax " << amax << " scale 2^" << (static_cast<int>(b.scale().bits()) - 127)
		                               << ", expected 2^" << scale_exp << '\n';
		++fails;
	}
	for (std::size_t i = 0; i < x.size(); ++i) {
		const unsigned got = element_bits(b.element(i));
		const unsigned ref = reference_element<Elem>(x[i], scale_exp);
		if (got != ref) {
			if (reportTestCases) std::cerr << "FAIL " << tag << ": x[" << i << "] = " << x[i] << " (amax " << amax << ") element 0x"
			                               << std::hex << got << ", expected 0x" << ref << std::dec << '\n';
			++fails;
		}
	}
	return fails;
}

// blocks at the edges of the scale range, near powers of two, and random blocks
template<typename Block>
int VerifyScaling(const std::string& tag, unsigned randomBlocks, bool reportTestCases) {
	int fails = 0;
	// just below and at powers of two: floor(log2(amax)) must not round up
	for (int k = -125; k <= 127; ++k) {
		const float p = std::ldexp(1.0f, k);
		const float below = std::nextafter(p, 0.0f);
		fails += VerifyBlock<Block>({ below, below / 3.0f, -below / 7.0f, 0.0f }, tag, reportTestCases);
		fails += VerifyBlock<Block>({ p, -p * 0.75f, p / 5.0f }, tag, reportTestCases);
	}
	// the bottom of the scale range: the scale clamps at 2^-127 and the elements stay finite
	fails += VerifyBlock<Block>({ 0x1p-126f, 0.0f, 0.0f }, tag, reportTestCases);
	fails += VerifyBlock<Block>({ 0x1p-127f, -0x1p-128f, 0.0f }, tag, reportTestCases);
	fails += VerifyBlock<Block>({ 0x1p-149f, 0.0f }, tag, reportTestCases);
	fails += VerifyBlock<Block>({ std::nextafter(0x1p-126f, 0.0f), 0x1p-130f, -0x1p-133f }, tag, reportTestCases);
	// the top of the scale range
	fails += VerifyBlock<Block>({ FLT_MAX, -FLT_MAX / 2.0f, 1.0f }, tag, reportTestCases);
	fails += VerifyBlock<Block>({ 0x1p127f, 0x1p120f, -0x1p126f }, tag, reportTestCases);
	// all zeros
	fails += VerifyBlock<Block>({ 0.0f, -0.0f, 0.0f }, tag, reportTestCases);

	std::mt19937 rng(2083);
	std::uniform_real_distribution<float> mant(-2.0f, 2.0f);
	std::uniform_int_distribution<int> centre(-140, 125), spread(0, 12);
	for (unsigned blk = 0; blk < randomBlocks; ++blk) {
		const int c = centre(rng);
		std::vector<float> x(Block::blockSize);
		for (auto& v : x) v = std::ldexp(mant(rng), c - spread(rng));
		fails += VerifyBlock<Block>(x, tag, reportTestCases);
	}
	return fails;
}

// a NaN input makes the whole block NaN: the scale is e8m0's NaN encoding, and every
// element dequantizes to NaN, wherever the NaN sits and whatever the other inputs are
template<typename Block>
int VerifyNaNBlock(const std::string& tag, bool reportTestCases) {
	const float nan = std::numeric_limits<float>::quiet_NaN();
	const std::vector<std::vector<float>> blocks = {
		{ nan },
		{ 1.0f, nan, -3.0f },
		{ 0.0f, 0.0f, nan },                                   // otherwise an all-zero block
		{ nan, std::numeric_limits<float>::infinity(), 2.0f },
		{ 0x1p-126f, -nan },
	};
	int fails = 0;
	for (const auto& x : blocks) {
		Block b;
		b.quantize(x.data(), x.size());
		std::vector<float> out(x.size(), 0.0f);
		b.dequantize(out.data(), out.size());
		bool allNaN = b.scale().isnan();
		for (std::size_t i = 0; i < out.size(); ++i) allNaN = allNaN && (out[i] != out[i]) && (b[i] != b[i]);
		if (!allNaN) {
			if (reportTestCases) std::cerr << "FAIL " << tag << ": a block holding a NaN has scale bits 0x" << std::hex
			                               << b.scale().bits() << std::dec << ", expected 0xff and NaN elements\n";
			++fails;
		}
	}
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

	std::string test_suite  = "mxblock OCP MX scaling and rounding";
	std::string test_tag    = "ocp scaling";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		float v[4] = { 100.0f, 1.5f, 2.5f, -3.25f };
		mxint8 b;
		b.quantize(v, 4);
		b.dequantize(v, 4);
		std::cout << "mxint8 {100, 1.5, 2.5, -3.25} -> {" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "}\n";
	}
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	{
		// the issue's block: 100 is not clipped, 2.5 rounds to 2
		float v[4] = { 100.0f, 1.5f, 2.5f, -3.25f };
		const float expected[4] = { 100.0f, 2.0f, 2.0f, -3.0f };
		mxint8 b;
		b.quantize(v, 4);
		b.dequantize(v, 4);
		for (int i = 0; i < 4; ++i) {
			if (v[i] != expected[i]) {
				std::cerr << "FAIL: mxint8 {100, 1.5, 2.5, -3.25}[" << i << "] = " << v[i] << ", expected " << expected[i] << '\n';
				++nrOfFailedTestCases;
			}
		}
	}
	{
		// int8 ties at scale 2^0: k + 0.5 for k in [-64, 63] rounds to the even neighbour.
		// The last element, 64, puts amax in [64, 128) so that the scale is 2^0.
		mxblock<std::int8_t, 129> b;
		float x[129];
		for (int k = -64; k < 64; ++k) x[k + 64] = static_cast<float>(k) + 0.5f;
		x[128] = 64.0f;
		b.quantize(x);
		if (b.scale().scale() != 0) {
			std::cerr << "FAIL: int8 ties block scale 2^" << b.scale().scale() << ", expected 2^0\n";
			++nrOfFailedTestCases;
		}
		for (int k = -64; k < 64; ++k) {
			const int expected = (k % 2 == 0) ? k : k + 1;   // the even one of k, k + 1
			if (b.element(static_cast<std::size_t>(k + 64)) != expected) {
				std::cerr << "FAIL: int8 tie " << x[k + 64] << " -> " << int(b.element(static_cast<std::size_t>(k + 64))) << ", expected " << expected << '\n';
				++nrOfFailedTestCases;
			}
		}
	}
	{
		// a block's largest element keeps 7 bits: amax in [64, 128) * 2^s round-trips to
		// within half a step unless it rounds up to 128, which saturates to 127
		int clipped = 0;
		for (int code = 64; code < 128; ++code) {
			for (int s : { -20, 0, 9 }) {
				const float amax = std::ldexp(static_cast<float>(code) + 0.25f, s);
				float v[2] = { amax, -amax / 3.0f };
				mxint8 b;
				b.quantize(v, 2);
				if (b.element(0) != static_cast<std::int8_t>(code)) ++clipped;
			}
		}
		if (clipped != 0) {
			std::cerr << "FAIL: mxint8 clipped " << clipped << " of 192 block maxima\n";
			++nrOfFailedTestCases;
		}
	}
	{
		// the issue's mxfp8 block below a 2^-127 scale: exact, no inf, no NaN
		float v[3] = { 0x1p-126f, 0.0f, 0.0f };
		mxfp8 b;
		b.quantize(v, 3);
		b.dequantize(v, 3);
		if (v[0] != 0x1p-126f || v[1] != 0.0f || v[2] != 0.0f) {
			std::cerr << "FAIL: mxfp8 {2^-126, 0, 0} -> {" << v[0] << ", " << v[1] << ", " << v[2] << "}\n";
			++nrOfFailedTestCases;
		}
	}
	{
		// floor(log2(amax)) just below a power of two: 16 - 2^-20 has shared exponent 3
		float v[1] = { std::nextafter(16.0f, 0.0f) };
		mxfp8 b;
		b.quantize(v, 1);
		if (b.scale().scale() != 3 - 8) {
			std::cerr << "FAIL: mxfp8 {16 - ulp} scale 2^" << b.scale().scale() << ", expected 2^-5\n";
			++nrOfFailedTestCases;
		}
	}

	nrOfFailedTestCases += ReportTestResult(VerifyNaNBlock<mxint8>("mxint8", reportTestCases),       "mxint8",    "NaN input");
	nrOfFailedTestCases += ReportTestResult(VerifyNaNBlock<mxfp8>("mxfp8", reportTestCases),         "mxfp8",     "NaN input");
	nrOfFailedTestCases += ReportTestResult(VerifyNaNBlock<mxfp8e5m2>("mxfp8e5m2", reportTestCases), "mxfp8e5m2", "NaN input");
	nrOfFailedTestCases += ReportTestResult(VerifyNaNBlock<mxfp6>("mxfp6", reportTestCases),         "mxfp6",     "NaN input");
	nrOfFailedTestCases += ReportTestResult(VerifyNaNBlock<mxfp6e2m3>("mxfp6e2m3", reportTestCases), "mxfp6e2m3", "NaN input");
	nrOfFailedTestCases += ReportTestResult(VerifyNaNBlock<mxfp4>("mxfp4", reportTestCases),         "mxfp4",     "NaN input");

	nrOfFailedTestCases += ReportTestResult(VerifyScaling<mxint8>("mxint8", 200, reportTestCases),        "mxint8",    test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyScaling<mxfp8>("mxfp8", 200, reportTestCases),          "mxfp8",     test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyScaling<mxfp8e5m2>("mxfp8e5m2", 200, reportTestCases),  "mxfp8e5m2", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyScaling<mxfp6>("mxfp6", 200, reportTestCases),          "mxfp6",     test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyScaling<mxfp6e2m3>("mxfp6e2m3", 200, reportTestCases),  "mxfp6e2m3", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyScaling<mxfp4>("mxfp4", 200, reportTestCases),          "mxfp4",     test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyScaling<mxint8>("mxint8", 5000, reportTestCases), "mxint8", test_tag + " random");
	nrOfFailedTestCases += ReportTestResult(VerifyScaling<mxfp8>("mxfp8", 5000, reportTestCases),   "mxfp8",  test_tag + " random");
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
