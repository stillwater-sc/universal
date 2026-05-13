// constexpr.cpp: compile-time tests for zfparray (multi-block compressed array)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Scope of this constexpr coverage:
//
// zfparray<Real, Dim> is a multi-block compressed array layered on top of
// the single-block ZFP codec.  After PR #816 (this file) the container is
// usable in constant-evaluated contexts, building on the codec promotion
// from PR #815.
//
// Supported pattern: a constexpr lambda that constructs a zfparray,
// exercises the API, and returns -- the vector storage is freed at lambda
// exit, satisfying C++20's rule that constexpr allocations not persist
// beyond the constant expression.  Static-storage `constexpr zfparray`
// instances are NOT supported.

#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <universal/number/zfpblock/zfpblock.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "zfparray constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using zfparray_f1 = zfparray<float, 1>;

	// ----------------------------------------------------------------------------
	// Default-constructed zfparray has _n = 0, _rate = 0, no blocks.
	// All trivial accessors are constexpr.
	// ----------------------------------------------------------------------------
	{
		constexpr size_t cx_size = []() {
			zfparray_f1 a{};
			return a.size();
		}();
		static_assert(cx_size == 0u, "constexpr default zfparray::size() == 0");

		constexpr size_t cx_blocks = []() {
			zfparray_f1 a{};
			return a.num_blocks();
		}();
		static_assert(cx_blocks == 0u, "constexpr default zfparray::num_blocks() == 0");

		constexpr size_t cx_bytes = []() {
			zfparray_f1 a{};
			return a.compressed_bytes();
		}();
		static_assert(cx_bytes == 0u, "constexpr default zfparray::compressed_bytes() == 0");

		constexpr double cx_ratio = []() {
			zfparray_f1 a{};
			return a.compression_ratio();
		}();
		static_assert(cx_ratio == 0.0, "constexpr default zfparray::compression_ratio() == 0.0");
	}

	// ----------------------------------------------------------------------------
	// Sized-rate construction: zfparray(n, rate) sets up the compressed
	// store but leaves blocks zero-initialized.
	// ----------------------------------------------------------------------------
	{
		constexpr bool cx_sized_ctor = []() {
			zfparray_f1 a(8, 16.0);  // 8 elements, 16 bpv -> 2 blocks of 4
			if (a.size() != 8u) return false;
			if (a.num_blocks() != 2u) return false;
			if (a.rate() != 16.0) return false;
			// bytes_per_block = ceil(16.0 * 4 / 8) = 8
			if (a.bytes_per_block() != 8u) return false;
			// compressed_bytes = 2 * 8 = 16
			if (a.compressed_bytes() != 16u) return false;
			return true;
		}();
		static_assert(cx_sized_ctor, "constexpr zfparray(n, rate) sets up storage correctly");
	}

	// ----------------------------------------------------------------------------
	// Construct-from-source roundtrip: zfparray(n, rate, src) compresses
	// at construction; decompress recovers values within tolerance.
	// Exercises the full per-block codec pipeline (encode + decode)
	// at compile time, through the multi-block container.
	// ----------------------------------------------------------------------------
	{
		constexpr bool cx_compress_decompress = []() {
			float src[8] = { 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f, 128.0f };
			zfparray_f1 a(8, 32.0, src);  // 32 bpv -> 2 blocks, lossless-ish for powers of 2

			float dst[8] = {};
			a.decompress(dst);

			auto abs_diff = [](float x, float y) {
				float d = x - y;
				return d < 0.0f ? -d : d;
			};
			constexpr float tol_rel = 1.0f / (1u << 20);
			for (unsigned i = 0; i < 8; ++i) {
				float ref     = src[i];
				float ref_abs = ref < 0.0f ? -ref : ref;
				if (abs_diff(dst[i], ref) > tol_rel * ref_abs) return false;
			}
			return true;
		}();
		static_assert(cx_compress_decompress,
			"constexpr zfparray(n, rate, src) + decompress() roundtrips within tolerance");
	}

	// ----------------------------------------------------------------------------
	// set/get roundtrip through the write-back cache: set writes through
	// the cache, get reads back through the cache.  This exercises
	// load_block / write_back_cache / flush at constant evaluation.
	// ----------------------------------------------------------------------------
	{
		constexpr bool cx_set_get = []() {
			zfparray_f1 a(4, 32.0);  // 4 elements, single block
			a.set(0, 1.0f);
			a.set(1, 2.0f);
			a.set(2, 4.0f);
			a.set(3, 8.0f);
			a.flush();

			// All reads now go through the (clean) cache.
			auto abs_diff = [](float x, float y) {
				float d = x - y;
				return d < 0.0f ? -d : d;
			};
			constexpr float tol_rel = 1.0f / (1u << 20);
			float refs[4] = { 1.0f, 2.0f, 4.0f, 8.0f };
			for (size_t i = 0; i < 4; ++i) {
				float got = a(i);
				if (abs_diff(got, refs[i]) > tol_rel * refs[i]) return false;
			}
			return true;
		}();
		static_assert(cx_set_get,
			"constexpr zfparray set/flush/get through write-back cache");
	}

	// ----------------------------------------------------------------------------
	// Cross-block access: setting elements in different blocks exercises
	// cache eviction (load_block -> write_back_cache -> load_block).
	// ----------------------------------------------------------------------------
	{
		constexpr bool cx_cross_block = []() {
			zfparray_f1 a(8, 32.0);  // 8 elements, 2 blocks of 4
			a.set(0, 1.0f);  // block 0
			a.set(5, 32.0f); // block 1 (forces eviction of block 0)
			a.set(2, 4.0f);  // block 0 (forces eviction of block 1, re-load block 0)
			a.flush();

			auto abs_diff = [](float x, float y) {
				float d = x - y;
				return d < 0.0f ? -d : d;
			};
			constexpr float tol_rel = 1.0f / (1u << 20);
			if (abs_diff(a(0), 1.0f)  > tol_rel * 1.0f)  return false;
			if (abs_diff(a(2), 4.0f)  > tol_rel * 4.0f)  return false;
			if (abs_diff(a(5), 32.0f) > tol_rel * 32.0f) return false;
			return true;
		}();
		static_assert(cx_cross_block,
			"constexpr zfparray cross-block set/get with cache eviction");
	}

	std::cout << "zfparray constexpr verification: "
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
