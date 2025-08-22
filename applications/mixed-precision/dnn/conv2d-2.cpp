// conv2d-1.cpp: mixed-precision convolution benchmark
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Energy-Efficient 2D Convolution Kernel using Universal Numbers Library
// 
// This implementation avoids im2col data duplication by using direct convolution
// with mixed-precision support through the Universal Numbers Library.

#include <iostream>
#include <chrono>
#include <random>
#include <iomanip>
#include <string>
#include <memory>
#include <cassert>

#define POSIT_FAST_SPECIALIZATION 1
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include "conv2d-2.hpp"

int main()
try {
	using namespace sw::universal;

    // Example usage with different precision configurations
	using Conv2D_float = Conv2D<float, float, float, float>;
    using Conv2D_fixpnt = Conv2D<fixpnt<8, 4>, fixpnt<8, 4>, fixpnt<8, 4>, fixpnt<8, 4>>;
    using Conv2D_posit = Conv2D<posit<16, 1>, posit<8, 0>, posit<32, 2>, posit<16, 1>>;
    using Conv2D_cfloat = Conv2D<cfloat<8, 2>, cfloat<8, 2>, cfloat<16, 5>, cfloat<8, 2>>;

    BenchmarkRunner benchmark;

    std::cout << "Starting Conv2D Energy Efficiency Benchmark...\n\n";

    ConvolutionParameters2D conv2dParams({
        .batch_size = 1, .in_channels = 64, .out_channels = 128,
        .in_height = 28, .in_width = 28,
        .kernel_height = 3, .kernel_width = 3,
        .stride_h = 1, .stride_w = 1, .pad_h = 1, .pad_w = 1,
        .dilation_h = 1, .dilation_w = 1
        });

    Conv2D_float conv(conv2dParams);
    benchmark.conv2D<Conv2D_float>(conv2dParams);

#if 0
    // Run comprehensive benchmark with different Conv2D types
    benchmark.run_comprehensive_benchmark<
        Conv2D_fixpnt,
        Conv2D_posit,
        Conv2D_cfloat,
        Conv2D<float, float, float, float>
    >();
#endif
    // Run tile size analysis
    benchmark.run_tile_size_analysis();

    std::cout << "\nBenchmark completed successfully!\n";

}
catch (const std::exception& e) {
    std::cerr << "Benchmark failed with exception: " << e.what() << "\n";
    return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Unexpected exception caught\n" << std::endl;
	return EXIT_FAILURE;
}
