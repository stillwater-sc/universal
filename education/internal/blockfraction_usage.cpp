// blockfraction_usage.cpp: educational example for blockfraction usage
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project
#include <universal/utility/directives.hpp>
#include <universal/internal/blockfraction/blockfraction.hpp>
#include <iostream>
#include <string>

using namespace sw::universal;

int main() {
    std::cout << "BlockFraction Usage: Floating-Point Fraction Management\n";
    std::cout << "=======================================================\n\n";

    // Example 1: Basic blockfraction creation and manipulation
    {
        std::cout << "Example 1: Basic BlockFraction Operations\n";
        std::cout << "-----------------------------------------\n";

        blockfraction<32, uint32_t> frac1, frac2, result;

        // Set fraction values (interpreting as binary fractions)
        frac1.setbits(0x80000000U);  // 1.0 (MSB set)
        frac2.setbits(0x40000000U);  // 0.5 (second MSB set)

        std::cout << "Fraction 1: " << to_binary(frac1, true) << " (represents 1.0)\n";
        std::cout << "Fraction 2: " << to_binary(frac2, true) << " (represents 0.5)\n";

        // Basic arithmetic operations
        result.add(frac1, frac2);
        std::cout << "Addition:   " << to_binary(result, true) << " (should represent 1.5)\n";

        result.sub(frac1, frac2);
        std::cout << "Subtraction:" << to_binary(result, true) << " (should represent 0.5)\n";
        std::cout << std::endl;
    }

    // Example 2: Fraction scaling for different operations
    //{
    //    std::cout << "Example 2: Scaling for Different Arithmetic Operations\n";
    //    std::cout << "------------------------------------------------------\n";

    //    blockfraction<32, uint32_t> multiplicand, multiplier;
    //    blockfraction<64, uint32_t> product;  // Wider result for multiplication

    //    // Set up operands for multiplication
    //    multiplicand.setbits(0x80000000U);  // 1.0
    //    multiplier.setbits(0x60000000U);    // 0.75 (0.11 in binary)

    //    std::cout << "Multiplicand: " << to_binary(multiplicand, true) << " (1.0)\n";
    //    std::cout << "Multiplier:   " << to_binary(multiplier, true) << " (0.75)\n";

    //    // For multiplication, we need to handle the scaling properly
    //    // The result will be in a different format than the inputs
    //    uint64_t mult_result = static_cast<uint64_t>(multiplicand.to_ull()) * multiplier.to_ull();
    //    product.setbits(mult_result);

    //    std::cout << "Raw product:  " << to_binary(product, true) << std::endl;
    //    std::cout << "Note: Product needs proper scaling interpretation\n";
    //    std::cout << "      (1.0 * 0.75 should equal 0.75)\n";
    //    std::cout << std::endl;
    //}

    // Example 3: Radix point positioning and interpretation
    {
        std::cout << "Example 3: Radix Point Positioning\n";
        std::cout << "-----------------------------------\n";

        blockfraction<32, uint32_t> fraction;

        // Same bit pattern, different interpretations based on radix point
        fraction.setbits(0xC0000000U);  // 11000000...

        std::cout << "Bit pattern: " << to_binary(fraction, true) << std::endl;
        std::cout << "Different radix point interpretations:\n";
        std::cout << "  Radix after bit 31: 1.1000... = 1.5\n";
        std::cout << "  Radix after bit 30: 11.000... = 3.0\n";
        std::cout << "  Radix after bit 29: 110.00... = 6.0\n";
        std::cout << "  Radix after bit 28: 1100.0... = 12.0\n";

        // The blockfraction doesn't store the radix point position
        // That's managed by the arithmetic context (add/mul/div operations)
        std::cout << "\nRadix point management is operation-dependent:\n";
        std::cout << "- Addition: Fixed point alignment\n";
        std::cout << "- Multiplication: Product scaling\n";
        std::cout << "- Division: Quotient scaling\n";
        std::cout << std::endl;
    }

    // Example 4: Alignment for addition/subtraction
    {
        std::cout << "Example 4: Alignment for Addition/Subtraction\n";
        std::cout << "---------------------------------------------\n";

        blockfraction<32, uint32_t> aligned_frac1, aligned_frac2;

        // Simulate fractions that need alignment due to different exponents
        // Say we have 1.25 * 2^0 and 1.5 * 2^2 (which is 6.0)
        aligned_frac1.setbits(0xA0000000U);  // 1.25 (1.01 in binary)
        aligned_frac2.setbits(0xC0000000U);  // 1.5  (1.1 in binary)

        std::cout << "Before alignment:\n";
        std::cout << "Frac1 (1.25 * 2^0): " << to_binary(aligned_frac1, true) << std::endl;
        std::cout << "Frac2 (1.5 * 2^2):  " << to_binary(aligned_frac2, true) << std::endl;

        // To add these, we need to align to the same scale
        // The 1.5 * 2^2 needs to be shifted to account for the exponent difference
        // Effectively: 1.25 + (1.5 << 2) = 1.25 + 6.0 = 7.25

        blockfraction<32, uint32_t> shifted_frac2 = aligned_frac2;
        shifted_frac2 <<= 2;  // Shift left by exponent difference

        std::cout << "\nAfter alignment (shifting frac2 left by 2):\n";
        std::cout << "Frac1:          " << to_binary(aligned_frac1, true) << std::endl;
        std::cout << "Frac2 shifted:  " << to_binary(shifted_frac2, true) << std::endl;

        blockfraction<32, uint32_t> aligned_sum;
        aligned_sum.add(aligned_frac1, shifted_frac2);
        std::cout << "Sum:            " << to_binary(aligned_sum, true) << std::endl;
        std::cout << "This represents 7.25 in the appropriate scaling\n";
        std::cout << std::endl;
    }

    // Example 5: Normalization after arithmetic
    {
        std::cout << "Example 5: Normalization After Arithmetic\n";
        std::cout << "-----------------------------------------\n";

        blockfraction<32, uint32_t> unnormalized, normalized;

        // Simulate a result that needs normalization (no leading 1)
        unnormalized.setbits(0x30000000U);  // 0.11 (needs left shift to get leading 1)

        std::cout << "Unnormalized: " << to_binary(unnormalized, true) << std::endl;

        // Normalize by shifting until we get a leading 1
        normalized = unnormalized;
        int left_shifts = 0;
        while (!normalized.test(31) && left_shifts < 32) {
            normalized <<= 1;
            ++left_shifts;
        }

        std::cout << "Normalized:   " << to_binary(normalized, true) << std::endl;
        std::cout << "Shifts needed: " << left_shifts << std::endl;
        std::cout << "This shift count adjusts the exponent in the final result\n";
        std::cout << std::endl;
    }

#ifdef LATER
    // Example 6: Long division with blockfraction
    {
        std::cout << "Example 6: Long Division\n";
        std::cout << "-----------------------\n";

        blockfraction<32, uint32_t> dividend, divisor;

        dividend.setbits(0xE0000000U);  // 1.75 (1.11 in binary)
        divisor.setbits(0x80000000U);   // 1.0  (1.0 in binary)

        std::cout << "Dividend: " << to_binary(dividend, true) << " (1.75)\n";
        std::cout << "Divisor:  " << to_binary(divisor, true) << " (1.0)\n";

        // Perform long division
        auto div_result = longdivision(dividend, divisor);

        std::cout << "Quotient:  " << to_binary(div_result.quo, true) << std::endl;
        std::cout << "Remainder: " << to_binary(div_result.rem, true) << std::endl;
        std::cout << "Expected quotient: 1.75 (since 1.75 / 1.0 = 1.75)\n";
        std::cout << std::endl;
    }

    // Example 7: Guard bits and sticky bits for rounding
    {
        std::cout << "Example 7: Guard Bits and Sticky Bits\n";
        std::cout << "-------------------------------------\n";

        // Extended precision for intermediate calculations
        blockfraction<64, uint32_t> extended_precision;
        blockfraction<32, uint32_t> rounded_result;

        // Simulate a calculation that produces extra precision bits
        extended_precision.setbits(0x123456789ABCDEF0ULL);

        std::cout << "Extended precision: " << to_hex(extended_precision) << std::endl;

        // Extract the main result (upper 32 bits) and guard/sticky bits
        uint64_t full_bits = extended_precision.to_ull();
        uint32_t main_bits = static_cast<uint32_t>(full_bits >> 32);
        uint32_t guard_sticky_bits = static_cast<uint32_t>(full_bits);

        rounded_result.setbits(main_bits);

        std::cout << "Main result bits:     0x" << std::hex << main_bits << std::dec << std::endl;
        std::cout << "Guard/sticky bits:    0x" << std::hex << guard_sticky_bits << std::dec << std::endl;

        // Check rounding conditions
        bool guard_bit = (guard_sticky_bits & 0x80000000U) != 0;
        bool sticky_bits = (guard_sticky_bits & 0x7FFFFFFFU) != 0;
        bool round_bit = (main_bits & 1) != 0;  // LSB of result (for round-to-even)

        std::cout << "\nRounding analysis:\n";
        std::cout << "Guard bit: " << (guard_bit ? "1" : "0") << std::endl;
        std::cout << "Sticky bits: " << (sticky_bits ? "non-zero" : "zero") << std::endl;
        std::cout << "Round bit (LSB): " << (round_bit ? "1" : "0") << std::endl;

        // Round-to-nearest-even logic
        bool should_round_up = guard_bit && (sticky_bits || round_bit);
        std::cout << "Should round up: " << (should_round_up ? "YES" : "NO") << std::endl;

        if (should_round_up) {
            auto incremented = rounded_result + blockfraction<32, uint32_t>(1);
            std::cout << "Rounded result: " << to_hex(incremented) << std::endl;
        } else {
            std::cout << "Rounded result: " << to_hex(rounded_result) << std::endl;
        }
        std::cout << std::endl;
    }

    // Example 8: Different block sizes and their trade-offs
    {
        std::cout << "Example 8: Block Size Trade-offs\n";
        std::cout << "--------------------------------\n";

        blockfraction<128, uint8_t> frac_8bit;
        blockfraction<128, uint16_t> frac_16bit;
        blockfraction<128, uint32_t> frac_32bit;
        blockfraction<128, uint64_t> frac_64bit;

        std::cout << "128-bit fractions with different block sizes:\n";
        std::cout << "8-bit blocks:  " << frac_8bit.nrBlocks << " blocks, "
                  << sizeof(frac_8bit) << " bytes\n";
        std::cout << "16-bit blocks: " << frac_16bit.nrBlocks << " blocks, "
                  << sizeof(frac_16bit) << " bytes\n";
        std::cout << "32-bit blocks: " << frac_32bit.nrBlocks << " blocks, "
                  << sizeof(frac_32bit) << " bytes\n";
        std::cout << "64-bit blocks: " << frac_64bit.nrBlocks << " blocks, "
                  << sizeof(frac_64bit) << " bytes\n";

        std::cout << "\nTrade-offs:\n";
        std::cout << "- Smaller blocks: More operations, better for SIMD\n";
        std::cout << "- Larger blocks: Fewer operations, better CPU utilization\n";
        std::cout << "- Choose based on target architecture and use case\n";
        std::cout << std::endl;
    }
#endif
    std::cout << "BlockFraction educational examples completed!\n";
    std::cout << "\nKey takeaways:\n";
    std::cout << "1. blockfraction manages floating-point fraction bits efficiently\n";
    std::cout << "2. Radix point interpretation depends on arithmetic operation context\n";
    std::cout << "3. Alignment is crucial for addition/subtraction operations\n";
    std::cout << "4. Normalization maintains proper significand format\n";
    std::cout << "5. Guard/sticky bits enable accurate rounding\n";
    std::cout << "6. Block size choice affects performance and memory usage\n";
    std::cout << "7. Used internally by cfloat, posit, and other floating-point types\n";

    return EXIT_SUCCESS;
}