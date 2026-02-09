// api.cpp: application programming interface tests for zfparray (ZFP compressed array container)
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

	std::string test_suite = "zfparray API tests";
	int nrOfFailedTestCases = 0;

	// verify all 6 type aliases compile
	std::cout << "+---------    zfparray type aliases   --------+\n";
	{
		zfparray1f a; (void)a;
		zfparray1d b; (void)b;
		zfparray2f c; (void)c;
		zfparray2d d; (void)d;
		zfparray3f e; (void)e;
		zfparray3d f; (void)f;
		std::cout << "All 6 zfparray type aliases compile: PASS\n";
	}

	// construct with size and rate
	std::cout << "+---------    construction   --------+\n";
	{
		constexpr double rate = 8.0;
		zfparray1f arr(20, rate);
		std::cout << "size          : " << arr.size() << '\n';
		std::cout << "num_blocks    : " << arr.num_blocks() << '\n';
		std::cout << "rate          : " << arr.rate() << " bpv\n";
		std::cout << "bytes_per_block: " << arr.bytes_per_block() << '\n';
		std::cout << "compressed_bytes: " << arr.compressed_bytes() << '\n';

		if (arr.size() != 20) {
			std::cerr << "FAIL: size() expected 20, got " << arr.size() << '\n';
			++nrOfFailedTestCases;
		}
		if (arr.num_blocks() != 5) {
			std::cerr << "FAIL: num_blocks() expected 5, got " << arr.num_blocks() << '\n';
			++nrOfFailedTestCases;
		}
		if (arr.rate() != rate) {
			std::cerr << "FAIL: rate() expected " << rate << ", got " << arr.rate() << '\n';
			++nrOfFailedTestCases;
		}
		// bytes_per_block = ceil(8.0 * 4 / 8) = 4
		if (arr.bytes_per_block() != 4) {
			std::cerr << "FAIL: bytes_per_block() expected 4, got " << arr.bytes_per_block() << '\n';
			++nrOfFailedTestCases;
		}
		// compressed_bytes = 5 blocks * 4 bytes = 20
		if (arr.compressed_bytes() != 20) {
			std::cerr << "FAIL: compressed_bytes() expected 20, got " << arr.compressed_bytes() << '\n';
			++nrOfFailedTestCases;
		}
	}

	// set/get element access
	std::cout << "+---------    set/get element access   --------+\n";
	{
		zfparray1f arr(8, 16.0);  // 16 bpv for higher fidelity
		for (size_t i = 0; i < 8; ++i) {
			arr.set(i, static_cast<float>(i + 1));
		}
		// flush to ensure all blocks are written back
		arr.flush();

		bool pass = true;
		for (size_t i = 0; i < 8; ++i) {
			float val = arr(i);
			float expected = static_cast<float>(i + 1);
			float err = std::abs(val - expected);
			if (err > 0.5f) {  // generous tolerance for compressed storage
				std::cerr << "FAIL: arr(" << i << ") = " << val
				          << ", expected ~" << expected << ", err = " << err << '\n';
				pass = false;
				++nrOfFailedTestCases;
			}
		}
		if (pass) std::cout << "set/get round-trip: PASS\n";
	}

	// construct from raw data and bulk decompress
	std::cout << "+---------    bulk compress/decompress   --------+\n";
	{
		constexpr size_t N = 16;
		float src[N];
		for (size_t i = 0; i < N; ++i) src[i] = static_cast<float>(i) * 0.25f;

		zfparray1f arr(N, 16.0, src);

		float dst[N];
		arr.decompress(dst);

		double max_err = 0.0;
		for (size_t i = 0; i < N; ++i) {
			double err = std::abs(static_cast<double>(dst[i]) - static_cast<double>(src[i]));
			if (err > max_err) max_err = err;
		}
		std::cout << "Bulk round-trip max error: " << max_err << '\n';
		if (max_err > 1.0) {
			std::cerr << "FAIL: max error too large: " << max_err << '\n';
			++nrOfFailedTestCases;
		}
	}

	// compression ratio
	std::cout << "+---------    compression ratio   --------+\n";
	{
		zfparray1f arr(100, 8.0);  // 8 bpv for float (32 bits native) → ~4x
		double ratio = arr.compression_ratio();
		std::cout << "100 floats at 8 bpv: ratio = " << ratio << "x\n";
		if (ratio < 3.0 || ratio > 5.0) {
			std::cerr << "FAIL: compression ratio expected ~4x, got " << ratio << '\n';
			++nrOfFailedTestCases;
		}
	}

	// resize
	std::cout << "+---------    resize   --------+\n";
	{
		zfparray1f arr(10, 8.0);
		arr.resize(20);
		if (arr.size() != 20) {
			std::cerr << "FAIL: after resize, size() expected 20, got " << arr.size() << '\n';
			++nrOfFailedTestCases;
		}
		if (arr.num_blocks() != 5) {
			std::cerr << "FAIL: after resize, num_blocks() expected 5, got " << arr.num_blocks() << '\n';
			++nrOfFailedTestCases;
		}
		std::cout << "resize: PASS\n";
	}

	// raw data access
	std::cout << "+---------    raw data access   --------+\n";
	{
		zfparray1f arr(8, 8.0);
		const uint8_t* p = arr.data();
		size_t sz = arr.data_size();
		std::cout << "data() = " << (p ? "non-null" : "null")
		          << ", data_size() = " << sz << '\n';
		if (p == nullptr || sz == 0) {
			std::cerr << "FAIL: raw data access failed\n";
			++nrOfFailedTestCases;
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
