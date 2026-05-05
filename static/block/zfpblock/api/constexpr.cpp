// constexpr.cpp: compile-time tests for zfpblock (accessor subset)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Scope of this constexpr coverage:
//
// zfpblock combines:
//   * a static block-buffer container with simple integer accessors -- all
//     constexpr-promotable (this file exercises that subset);
//   * a 564-line ZFP transform codec (zfp_codec.hpp) using std::frexp /
//     std::ldexp / std::memset and bit-stream packing -- NOT yet constexpr;
//   * a std::vector-backed multi-block array (zfparray) -- NOT in scope.
//
// The codec promotion is deferred per the issue note in zfpblock_impl.hpp.
// This file locks in the constexpr contract for what IS achievable today.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <universal/number/zfpblock/zfpblock.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "zfpblock constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// zfpblock<float, 1>: 4-element 1D block of floats.
	// zfpblock<double, 2>: 4x4 = 16-element 2D block of doubles.
	// zfpblock<double, 3>: 4x4x4 = 64-element 3D block of doubles.
	using zfp_f1 = zfpblock<float, 1>;
	using zfp_d2 = zfpblock<double, 2>;
	using zfp_d3 = zfpblock<double, 3>;

	// ----------------------------------------------------------------------------
	// Static accessors: BLOCK_SIZE, MAX_BYTES, block_size(), dim().
	// ----------------------------------------------------------------------------
	{
		// zfp_block_size<Dim>::value = 4^Dim (4, 16, 64 for Dim 1/2/3)
		static_assert(zfp_f1::BLOCK_SIZE == 4u,  "constexpr zfpblock<float, 1>::BLOCK_SIZE == 4");
		static_assert(zfp_d2::BLOCK_SIZE == 16u, "constexpr zfpblock<double, 2>::BLOCK_SIZE == 16");
		static_assert(zfp_d3::BLOCK_SIZE == 64u, "constexpr zfpblock<double, 3>::BLOCK_SIZE == 64");

		static_assert(zfp_f1::block_size() == 4u,  "constexpr zfp_f1::block_size() == 4");
		static_assert(zfp_d3::block_size() == 64u, "constexpr zfp_d3::block_size() == 64");

		static_assert(zfp_f1::dim() == 1u, "constexpr zfp_f1::dim() == 1");
		static_assert(zfp_d2::dim() == 2u, "constexpr zfp_d2::dim() == 2");
		static_assert(zfp_d3::dim() == 3u, "constexpr zfp_d3::dim() == 3");

		// MAX_BYTES = upper bound on compressed buffer size; non-zero per Dim.
		static_assert(zfp_f1::MAX_BYTES > 0u, "constexpr zfp_f1::MAX_BYTES > 0");
		static_assert(zfp_d3::MAX_BYTES > 0u, "constexpr zfp_d3::MAX_BYTES > 0");
	}

	// ----------------------------------------------------------------------------
	// compute_limits is a private static helper, but the public-facing
	// behavior is reachable via the constexpr accessors.  We exercise the
	// accessor subset directly: a default-constructed zfpblock has _nbits=0,
	// so compressed_bits() / compressed_bytes() / compression_ratio() are all
	// constexpr-deterministic.
	// ----------------------------------------------------------------------------
	{
		// Wrap construction + accessor reads in a constexpr lambda so we
		// can value-initialize the zfpblock storage (default ctor leaves
		// the byte buffer indeterminate).
		constexpr size_t cx_nbits = []() {
			zfp_f1 b{};
			return b.compressed_bits();
		}();
		// Default constructed _nbits is uninitialized per the trivial ctor,
		// so we don't assert a specific value -- just that compressed_bits()
		// is constexpr-callable.  The cx_nbits value is whatever the
		// value-init produced (typically 0 for explicit `{}` aggregates).
		(void)cx_nbits;

		constexpr size_t cx_bytes = []() {
			zfp_f1 b{};
			return b.compressed_bytes();
		}();
		(void)cx_bytes;

		constexpr double cx_ratio = []() {
			zfp_f1 b{};
			return b.compression_ratio();
		}();
		// With _nbits == 0 (value-init), compression_ratio returns 0.0 by contract.
		static_assert(cx_ratio == 0.0, "constexpr compression_ratio for empty block == 0.0");
	}

	// ----------------------------------------------------------------------------
	// data() pointer is constexpr (returns &_buffer[0]).  Useful for
	// constexpr inspection of pre-populated blocks.  We just verify the
	// expression is well-formed at constant evaluation.
	// ----------------------------------------------------------------------------
	{
		// data() returns const uint8_t* pointing into the block's buffer.
		// In a constant expression, the pointer's value isn't directly
		// comparable across translation units, but the call must compile.
		constexpr bool has_data = []() {
			zfp_f1 b{};
			return b.data() != nullptr;
		}();
		static_assert(has_data, "constexpr data() returns valid pointer");
	}

	// ----------------------------------------------------------------------------
	// Mode and parameter accessors.  Default-constructed zfpblock has
	// _mode and _param value-initialized (zfp_mode is an enum, defaulted to
	// 0 = zfp_mode::fixed_rate; _param defaulted to 0.0).
	// ----------------------------------------------------------------------------
	{
		constexpr zfp_mode cx_mode = []() {
			zfp_f1 b{};
			return b.mode();
		}();
		(void)cx_mode;  // value depends on zfp_mode enum's first entry; not pinned

		constexpr double cx_param = []() {
			zfp_f1 b{};
			return b.param();
		}();
		static_assert(cx_param == 0.0, "constexpr default param() == 0.0");
	}

	std::cout << "zfpblock constexpr verification: "
	          << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception\n";
	return EXIT_FAILURE;
}
