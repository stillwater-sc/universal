// roundtrip.cpp: compression quality tests for zfparray (ZFP compressed array container)
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
#include <vector>

// compute RMSE between two arrays
template<typename Real>
double compute_rmse(const Real* a, const Real* b, size_t n) {
	double sum_sq = 0.0;
	for (size_t i = 0; i < n; ++i) {
		double diff = static_cast<double>(a[i]) - static_cast<double>(b[i]);
		sum_sq += diff * diff;
	}
	return std::sqrt(sum_sq / static_cast<double>(n));
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "zfparray roundtrip tests";
	int nrOfFailedTestCases = 0;

	// test 1: RMSE decreases as rate increases (sinusoidal data)
	std::cout << "+---------    RMSE vs rate (sinusoidal)   --------+\n";
	{
		constexpr size_t N = 128;
		float src[N];
		for (size_t i = 0; i < N; ++i) {
			src[i] = static_cast<float>(std::sin(2.0 * 3.14159265358979 * static_cast<double>(i) / N));
		}

		double rates[] = { 4.0, 8.0, 16.0, 32.0 };
		double prev_rmse = 1e30;
		bool monotone = true;

		for (double rate : rates) {
			zfparray1f arr(N, rate, src);
			float dst[N];
			arr.decompress(dst);
			double rmse = compute_rmse(src, dst, N);
			std::cout << "  rate=" << std::setw(2) << rate << " bpv: RMSE=" << std::scientific << rmse
			          << ", ratio=" << std::fixed << std::setprecision(1) << arr.compression_ratio() << "x\n";

			if (rmse >= prev_rmse && rate > 4.0) {
				std::cerr << "FAIL: RMSE did not decrease from rate "
				          << (rate == 8.0 ? 4.0 : (rate == 16.0 ? 8.0 : 16.0))
				          << " to " << rate << '\n';
				monotone = false;
				++nrOfFailedTestCases;
			}
			prev_rmse = rmse;
		}
		if (monotone) std::cout << "RMSE monotonically decreases with rate: PASS\n";
	}

	// test 2: large array (multiple blocks)
	std::cout << "+---------    large array round-trip   --------+\n";
	{
		constexpr size_t N = 1000;
		std::vector<float> src(N);
		for (size_t i = 0; i < N; ++i) {
			src[i] = static_cast<float>(std::sin(static_cast<double>(i) * 0.01)
			                           + 0.5 * std::cos(static_cast<double>(i) * 0.03));
		}

		zfparray1f arr(N, 16.0, src.data());
		std::cout << "  size=" << arr.size() << ", blocks=" << arr.num_blocks()
		          << ", compressed=" << arr.compressed_bytes() << " bytes\n";

		std::vector<float> dst(N);
		arr.decompress(dst.data());
		double rmse = compute_rmse(src.data(), dst.data(), N);
		std::cout << "  RMSE=" << std::scientific << rmse << '\n';

		if (rmse > 1.0) {
			std::cerr << "FAIL: large array RMSE too high: " << rmse << '\n';
			++nrOfFailedTestCases;
		} else {
			std::cout << "large array round-trip: PASS\n";
		}
	}

	// test 3: partial block handling (non-multiple-of-4 sizes)
	std::cout << "+---------    partial block handling   --------+\n";
	{
		// test sizes that are NOT multiples of 4
		size_t sizes[] = { 1, 2, 3, 5, 7, 9, 13, 15 };
		bool all_pass = true;

		for (size_t n : sizes) {
			std::vector<float> src(n);
			for (size_t i = 0; i < n; ++i) {
				src[i] = static_cast<float>(i + 1) * 0.5f;
			}

			zfparray1f arr(n, 16.0, src.data());
			std::vector<float> dst(n);
			arr.decompress(dst.data());

			double rmse = compute_rmse(src.data(), dst.data(), n);
			if (rmse > 1.0) {
				std::cerr << "FAIL: partial block n=" << n << ", RMSE=" << rmse << '\n';
				all_pass = false;
				++nrOfFailedTestCases;
			}
		}
		if (all_pass) std::cout << "partial block handling: PASS\n";
	}

	// test 4: double precision
	std::cout << "+---------    double precision round-trip   --------+\n";
	{
		constexpr size_t N = 64;
		double src[N];
		for (size_t i = 0; i < N; ++i) {
			src[i] = std::sin(2.0 * 3.14159265358979 * static_cast<double>(i) / N);
		}

		zfparray1d arr(N, 16.0, src);
		double dst[N];
		arr.decompress(dst);
		double rmse = compute_rmse(src, dst, N);
		std::cout << "  double RMSE at 16 bpv: " << std::scientific << rmse << '\n';

		if (rmse > 1.0) {
			std::cerr << "FAIL: double precision RMSE too high: " << rmse << '\n';
			++nrOfFailedTestCases;
		} else {
			std::cout << "double precision round-trip: PASS\n";
		}
	}

	// test 5: set_rate recompression
	std::cout << "+---------    set_rate recompression   --------+\n";
	{
		constexpr size_t N = 16;
		float src[N];
		for (size_t i = 0; i < N; ++i) src[i] = static_cast<float>(i);

		zfparray1f arr(N, 8.0, src);
		size_t old_bytes = arr.compressed_bytes();

		arr.set_rate(16.0);
		size_t new_bytes = arr.compressed_bytes();

		std::cout << "  8 bpv: " << old_bytes << " bytes, 16 bpv: " << new_bytes << " bytes\n";
		if (new_bytes <= old_bytes) {
			std::cerr << "FAIL: set_rate(16) should use more bytes than rate=8\n";
			++nrOfFailedTestCases;
		}

		// verify data is still approximately correct after rate change
		float dst[N];
		arr.decompress(dst);
		double rmse = compute_rmse(src, dst, N);
		std::cout << "  RMSE after set_rate: " << std::scientific << rmse << '\n';
		if (rmse > 2.0) {
			std::cerr << "FAIL: data corrupted after set_rate, RMSE=" << rmse << '\n';
			++nrOfFailedTestCases;
		} else {
			std::cout << "set_rate recompression: PASS\n";
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
