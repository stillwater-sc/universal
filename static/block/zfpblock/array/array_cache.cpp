// cache.cpp: cache behavior tests for zfparray (ZFP compressed array container)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/zfpblock/zfpblock.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "zfparray cache tests";
	int nrOfFailedTestCases = 0;

	// use a high rate for accurate round-trips so we can isolate cache behavior
	constexpr double rate = 24.0;

	// test 1: cache eviction on cross-block access
	std::cout << "+---------    cache eviction   --------+\n";
	{
		// 12 elements = 3 blocks of 4 (1D, BLOCK_SIZE=4)
		constexpr size_t N = 12;
		float src[N];
		for (size_t i = 0; i < N; ++i) src[i] = static_cast<float>(i + 1);

		zfparray1f arr(N, rate, src);

		// access block 0 element
		float v0 = arr(0);
		// access block 2 element — triggers eviction of block 0
		float v8 = arr(8);
		// access block 0 again — triggers reload
		float v1 = arr(1);

		float err0 = std::abs(v0 - 1.0f);
		float err8 = std::abs(v8 - 9.0f);
		float err1 = std::abs(v1 - 2.0f);

		bool pass = (err0 < 0.1f) && (err8 < 0.1f) && (err1 < 0.1f);
		if (!pass) {
			std::cerr << "FAIL: cache eviction\n"
			          << "  v0=" << v0 << " (exp 1, err " << err0 << ")\n"
			          << "  v8=" << v8 << " (exp 9, err " << err8 << ")\n"
			          << "  v1=" << v1 << " (exp 2, err " << err1 << ")\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "cache eviction and reload: PASS\n";
		}
	}

	// test 2: dirty write-back persists changes across eviction
	std::cout << "+---------    dirty write-back   --------+\n";
	{
		constexpr size_t N = 8;
		float src[N];
		for (size_t i = 0; i < N; ++i) src[i] = static_cast<float>(i + 1);

		zfparray1f arr(N, rate, src);

		// modify element in block 0
		arr.set(0, 99.0f);

		// access block 1 — evicts block 0, triggering write-back
		float v4 = arr(4);
		(void)v4;

		// access block 0 again — should reload with the modified value
		float v0 = arr(0);
		float err = std::abs(v0 - 99.0f);

		if (err > 0.5f) {
			std::cerr << "FAIL: dirty write-back, v0=" << v0
			          << " (expected ~99, err " << err << ")\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "dirty write-back persists: PASS (v0=" << v0 << ")\n";
		}
	}

	// test 3: flush() writes back without evicting
	std::cout << "+---------    flush   --------+\n";
	{
		constexpr size_t N = 4;
		float src[N] = { 1.0f, 2.0f, 3.0f, 4.0f };

		zfparray1f arr(N, rate, src);

		// modify and flush
		arr.set(0, 42.0f);
		arr.flush();

		// same block should still be cached — reading should return the cached value
		float v0 = arr(0);
		float err = std::abs(v0 - 42.0f);

		if (err > 0.5f) {
			std::cerr << "FAIL: flush, v0=" << v0 << " (expected ~42, err " << err << ")\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "flush writes back: PASS (v0=" << v0 << ")\n";
		}

		// also verify via bulk decompress
		float dst[N];
		arr.decompress(dst);
		float err_bulk = std::abs(dst[0] - 42.0f);
		if (err_bulk > 0.5f) {
			std::cerr << "FAIL: flush verify via decompress, dst[0]=" << dst[0]
			          << " (expected ~42, err " << err_bulk << ")\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "flush verified via decompress: PASS\n";
		}
	}

	// test 4: clear_cache() invalidates
	std::cout << "+---------    clear_cache   --------+\n";
	{
		constexpr size_t N = 4;
		float src[N] = { 10.0f, 20.0f, 30.0f, 40.0f };

		zfparray1f arr(N, rate, src);

		// load block into cache
		float v0 = arr(0);
		(void)v0;

		// clear cache
		arr.clear_cache();

		// accessing again should reload from compressed store
		float v0_again = arr(0);
		float err = std::abs(v0_again - 10.0f);

		if (err > 0.5f) {
			std::cerr << "FAIL: clear_cache, v0=" << v0_again
			          << " (expected ~10, err " << err << ")\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "clear_cache and reload: PASS\n";
		}
	}

	// test 5: multiple set() calls within same block
	std::cout << "+---------    multiple sets in same block   --------+\n";
	{
		zfparray1f arr(4, rate);
		arr.set(0, 1.0f);
		arr.set(1, 2.0f);
		arr.set(2, 3.0f);
		arr.set(3, 4.0f);
		arr.flush();

		bool pass = true;
		for (size_t i = 0; i < 4; ++i) {
			float expected = static_cast<float>(i + 1);
			float val = arr(i);
			float err = std::abs(val - expected);
			if (err > 0.5f) {
				std::cerr << "FAIL: multi-set, arr(" << i << ")=" << val
				          << " (expected " << expected << ", err " << err << ")\n";
				pass = false;
				++nrOfFailedTestCases;
			}
		}
		if (pass) std::cout << "multiple sets in same block: PASS\n";
	}

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
