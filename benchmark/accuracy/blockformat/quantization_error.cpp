// quantization_error.cpp: compare quantization RMSE, SNR, and QSNR across mxblock, nvblock, and zfparray
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <universal/number/mxfloat/mxfloat.hpp>
#include <universal/number/nvblock/nvblock.hpp>
#include <universal/number/zfpblock/zfparray.hpp>
#include <universal/quantization/error_metrics.hpp>

// ---------------------------------------------------------------------------
// data generators
// ---------------------------------------------------------------------------

static std::vector<float> generate_sinusoidal(size_t n) {
	constexpr double TWO_PI = 6.283185307179586;
	std::vector<float> v(n);
	for (size_t i = 0; i < n; ++i) {
		v[i] = static_cast<float>(std::sin(TWO_PI * static_cast<double>(i) / static_cast<double>(n)));
	}
	return v;
}

static std::vector<float> generate_ramp(size_t n) {
	std::vector<float> v(n);
	for (size_t i = 0; i < n; ++i) {
		v[i] = static_cast<float>(static_cast<double>(i) / static_cast<double>(n));
	}
	return v;
}

// ---------------------------------------------------------------------------
// formatted output for one benchmark row
// ---------------------------------------------------------------------------

static void print_row(const std::string& label, double rate_bpv, double ratio,
                      const std::vector<float>& src, const std::vector<float>& dst) {
	using sw::universal::rmse;
	using sw::universal::snr;
	using sw::universal::qsnr;
	std::cout << std::left  << std::setw(20) << label
	          << std::right << std::setw(8)  << std::fixed << std::setprecision(2) << rate_bpv
	          << std::setw(7)  << std::setprecision(1) << ratio << 'x'
	          << std::setw(14) << std::scientific << std::setprecision(4) << rmse(src, dst)
	          << std::setw(12) << std::fixed << std::setprecision(2) << snr(src, dst)
	          << std::setw(12) << std::fixed << std::setprecision(2) << qsnr(src, dst)
	          << '\n';
}

// ---------------------------------------------------------------------------
// benchmark entry: quantize + dequantize through mxblock
// ---------------------------------------------------------------------------

template<typename MXBlockType, size_t BlockSize>
static void benchmark_mxblock(const std::string& label, double rate_bpv, double ratio,
                               const std::vector<float>& src, std::vector<float>& dst) {
	size_t n = src.size();
	for (size_t offset = 0; offset < n; offset += BlockSize) {
		MXBlockType blk;
		size_t count = std::min(BlockSize, n - offset);
		blk.quantize(src.data() + offset, count);
		blk.dequantize(dst.data() + offset, count);
	}
	print_row(label, rate_bpv, ratio, src, dst);
}

// ---------------------------------------------------------------------------
// benchmark entry: quantize + dequantize through nvblock
// ---------------------------------------------------------------------------

template<typename NVBlockType, size_t BlockSize>
static void benchmark_nvblock(const std::string& label, double rate_bpv, double ratio,
                               const std::vector<float>& src, std::vector<float>& dst) {
	size_t n = src.size();
	for (size_t offset = 0; offset < n; offset += BlockSize) {
		NVBlockType blk;
		size_t count = std::min(BlockSize, n - offset);
		blk.quantize(src.data() + offset, 1.0f, count);
		blk.dequantize(dst.data() + offset, 1.0f, count);
	}
	print_row(label, rate_bpv, ratio, src, dst);
}

// ---------------------------------------------------------------------------
// benchmark entry: compress + decompress through zfparray
// ---------------------------------------------------------------------------

static void benchmark_zfp(const std::string& label, double rate_bpv,
                           const std::vector<float>& src, std::vector<float>& dst) {
	using namespace sw::universal;
	size_t n = src.size();
	zfparray1f arr(n, rate_bpv, src.data());
	arr.decompress(dst.data());

	double ratio = arr.compression_ratio();
	print_row(label, rate_bpv, ratio, src, dst);
}

// ---------------------------------------------------------------------------
// print header
// ---------------------------------------------------------------------------

static void print_header(const std::string& pattern, size_t n) {
	std::cout << '\n' << pattern << " (N=" << n << "):\n";
	std::cout << std::left  << std::setw(20) << "Format"
	          << std::right << std::setw(8)  << "Rate"
	          << std::setw(9) << "Ratio"
	          << std::setw(14) << "RMSE"
	          << std::setw(12) << "SNR(dB)"
	          << std::setw(12) << "QSNR(dB)"
	          << '\n';
	std::cout << std::string(75, '-') << '\n';
}

// ---------------------------------------------------------------------------
// run all benchmarks on one data pattern
// ---------------------------------------------------------------------------

static void run_pattern(const std::string& pattern, const std::vector<float>& src) {
	using namespace sw::universal;

	size_t n = src.size();
	std::vector<float> dst(n);

	print_header(pattern, n);

	// mxfp4: e2m1, BS=32 -> 1B scale + 32x0.5B = 17B for 128B -> 7.5x, 4.25 bpv
	benchmark_mxblock<mxfp4, 32>("mxfp4  (e2m1,32)", 4.25, 7.5, src, dst);

	// mxfp8: e4m3, BS=32 -> 1B scale + 32x1B = 33B for 128B -> 3.9x, 8.25 bpv
	benchmark_mxblock<mxfp8, 32>("mxfp8  (e4m3,32)", 8.25, 3.9, src, dst);

	// nvfp4: e2m1, BS=16 -> 1B scale + 16x0.5B = 9B for 64B -> 7.1x, 4.5 bpv
	benchmark_nvblock<nvfp4, 16>("nvfp4  (e2m1,16)", 4.50, 7.1, src, dst);

	// zfp at various rates
	benchmark_zfp("zfp1f  rate=4",   4.0,  src, dst);
	benchmark_zfp("zfp1f  rate=8",   8.0,  src, dst);
	benchmark_zfp("zfp1f  rate=16", 16.0,  src, dst);
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	constexpr size_t N = 1024;

	std::cout << "Block format quantization error benchmark\n";
	std::cout << "Comparing mxblock (OCP MX), nvblock (NVIDIA NVFP4), zfparray (ZFP)\n";

	run_pattern("Sinusoidal data", generate_sinusoidal(N));
	run_pattern("Linear ramp data", generate_ramp(N));

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
