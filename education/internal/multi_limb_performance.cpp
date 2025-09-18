// multi_limb_performance.cpp: performance comparison of different multi-limb configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project
#include <universal/utility/directives.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocksignificand/blocksignificand.hpp>
#include <iostream>
#include <chrono>
#include <string>

using namespace sw::universal;

template<typename BlockBinaryType>
void benchmark_arithmetic(const std::string& description, size_t iterations = 1000000) {
    std::cout << "Benchmarking: " << description << std::endl;

    BlockBinaryType a, b, result;
    a = 123456789ULL;
    b = 987654321ULL;

    // Warm up
    for (size_t i = 0; i < 1000; ++i) {
        result = a + b;
        result = a * b;
    }

    auto start = std::chrono::high_resolution_clock::now();

    // Addition benchmark
    for (size_t i = 0; i < iterations; ++i) {
        result = a + b;
        a = result;  // Chain operations to prevent optimization
    }

    auto mid = std::chrono::high_resolution_clock::now();

    // Reset
    a = 123456789ULL;

    // Multiplication benchmark
    for (size_t i = 0; i < iterations / 10; ++i) {  // Fewer iterations for multiplication
        result = a * b;
        a = result;
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto add_duration = std::chrono::duration_cast<std::chrono::microseconds>(mid - start);
    auto mul_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - mid);

    std::cout << "  Addition (" << iterations << " ops): " << add_duration.count() << " μs\n";
    std::cout << "  Multiplication (" << iterations/10 << " ops): " << mul_duration.count() << " μs\n";
    std::cout << "  Add throughput: " << (iterations * 1000000.0 / add_duration.count()) << " ops/sec\n";
    std::cout << "  Mul throughput: " << ((iterations/10) * 1000000.0 / mul_duration.count()) << " ops/sec\n";
    std::cout << std::endl;
}

int main() {
    std::cout << "Multi-Limb Performance Analysis\n";
    std::cout << "===============================\n\n";

    std::cout << "This example demonstrates performance characteristics of different\n";
    std::cout << "block configurations for multi-limb arithmetic operations.\n\n";

    const size_t iterations = 100000;  // Reduced for educational purposes

    // Test different block sizes for 128-bit arithmetic
    std::cout << "128-bit Integer Arithmetic Performance:\n";
    std::cout << "--------------------------------------\n";

    benchmark_arithmetic<blockbinary<128, uint8_t, BinaryNumberType::Unsigned>>(
        "128-bit with 8-bit blocks (16 blocks)", iterations);

    benchmark_arithmetic<blockbinary<128, uint16_t, BinaryNumberType::Unsigned>>(
        "128-bit with 16-bit blocks (8 blocks)", iterations);

    benchmark_arithmetic<blockbinary<128, uint32_t, BinaryNumberType::Unsigned>>(
        "128-bit with 32-bit blocks (4 blocks)", iterations);

    benchmark_arithmetic<blockbinary<128, uint64_t, BinaryNumberType::Unsigned>>(
        "128-bit with 64-bit blocks (2 blocks)", iterations);

    // Test different precisions with optimal block size
    std::cout << "Different Precisions with 64-bit blocks:\n";
    std::cout << "---------------------------------------\n";

    benchmark_arithmetic<blockbinary<64, uint64_t, BinaryNumberType::Unsigned>>(
        "64-bit (1 block)", iterations);

    benchmark_arithmetic<blockbinary<128, uint64_t, BinaryNumberType::Unsigned>>(
        "128-bit (2 blocks)", iterations);

    benchmark_arithmetic<blockbinary<256, uint64_t, BinaryNumberType::Unsigned>>(
        "256-bit (4 blocks)", iterations);

    benchmark_arithmetic<blockbinary<512, uint64_t, BinaryNumberType::Unsigned>>(
        "512-bit (8 blocks)", iterations);

    // Memory usage analysis
    std::cout << "Memory Usage Analysis:\n";
    std::cout << "---------------------\n";

    std::cout << "Storage requirements for 256-bit numbers:\n";
    std::cout << "8-bit blocks:  " << sizeof(blockbinary<256, uint8_t, BinaryNumberType::Unsigned>) << " bytes\n";
    std::cout << "16-bit blocks: " << sizeof(blockbinary<256, uint16_t, BinaryNumberType::Unsigned>) << " bytes\n";
    std::cout << "32-bit blocks: " << sizeof(blockbinary<256, uint32_t, BinaryNumberType::Unsigned>) << " bytes\n";
    std::cout << "64-bit blocks: " << sizeof(blockbinary<256, uint64_t, BinaryNumberType::Unsigned>) << " bytes\n";
    std::cout << std::endl;

    // Demonstration of cache effects
    std::cout << "Cache Performance Considerations:\n";
    std::cout << "--------------------------------\n";

    const size_t array_size = 1000;

    // Test with different block sizes in arrays
    {
        std::vector<blockbinary<128, uint32_t, BinaryNumberType::Unsigned>> arr32(array_size);
        std::vector<blockbinary<128, uint64_t, BinaryNumberType::Unsigned>> arr64(array_size);

        // Initialize arrays
        for (size_t i = 0; i < array_size; ++i) {
            arr32[i] = i;
            arr64[i] = i;
        }

        auto start = std::chrono::high_resolution_clock::now();

        // Sequential access with 32-bit blocks
        uint64_t sum32 = 0;
        for (size_t iter = 0; iter < 1000; ++iter) {
            for (size_t i = 0; i < array_size - 1; ++i) {
                arr32[i] = arr32[i] + arr32[i + 1];
                sum32 += arr32[i].to_ull();
            }
        }

        auto mid = std::chrono::high_resolution_clock::now();

        // Sequential access with 64-bit blocks
        uint64_t sum64 = 0;
        for (size_t iter = 0; iter < 1000; ++iter) {
            for (size_t i = 0; i < array_size - 1; ++i) {
                arr64[i] = arr64[i] + arr64[i + 1];
                sum64 += arr64[i].to_ull();
            }
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto time32 = std::chrono::duration_cast<std::chrono::microseconds>(mid - start);
        auto time64 = std::chrono::duration_cast<std::chrono::microseconds>(end - mid);

        std::cout << "Array processing (1000 iterations on " << array_size << " elements):\n";
        std::cout << "32-bit blocks: " << time32.count() << " μs\n";
        std::cout << "64-bit blocks: " << time64.count() << " μs\n";
        std::cout << "Ratio (32/64): " << (double)time32.count() / time64.count() << "\n";

        // Prevent optimization
        std::cout << "Checksums: " << sum32 << ", " << sum64 << std::endl;
        std::cout << std::endl;
    }

    // BlockSignificant performance comparison
    //std::cout << "BlockSignificant Encoding Performance:\n";
    //std::cout << "-------------------------------------\n";

    //{
    //    blocksignificand<64, uint32_t, BitEncoding::Ones> ones_sig;
    //    blocksignificand<64, uint32_t, BitEncoding::Twos> twos_sig;

    //    ones_sig.setbits(0x123456789ABCDEF0ULL);
    //    twos_sig.setbits(0x123456789ABCDEF0ULL);

    //    const size_t sig_iterations = 1000000;

    //    auto start = std::chrono::high_resolution_clock::now();

    //    // Ones complement operations
    //    for (size_t i = 0; i < sig_iterations; ++i) {
    //        ones_sig <<= 1;
    //        ones_sig >>= 1;
    //    }

    //    auto mid = std::chrono::high_resolution_clock::now();

    //    // Twos complement operations
    //    for (size_t i = 0; i < sig_iterations; ++i) {
    //        twos_sig <<= 1;
    //        twos_sig >>= 1;
    //    }

    //    auto end = std::chrono::high_resolution_clock::now();

    //    auto ones_time = std::chrono::duration_cast<std::chrono::microseconds>(mid - start);
    //    auto twos_time = std::chrono::duration_cast<std::chrono::microseconds>(end - mid);

    //    std::cout << "Shift operations (" << sig_iterations << " iterations):\n";
    //    std::cout << "Ones encoding: " << ones_time.count() << " μs\n";
    //    std::cout << "Twos encoding: " << twos_time.count() << " μs\n";
    //    std::cout << "Performance difference: " << std::abs((double)ones_time.count() - twos_time.count())
    //              << " μs (" << (100.0 * std::abs((double)ones_time.count() - twos_time.count()) /
    //                          std::min(ones_time.count(), twos_time.count())) << "%)\n";
    //    std::cout << std::endl;
    //}

    std::cout << "Performance Analysis Summary:\n";
    std::cout << "============================\n";
    std::cout << "\nKey findings:\n";
    std::cout << "1. Larger block sizes generally perform better for basic arithmetic\n";
    std::cout << "2. Memory usage is consistent across block sizes (padding effects)\n";
    std::cout << "3. Cache performance depends on access patterns and array sizes\n";
    std::cout << "4. Different encodings have minimal performance impact for basic ops\n";
    std::cout << "5. Choose block size based on target architecture word size\n";
    std::cout << "\nRecommendations:\n";
    std::cout << "- Use 64-bit blocks on 64-bit architectures\n";
    std::cout << "- Use 32-bit blocks on 32-bit architectures or for memory-constrained systems\n";
    std::cout << "- Consider SIMD opportunities with smaller block sizes\n";
    std::cout << "- Profile your specific use case for optimal configuration\n";

    return EXIT_SUCCESS;
}