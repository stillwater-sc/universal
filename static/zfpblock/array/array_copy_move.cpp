// copy_move.cpp: copy and move semantics tests for zfparray (ZFP compressed array container)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/zfpblock/zfpblock.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "zfparray copy/move tests";
	int nrOfFailedTestCases = 0;

	constexpr double rate = 24.0;
	constexpr size_t N = 8;

	// build a reference array
	float src[N];
	for (size_t i = 0; i < N; ++i) src[i] = static_cast<float>(i + 1);

	// test 1: copy construction preserves data
	std::cout << "+---------    copy construction   --------+\n";
	{
		zfparray1f original(N, rate, src);
		zfparray1f copy(original);

		if (copy.size() != original.size()) {
			std::cerr << "FAIL: copy size mismatch\n";
			++nrOfFailedTestCases;
		}
		if (copy.rate() != original.rate()) {
			std::cerr << "FAIL: copy rate mismatch\n";
			++nrOfFailedTestCases;
		}

		float dst[N];
		copy.decompress(dst);
		bool pass = true;
		for (size_t i = 0; i < N; ++i) {
			float err = std::abs(dst[i] - src[i]);
			if (err > 0.5f) {
				std::cerr << "FAIL: copy dst[" << i << "]=" << dst[i]
				          << " (expected ~" << src[i] << ", err " << err << ")\n";
				pass = false;
				++nrOfFailedTestCases;
			}
		}
		if (pass) std::cout << "copy construction: PASS\n";
	}

	// test 2: copy construction with dirty cache
	std::cout << "+---------    copy with dirty cache   --------+\n";
	{
		zfparray1f original(N, rate, src);
		// modify element (makes cache dirty)
		original.set(0, 99.0f);

		// copy should flush dirty cache first
		zfparray1f copy(original);

		float val = copy(0);
		float err = std::abs(val - 99.0f);
		if (err > 0.5f) {
			std::cerr << "FAIL: copy with dirty cache, val=" << val
			          << " (expected ~99, err " << err << ")\n";
			++nrOfFailedTestCases;
		} else {
			std::cout << "copy with dirty cache: PASS\n";
		}
	}

	// test 3: move construction
	std::cout << "+---------    move construction   --------+\n";
	{
		zfparray1f original(N, rate, src);
		size_t orig_bytes = original.compressed_bytes();

		zfparray1f moved(std::move(original));

		// moved-to should have the data
		if (moved.size() != N) {
			std::cerr << "FAIL: moved size expected " << N << ", got " << moved.size() << '\n';
			++nrOfFailedTestCases;
		}
		if (moved.compressed_bytes() != orig_bytes) {
			std::cerr << "FAIL: moved compressed_bytes mismatch\n";
			++nrOfFailedTestCases;
		}

		// moved-from should be empty
		if (original.size() != 0) {
			std::cerr << "FAIL: moved-from size expected 0, got " << original.size() << '\n';
			++nrOfFailedTestCases;
		}

		// verify data integrity
		float dst[N];
		moved.decompress(dst);
		bool pass = true;
		for (size_t i = 0; i < N; ++i) {
			float err = std::abs(dst[i] - src[i]);
			if (err > 0.5f) {
				pass = false;
				++nrOfFailedTestCases;
			}
		}
		if (pass) std::cout << "move construction: PASS\n";
	}

	// test 4: copy assignment
	std::cout << "+---------    copy assignment   --------+\n";
	{
		zfparray1f original(N, rate, src);
		zfparray1f target(4, 8.0);  // different size and rate

		target = original;

		if (target.size() != original.size()) {
			std::cerr << "FAIL: copy assignment size mismatch\n";
			++nrOfFailedTestCases;
		}

		float dst[N];
		target.decompress(dst);
		bool pass = true;
		for (size_t i = 0; i < N; ++i) {
			float err = std::abs(dst[i] - src[i]);
			if (err > 0.5f) {
				pass = false;
				++nrOfFailedTestCases;
			}
		}
		if (pass) std::cout << "copy assignment: PASS\n";
	}

	// test 5: move assignment
	std::cout << "+---------    move assignment   --------+\n";
	{
		zfparray1f original(N, rate, src);
		zfparray1f target(4, 8.0);

		target = std::move(original);

		if (target.size() != N) {
			std::cerr << "FAIL: move assignment size expected " << N << ", got " << target.size() << '\n';
			++nrOfFailedTestCases;
		}
		if (original.size() != 0) {
			std::cerr << "FAIL: move assignment source size expected 0, got " << original.size() << '\n';
			++nrOfFailedTestCases;
		}

		float dst[N];
		target.decompress(dst);
		bool pass = true;
		for (size_t i = 0; i < N; ++i) {
			float err = std::abs(dst[i] - src[i]);
			if (err > 0.5f) {
				pass = false;
				++nrOfFailedTestCases;
			}
		}
		if (pass) std::cout << "move assignment: PASS\n";
	}

	// test 6: self-assignment safety
	std::cout << "+---------    self-assignment   --------+\n";
	{
		zfparray1f arr(N, rate, src);

		// suppress self-assignment warnings — this is intentional
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
		arr = arr;
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

		if (arr.size() != N) {
			std::cerr << "FAIL: self-assignment size changed\n";
			++nrOfFailedTestCases;
		}

		float dst[N];
		arr.decompress(dst);
		bool pass = true;
		for (size_t i = 0; i < N; ++i) {
			float err = std::abs(dst[i] - src[i]);
			if (err > 0.5f) {
				pass = false;
				++nrOfFailedTestCases;
			}
		}
		if (pass) std::cout << "self-assignment: PASS\n";
	}

	// test 7: copy assignment with dirty target cache
	std::cout << "+---------    copy assign with dirty target   --------+\n";
	{
		zfparray1f original(N, rate, src);
		zfparray1f target(N, rate, src);

		// make target dirty
		target.set(0, 77.0f);

		// copy assign should flush target's dirty cache before overwriting
		target = original;

		float val = target(0);
		float err = std::abs(val - src[0]);
		if (err > 0.5f) {
			std::cerr << "FAIL: copy assign with dirty target, val=" << val << '\n';
			++nrOfFailedTestCases;
		} else {
			std::cout << "copy assign with dirty target: PASS\n";
		}
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
