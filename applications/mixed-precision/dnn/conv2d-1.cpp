// conv2d-1.cpp: mixed-precision convolution benchmark
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <chrono>

#include <universal/native/ieee754.hpp>
#define POSIT_FAST_SPECIALIZATION 1
#include <universal/number/posit/posit.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include "conv2d-1.hpp"

int main() {
    using namespace sw::universal;
    
    // Convenience type aliases for common Universal number configurations
    using Posit80 = posit<8, 0>;
    using Posit82 = posit<8, 2>;
    using CFloat16 = cfloat<16, 5>;
    using CFloat32 = cfloat<32, 8>;
    using FixedPoint16 = fixpnt<16, 8>;

	// MixedPrecisionConfig<InputT, WeightT, AccumT, OutputT> is a template struct that defines the types used in convolution

    // Example mixed-precision configurations
    using HighPrecisionConfig = MixedPrecisionConfig<float, float, double, float>;
    using BfloatMixedConfig = MixedPrecisionConfig<bfloat16, bfloat16, float, bfloat16>;
    using PositMixedConfig = MixedPrecisionConfig<Posit82, Posit82, float, Posit82>;
    using CFloatMixedConfig = MixedPrecisionConfig<CFloat16, CFloat16, CFloat32, CFloat16>;
    using HybridConfig = MixedPrecisionConfig<half, half, float, half>;

    // Create different precision configurations for comparison
    //constexpr size_t N = 1, C_in = 128, C_out = 64, H_in = 32, W_in = 32, K_h = 3, K_w = 3;
    //constexpr size_t N = 1, C_in = 64, C_out = 128, H_in = 32, W_in = 32, K_h = 3, K_w = 3;
    //constexpr size_t N = 1, C_in = 32, C_out = 16, H_in = 8, W_in = 8, K_h = 3, K_w = 3;
    //constexpr size_t N = 8, C_in = 64, C_out = 128, H_in = 32, W_in = 32, K_h = 3, K_w = 3;
    constexpr size_t N = 8, C_in = 32, C_out = 16, H_in = 8, W_in = 8, K_h = 3, K_w = 3;

    // Benchmark different configurations
    auto fp32_result = ConvolutionBenchmark<HighPrecisionConfig>::benchmark_direct_conv(
        N, C_in, H_in, W_in, C_out, K_h, K_w, 100);
    std::cout << "FP32 Config - GFLOPS: " << fp32_result.gflops
        << ", Memory: " << fp32_result.memory_footprint_bytes << " bytes\n";
    
    auto bfloat_result = ConvolutionBenchmark<BfloatMixedConfig>::benchmark_direct_conv(
        N, C_in, H_in, W_in, C_out, K_h, K_w, 100);
    std::cout << "Bfloat16 Config - GFLOPS: " << bfloat_result.gflops 
              << ", Memory: " << bfloat_result.memory_footprint_bytes << " bytes\n";
    
    auto posit_result = ConvolutionBenchmark<PositMixedConfig>::benchmark_direct_conv(
        N, C_in, H_in, W_in, C_out, K_h, K_w, 100);
    std::cout << "Posit8 Config - GFLOPS: " << posit_result.gflops
        << ", Memory: " << posit_result.memory_footprint_bytes << " bytes\n";

    return 0;
}

/*
 ETLO: 8/18/2025
 Ryzen 9: single thread

 N = 1, C_in = 128, C_out = 64, H_in = 32, W_in = 32, K_h = 3, K_w = 3
 FP32     Config - GFLOPS: 0.615096, Memory: 1049600 bytes
 Bfloat16 Config - GFLOPS: 0.216806, Memory:  524800 bytes
 Posit8   Config - GFLOPS: 0.117461, Memory:  262400 bytes

 N = 1, C_in = 64, C_out = 128, H_in = 32, W_in = 32, K_h = 3, K_w = 3
 FP32     Config - GFLOPS: 0.613619, Memory: 1017856 bytes
 Bfloat16 Config - GFLOPS: 0.231718, Memory:  508928 bytes
 Posit8   Config - GFLOPS: 0.151432, Memory:  254464 bytes

 N = 1, C_in = 32, C_out = 16, H_in = 8, W_in = 8, K_h = 3, K_w = 3
 FP32     Config - GFLOPS: 0.612652, Memory:   91136 bytes
 Bfloat16 Config - GFLOPS: 0.229921, Memory:   45568 bytes
 Posit8   Config - GFLOPS: 0.151830, Memory:   22784 bytes

 N = 8, C_in = 64, C_out = 128, H_in = 32, W_in = 32, K_h = 3, K_w = 3
 FP32     Config - GFLOPS: 2.206150, Memory: 6078464 bytes
 Bfloat16 Config - GFLOPS: 1.647780, Memory: 3039232 bytes
 Posit8   Config - GFLOPS: 0.850738, Memory: 1519616 bytes

 N = 8, C_in = 32, C_out = 16, H_in = 8, W_in = 8, K_h = 3, K_w = 3
 FP32     Config - GFLOPS: 2.047450, Memory:  102400 bytes
 Bfloat16 Config - GFLOPS: 1.329820, Memory:   51200 bytes
 Posit8   Config - GFLOPS: 0.728240, Memory:   25600 bytes

 */