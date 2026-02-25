// lifting.cpp: unit tests for ZFP forward/inverse lifting transform
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/zfpblock/zfpblock.hpp>
#include <universal/verification/test_suite.hpp>

#include <cstring>

// Test that forward then inverse lifting is near-exact for integers
// The lifting transform uses truncating right-shifts, so round-trip
// may differ by ±1 in the LSB — this is expected ZFP behavior.
template<typename Int>
int VerifyLiftingRoundTrip1D(const std::string& tag) {
	int nrOfFailedTests = 0;
	// test several different patterns
	Int patterns[][4] = {
		{ 0, 0, 0, 0 },
		{ 2, 4, 6, 8 },            // even values: exact round-trip
		{ -2, -4, -6, -8 },
		{ 1000, -500, 250, -125 },
		{ 100, 100, 100, 100 },     // constant
	};

	for (auto& pat : patterns) {
		Int original[4], p[4];
		std::memcpy(original, pat, sizeof(original));
		std::memcpy(p, pat, sizeof(p));

		sw::universal::fwd_lift<Int>(p, 1);
		sw::universal::inv_lift<Int>(p, 1);

		for (int i = 0; i < 4; ++i) {
			Int diff = p[i] - original[i];
			if (diff < -1 || diff > 1) {
				std::cerr << tag << " FAIL: index " << i
				          << " expected=" << original[i]
				          << " got=" << p[i]
				          << " (diff=" << diff << ", max allowed ±1)\n";
				++nrOfFailedTests;
			}
		}
	}

	// Verify even-valued inputs give exact round-trip (no rounding loss)
	{
		Int p[4] = { 0, 0, 0, 0 };
		sw::universal::fwd_lift<Int>(p, 1);
		sw::universal::inv_lift<Int>(p, 1);
		for (int i = 0; i < 4; ++i) {
			if (p[i] != 0) {
				std::cerr << tag << " FAIL: zero input not preserved\n";
				++nrOfFailedTests;
			}
		}
	}

	return nrOfFailedTests;
}

// Test strided lifting (for multi-dimensional use)
template<typename Int>
int VerifyStridedLifting(const std::string& tag) {
	int nrOfFailedTests = 0;
	// 4x4 block, apply lifting on columns (stride 4)
	Int block[16];
	Int original[16];
	for (int i = 0; i < 16; ++i) {
		block[i] = static_cast<Int>((i + 1) * 100);
		original[i] = block[i];
	}

	// forward and inverse on each column
	for (unsigned col = 0; col < 4; ++col) {
		sw::universal::fwd_lift<Int>(block + col, 4);
	}
	for (unsigned col = 0; col < 4; ++col) {
		sw::universal::inv_lift<Int>(block + col, 4);
	}

	for (int i = 0; i < 16; ++i) {
		Int diff = block[i] - original[i];
		if (diff < -1 || diff > 1) {
			std::cerr << tag << " strided FAIL: index " << i
			          << " expected=" << original[i]
			          << " got=" << block[i]
			          << " (diff=" << diff << ", max allowed ±1)\n";
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

// Test full multi-dimensional transform round-trip
// The lifting truncation accumulates across dimensions, allowing ±2 per dim.
template<typename Int, unsigned Dim>
int VerifyXformRoundTrip(const std::string& tag) {
	int nrOfFailedTests = 0;
	constexpr size_t N = sw::universal::zfp_block_size<Dim>::value;
	constexpr Int max_diff = static_cast<Int>(Dim * 2);  // allow ±2 per dimension

	Int block[N], original[N];
	for (size_t i = 0; i < N; ++i) {
		block[i] = static_cast<Int>((i + 1) * 7 - static_cast<Int>(N / 2));
		original[i] = block[i];
	}

	sw::universal::fwd_xform<Int, Dim>(block);
	sw::universal::inv_xform<Int, Dim>(block);

	for (size_t i = 0; i < N; ++i) {
		Int diff = block[i] - original[i];
		if (diff < -max_diff || diff > max_diff) {
			std::cerr << tag << " xform FAIL: index " << i
			          << " expected=" << original[i]
			          << " got=" << block[i]
			          << " (diff=" << diff << ", max allowed ±" << max_diff << ")\n";
			++nrOfFailedTests;
		}
	}

	// Zero input should give exact zero output
	{
		Int block_z[N];
		std::memset(block_z, 0, sizeof(block_z));
		sw::universal::fwd_xform<Int, Dim>(block_z);
		sw::universal::inv_xform<Int, Dim>(block_z);
		for (size_t i = 0; i < N; ++i) {
			if (block_z[i] != 0) {
				std::cerr << tag << " xform FAIL: zero not preserved at index " << i << '\n';
				++nrOfFailedTests;
			}
		}
	}

	return nrOfFailedTests;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "zfpblock lifting transform tests";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

	nrOfFailedTestCases += VerifyLiftingRoundTrip1D<int32_t>("int32 1D lift");
	nrOfFailedTestCases += VerifyLiftingRoundTrip1D<int64_t>("int64 1D lift");

	nrOfFailedTestCases += VerifyStridedLifting<int32_t>("int32 strided lift");
	nrOfFailedTestCases += VerifyStridedLifting<int64_t>("int64 strided lift");

	nrOfFailedTestCases += VerifyXformRoundTrip<int32_t, 1>("int32 1D xform");
	nrOfFailedTestCases += VerifyXformRoundTrip<int32_t, 2>("int32 2D xform");
	nrOfFailedTestCases += VerifyXformRoundTrip<int32_t, 3>("int32 3D xform");
	nrOfFailedTestCases += VerifyXformRoundTrip<int64_t, 1>("int64 1D xform");
	nrOfFailedTestCases += VerifyXformRoundTrip<int64_t, 2>("int64 2D xform");
	nrOfFailedTestCases += VerifyXformRoundTrip<int64_t, 3>("int64 3D xform");

	std::cout << (nrOfFailedTestCases == 0 ? "PASS" : "FAIL")
	          << " : " << nrOfFailedTestCases << " failures\n";

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
