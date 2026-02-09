// throughput.cpp: measure quantize+dequantize throughput for mxblock, nvblock, and zfparray
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <universal/number/mxfloat/mxfloat.hpp>
#include <universal/number/nvblock/nvblock.hpp>
#include <universal/number/zfpblock/zfparray.hpp>

constexpr size_t NR_OPS = 100000;

// ---------------------------------------------------------------------------
// timing harness
// ---------------------------------------------------------------------------

static void report(const std::string& label, size_t ops, double elapsed) {
	double ops_per_sec = static_cast<double>(ops) / elapsed;
	const char* scales[] = { " ", "K", "M", "G", "T" };
	double v = ops_per_sec;
	int si = 0;
	while (v >= 1000.0 && si < 4) { v /= 1000.0; si++; }
	std::cout << std::left  << std::setw(20) << label
	          << std::right << std::setw(10) << ops
	          << std::setw(14) << std::fixed << std::setprecision(6) << elapsed
	          << std::setw(10) << std::setprecision(0) << v << ' ' << scales[si] << "ops/sec"
	          << '\n';
}

// ---------------------------------------------------------------------------
// mxblock throughput
// ---------------------------------------------------------------------------

template<typename MXBlockType, size_t BlockSize>
static void bench_mxblock(const std::string& label) {
	// prepare one block of source data
	float src[BlockSize];
	float dst[BlockSize];
	for (size_t i = 0; i < BlockSize; ++i) {
		src[i] = static_cast<float>(std::sin(6.283185 * static_cast<double>(i) / static_cast<double>(BlockSize)));
	}

	MXBlockType blk;
	auto t0 = std::chrono::steady_clock::now();
	for (size_t i = 0; i < NR_OPS; ++i) {
		blk.quantize(src);
		blk.dequantize(dst);
	}
	auto t1 = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(t1 - t0).count();

	// prevent optimization
	if (dst[0] == -999.0f) std::cout << "dummy\n";

	report(label, NR_OPS, elapsed);
}

// ---------------------------------------------------------------------------
// nvblock throughput
// ---------------------------------------------------------------------------

template<typename NVBlockType, size_t BlockSize>
static void bench_nvblock(const std::string& label) {
	float src[BlockSize];
	float dst[BlockSize];
	for (size_t i = 0; i < BlockSize; ++i) {
		src[i] = static_cast<float>(std::sin(6.283185 * static_cast<double>(i) / static_cast<double>(BlockSize)));
	}

	NVBlockType blk;
	auto t0 = std::chrono::steady_clock::now();
	for (size_t i = 0; i < NR_OPS; ++i) {
		blk.quantize(src, 1.0f);
		blk.dequantize(dst, 1.0f);
	}
	auto t1 = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(t1 - t0).count();

	if (dst[0] == -999.0f) std::cout << "dummy\n";

	report(label, NR_OPS, elapsed);
}

// ---------------------------------------------------------------------------
// zfparray throughput
// ---------------------------------------------------------------------------

static void bench_zfp(const std::string& label, double rate) {
	using namespace sw::universal;
	constexpr size_t N = 4;  // zfp 1D block size

	float src[N];
	float dst[N];
	for (size_t i = 0; i < N; ++i) {
		src[i] = static_cast<float>(std::sin(6.283185 * static_cast<double>(i) / static_cast<double>(N)));
	}

	auto t0 = std::chrono::steady_clock::now();
	for (size_t i = 0; i < NR_OPS; ++i) {
		zfparray1f arr(N, rate, src);
		arr.decompress(dst);
	}
	auto t1 = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(t1 - t0).count();

	if (dst[0] == -999.0f) std::cout << "dummy\n";

	report(label, NR_OPS, elapsed);
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::cout << "Block quantize+dequantize throughput:\n";
	std::cout << std::left  << std::setw(20) << "Format"
	          << std::right << std::setw(10) << "Ops"
	          << std::setw(14) << "Time(s)"
	          << std::setw(16) << "Throughput"
	          << '\n';
	std::cout << std::string(60, '-') << '\n';

	bench_mxblock<mxfp4, 32>("mxfp4  (e2m1,32)");
	bench_mxblock<mxfp8, 32>("mxfp8  (e4m3,32)");
	bench_nvblock<nvfp4, 16>("nvfp4  (e2m1,16)");
	bench_zfp("zfp1f  rate=4",   4.0);
	bench_zfp("zfp1f  rate=8",   8.0);
	bench_zfp("zfp1f  rate=16", 16.0);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
