// blocksignificant_operations.cpp: educational example for blocksignificant usage
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
// 
// This file is part of the universal numbers project
#include <universal/utility/directives.hpp>
#include <universal/internal/blocksignificand/blocksignificand.hpp>
#include <iostream>
#include <string>

using namespace sw::universal;

int main() {
    std::cout << "BlockSignificant Operations: Floating-Point Significand Management\n";
    std::cout << "==================================================================\n\n";

    // Example 1: Different bit encodings for different operations
    {
        std::cout << "Example 1: Bit Encoding Types\n";
        std::cout << "-----------------------------\n";

        // Same value with different encodings
        blocksignificand<64, uint32_t, BitEncoding::Ones> ones_encoded;
        blocksignificand<64, uint32_t, BitEncoding::Twos> twos_encoded;
        blocksignificand<64, uint32_t, BitEncoding::Flex> flex_encoded;

        // Set to same fractional value (0.75 = 0.11 binary)
        uint64_t frac_bits = 0xC000000000000000ULL;  // 1.1 in the upper bits

        ones_encoded.setbits(frac_bits);
        twos_encoded.setbits(frac_bits);
        flex_encoded.setbits(frac_bits);

        std::cout << "Same bit pattern with different encodings:\n";
        std::cout << "Ones encoding: " << to_hex(ones_encoded) << std::endl;
        std::cout << "Twos encoding: " << to_hex(twos_encoded) << std::endl;
        std::cout << "Flex encoding: " << to_hex(flex_encoded) << std::endl;
        std::cout << std::endl;
    }

    // Example 2: Optimal encodings for different operations
    {
        std::cout << "Example 2: Operation-Specific Optimizations\n";
        std::cout << "-------------------------------------------\n";

        // For addition/subtraction: 2's complement is optimal
        blocksignificand<32, uint32_t, BitEncoding::Twos> add_operand1, add_operand2;

        // For multiplication: 1's complement is optimal
        blocksignificand<32, uint32_t, BitEncoding::Ones> mul_operand1, mul_operand2;

        // Set up some sample significands
        add_operand1.setbits(0x80000000U);  // 1.0
        add_operand2.setbits(0x40000000U);  // 0.5

        mul_operand1.setbits(0x80000000U);  // 1.0
        mul_operand2.setbits(0x60000000U);  // 0.75

        std::cout << "Addition operands (2's complement optimal):\n";
        std::cout << "Operand 1: " << to_hex(add_operand1) << " (represents ~1.0)\n";
        std::cout << "Operand 2: " << to_hex(add_operand2) << " (represents ~0.5)\n";

        std::cout << "\nMultiplication operands (1's complement optimal):\n";
        std::cout << "Operand 1: " << to_hex(mul_operand1) << " (represents ~1.0)\n";
        std::cout << "Operand 2: " << to_hex(mul_operand2) << " (represents ~0.75)\n";
        std::cout << std::endl;
    }

    // Example 3: Radix point management
    {
        std::cout << "Example 3: Radix Point Management\n";
        std::cout << "---------------------------------\n";

        blocksignificand<32, uint32_t, BitEncoding::Twos> significand;

        // Demonstrate how the radix point affects interpretation
        significand.setbits(0xC0000000U);  // Binary: 11000000...

        std::cout << "Bit pattern: " << to_binary(significand, true) << std::endl;
        std::cout << "This represents different values depending on radix point:\n";
        std::cout << "  If radix at bit 31: 1.1 (decimal 1.5)\n";
        std::cout << "  If radix at bit 30: 11.0 (decimal 3.0)\n";
        std::cout << "  If radix at bit 29: 110.0 (decimal 6.0)\n";

        // The blocksignificant itself doesn't store radix point location
        // That's managed by the containing floating-point type
        std::cout << "\nNote: radix point location is managed by containing types\n";
        std::cout << "(posit, cfloat, etc.) not by blocksignificant itself\n";
        std::cout << std::endl;
    }

    // Example 4: Normalization and shifting
    {
        std::cout << "Example 4: Normalization and Shifting\n";
        std::cout << "-------------------------------------\n";

        blocksignificand<32, uint32_t, BitEncoding::Twos> unnormalized, normalized;

        // Start with an unnormalized significand (leading zeros)
        unnormalized.setbits(0x00800000U);  // 0.000000001...

        std::cout << "Unnormalized: " << to_binary(unnormalized, true) << std::endl;

        // Normalize by shifting left to get leading 1
        normalized = unnormalized;
        int shifts = 0;
        while (!normalized.test(31) && shifts < 32) {
            normalized <<= 1;
            ++shifts;
        }

        std::cout << "Normalized:   " << to_binary(normalized, true) << std::endl;
        std::cout << "Shifts needed: " << shifts << std::endl;

        // This shift count would be used to adjust the exponent
        // in the containing floating-point representation
        std::cout << "Exponent adjustment needed: " << shifts << std::endl;
        std::cout << std::endl;
    }

    // Example 5: Rounding considerations
    {
        std::cout << "Example 5: Rounding Support\n";
        std::cout << "---------------------------\n";

        // Full precision result that needs rounding to target precision
        blocksignificand<64, uint32_t, BitEncoding::Twos> full_precision;
        blocksignificand<32, uint32_t, BitEncoding::Twos> rounded_result;

        // Simulate a calculation result that needs rounding
        full_precision.setbits(0x123456789ABCDEF0ULL);

        std::cout << "Full precision: " << to_hex(full_precision) << std::endl;

        // Extract the upper 32 bits for the rounded result
        uint64_t full_bits = full_precision.to_ull();
        uint32_t upper_bits = static_cast<uint32_t>(full_bits >> 32);
        uint32_t lower_bits = static_cast<uint32_t>(full_bits);

        rounded_result.setbits(upper_bits);

        std::cout << "Upper 32 bits: " << to_hex(rounded_result) << std::endl;
        std::cout << "Lower 32 bits: 0x" << std::hex << lower_bits << std::dec << std::endl;

        // Check if rounding is needed (look at bit 31 of lower part)
        bool round_up = (lower_bits & 0x80000000U) != 0;
        std::cout << "Round up needed: " << (round_up ? "YES" : "NO") << std::endl;

        if (round_up) {
            // Simple round-to-nearest-even logic would go here
            std::cout << "Rounding would increment the result\n";
        }
        std::cout << std::endl;
    }

    // Example 6: Performance characteristics
    {
        std::cout << "Example 6: Performance Characteristics\n";
        std::cout << "--------------------------------------\n";

        // Different block sizes for same precision
        blocksignificand<128, uint8_t, BitEncoding::Twos> blocks_8;
        blocksignificand<128, uint16_t, BitEncoding::Twos> blocks_16;
        blocksignificand<128, uint32_t, BitEncoding::Twos> blocks_32;
        blocksignificand<128, uint64_t, BitEncoding::Twos> blocks_64;

        std::cout << "128-bit significand with different block sizes:\n";
        std::cout << "8-bit blocks:  " << blocks_8.nrBlocks() << " blocks\n";
        std::cout << "16-bit blocks: " << blocks_16.nrBlocks() << " blocks\n";
        std::cout << "32-bit blocks: " << blocks_32.nrBlocks() << " blocks\n";
        std::cout << "64-bit blocks: " << blocks_64.nrBlocks() << " blocks\n";

        std::cout << "\nPerformance considerations:\n";
        std::cout << "- Fewer blocks = fewer operations for multi-block arithmetic\n";
        std::cout << "- Larger blocks = better utilization of CPU word size\n";
        std::cout << "- Choice depends on target architecture and precision needs\n";
        std::cout << std::endl;
    }

    // Example 7: Integration with floating-point operations
    {
        std::cout << "Example 7: Integration Context\n";
        std::cout << "------------------------------\n";

        std::cout << "blocksignificant is used internally by:\n";
        std::cout << "- cfloat: for IEEE-754 compatible arithmetic\n";
        std::cout << "- posit: for posit arithmetic with variable precision\n";
        std::cout << "- areal: for adaptive precision floating-point\n";
        std::cout << "- Custom floating-point implementations\n\n";

        std::cout << "Typical usage pattern:\n";
        std::cout << "1. Extract significand from floating-point encoding\n";
        std::cout << "2. Choose appropriate bit encoding for operation\n";
        std::cout << "3. Perform arithmetic with proper alignment\n";
        std::cout << "4. Round result to target precision\n";
        std::cout << "5. Pack back into floating-point format\n";
        std::cout << std::endl;
    }

    std::cout << "BlockSignificant educational examples completed!\n";
    std::cout << "\nKey takeaways:\n";
    std::cout << "1. Different bit encodings optimize different operations\n";
    std::cout << "2. Radix point management is external to blocksignificant\n";
    std::cout << "3. Normalization and shifting support denormalized numbers\n";
    std::cout << "4. Rounding support enables accurate floating-point arithmetic\n";
    std::cout << "5. Block size choice affects performance characteristics\n";
    std::cout << "6. Used internally by all Universal floating-point types\n";

    return EXIT_SUCCESS;
}