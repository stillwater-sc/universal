// blocksignificand_operations.cpp: educational example for blocksignificand usage
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
    std::cout << "BlockSignificand Operations: Floating-Point Significand Management\n";
    std::cout << "==================================================================\n\n";

    // Example 1: Basic blocksignificand construction and properties
    {
        std::cout << "Example 1: Basic BlockSignificand Construction\n";
        std::cout << "----------------------------------------------\n";

        blocksignificand<32, uint32_t> sig1;
        blocksignificand<64, uint32_t> sig2;

        std::cout << "32-bit significand:\n";
        std::cout << "  Number of blocks: " << sig1.nrBlocks << std::endl;
        std::cout << "  Bits per block:   " << sig1.bitsInBlock << std::endl;
        std::cout << "  Radix point:      " << sig1.radix() << std::endl;

        std::cout << "64-bit significand:\n";
        std::cout << "  Number of blocks: " << sig2.nrBlocks << std::endl;
        std::cout << "  Bits per block:   " << sig2.bitsInBlock << std::endl;
        std::cout << "  Radix point:      " << sig2.radix() << std::endl;
        std::cout << std::endl;
    }

    // Example 2: Setting and getting bits
    {
        std::cout << "Example 2: Bit Manipulation\n";
        std::cout << "---------------------------\n";

        blocksignificand<32, uint32_t> significand;

        // Set some bits to create a pattern
        significand.setbit(31, true);  // MSB
        significand.setbit(30, true);
        significand.setbit(28, true);
        significand.setbit(24, true);

        std::cout << "Bit pattern: " << to_binary(significand, true) << std::endl;
        std::cout << "As hex:      " << to_hex(significand) << std::endl;

        // Test individual bits
        std::cout << "Bit 31: " << (significand.test(31) ? "1" : "0") << std::endl;
        std::cout << "Bit 30: " << (significand.test(30) ? "1" : "0") << std::endl;
        std::cout << "Bit 29: " << (significand.test(29) ? "1" : "0") << std::endl;
        std::cout << "Bit 28: " << (significand.test(28) ? "1" : "0") << std::endl;
        std::cout << std::endl;
    }

    // Example 3: Addition operation
    {
        std::cout << "Example 3: Addition Operation\n";
        std::cout << "-----------------------------\n";

        blocksignificand<32, uint32_t> lhs, rhs, result;

        // Set up operands - representing normalized significands
        lhs.setbits(0x80000000U);    // 1.0 (MSB represents hidden bit)
        rhs.setbits(0x40000000U);    // 0.5

        std::cout << "LHS: " << to_binary(lhs, true) << " (represents ~1.0)\n";
        std::cout << "RHS: " << to_binary(rhs, true) << " (represents ~0.5)\n";

        // Perform addition
        result.add(lhs, rhs);

        std::cout << "Sum: " << to_binary(result, true) << " (should represent ~1.5)\n";
        std::cout << "Note: Actual interpretation depends on radix point position\n";
        std::cout << std::endl;
    }

    // Example 4: Subtraction operation
    {
        std::cout << "Example 4: Subtraction Operation\n";
        std::cout << "--------------------------------\n";

        blocksignificand<32, uint32_t> lhs, rhs, result;

        // Set up operands
        lhs.setbits(0xC0000000U);    // 1.5 (11.0 in binary)
        rhs.setbits(0x40000000U);    // 0.5 (01.0 in binary)

        std::cout << "LHS: " << to_binary(lhs, true) << " (represents ~1.5)\n";
        std::cout << "RHS: " << to_binary(rhs, true) << " (represents ~0.5)\n";

        // Perform subtraction
        result.sub(lhs, rhs);

        std::cout << "Difference: " << to_binary(result, true) << " (should represent ~1.0)\n";
        std::cout << std::endl;
    }

    // Example 5: Multiplication operation
    {
        std::cout << "Example 5: Multiplication Operation\n";
        std::cout << "-----------------------------------\n";

        blocksignificand<32, uint32_t> lhs, rhs, result;

        // Set up smaller operands to avoid overflow in the demo
        lhs.setbits(0x80000000U);    // 1.0
        rhs.setbits(0x60000000U);    // 0.75 (0.11 in binary)

        std::cout << "LHS: " << to_binary(lhs, true) << " (represents ~1.0)\n";
        std::cout << "RHS: " << to_binary(rhs, true) << " (represents ~0.75)\n";

        // Perform multiplication
        result.mul(lhs, rhs);

        std::cout << "Product: " << to_binary(result, true) << " (result needs interpretation)\n";
        std::cout << "Note: Multiplication result needs proper scaling in context\n";
        std::cout << std::endl;
    }

    // Example 6: Division operation
    {
        std::cout << "Example 6: Division Operation\n";
        std::cout << "-----------------------------\n";

        blocksignificand<32, uint32_t> dividend, divisor, result;

        // Set up operands
        dividend.setbits(0xC0000000U);  // 1.5 (represents 3.0 in some contexts)
        divisor.setbits(0x80000000U);   // 1.0 (represents 2.0 in some contexts)

        std::cout << "Dividend: " << to_binary(dividend, true) << std::endl;
        std::cout << "Divisor:  " << to_binary(divisor, true) << std::endl;

        // Perform division
        result.div(dividend, divisor);

        std::cout << "Quotient: " << to_binary(result, true) << std::endl;
        std::cout << "Note: Division result interpretation depends on input scaling\n";
        std::cout << std::endl;
    }

    // Example 7: Shift operations
    {
        std::cout << "Example 7: Shift Operations\n";
        std::cout << "---------------------------\n";

        blocksignificand<32, uint32_t> original, left_shifted, right_shifted;

        // Set up a test pattern
        original.setbits(0x12345678U);

        std::cout << "Original: " << to_hex(original) << " = " << to_binary(original, true) << std::endl;

        // Left shift
        left_shifted = original;
        left_shifted <<= 4;
        std::cout << "Left << 4: " << to_hex(left_shifted) << " = " << to_binary(left_shifted, true) << std::endl;

        // Right shift
        right_shifted = original;
        right_shifted >>= 4;
        std::cout << "Right >> 4: " << to_hex(right_shifted) << " = " << to_binary(right_shifted, true) << std::endl;
        std::cout << std::endl;
    }

    // Example 8: Radix point management
    {
        std::cout << "Example 8: Radix Point Management\n";
        std::cout << "---------------------------------\n";

        blocksignificand<32, uint32_t> sig1, sig2;

        std::cout << "Default radix point for 32-bit: " << sig1.radix() << std::endl;

        // The radix point determines interpretation of the bit pattern
        sig1.setbits(0xC0000000U);  // Same bit pattern, different interpretations

        std::cout << "Bit pattern: " << to_binary(sig1, true) << std::endl;
        std::cout << "With radix at " << sig1.radix() << ": represents a value with binary point at bit " << sig1.radix() << std::endl;

        // In blocksignificand, the radix point is managed internally
        // and depends on the operation context (add/sub vs mul/div)
        std::cout << "The radix point position affects how arithmetic results are interpreted\n";
        std::cout << std::endl;
    }

    // Example 9: Different block sizes performance characteristics
    {
        std::cout << "Example 9: Block Size Characteristics\n";
        std::cout << "-------------------------------------\n";

        blocksignificand<128, uint8_t> sig_8bit;
        blocksignificand<128, uint16_t> sig_16bit;
        blocksignificand<128, uint32_t> sig_32bit;
        blocksignificand<128, uint64_t> sig_64bit;

        std::cout << "128-bit significand with different block types:\n";
        std::cout << "uint8_t blocks:  " << sig_8bit.nrBlocks << " blocks\n";
        std::cout << "uint16_t blocks: " << sig_16bit.nrBlocks << " blocks\n";
        std::cout << "uint32_t blocks: " << sig_32bit.nrBlocks << " blocks\n";
        std::cout << "uint64_t blocks: " << sig_64bit.nrBlocks << " blocks\n";

        std::cout << "\nBlock size affects:\n";
        std::cout << "- Number of operations needed for multi-precision arithmetic\n";
        std::cout << "- Memory access patterns and cache efficiency\n";
        std::cout << "- SIMD optimization opportunities\n";
        std::cout << std::endl;
    }

    // Example 10: Integration context
    {
        std::cout << "Example 10: Integration with Universal Number Systems\n";
        std::cout << "-----------------------------------------------------\n";

        std::cout << "BlockSignificand is used internally by:\n";
        std::cout << "• blocktriple - for complete floating-point arithmetic\n";
        std::cout << "• cfloat - for IEEE-754 compatible operations\n";
        std::cout << "• posit - for posit arithmetic with variable precision\n";
        std::cout << "• areal - for adaptive precision floating-point\n";
        std::cout << "• Custom number systems requiring significand manipulation\n\n";

        std::cout << "Key design principles:\n";
        std::cout << "• Optimized for specific arithmetic operations\n";
        std::cout << "• Block-based storage for arbitrary precision\n";
        std::cout << "• Radix point management for proper scaling\n";
        std::cout << "• Efficient multi-limb arithmetic operations\n";
        std::cout << std::endl;
    }

    std::cout << "BlockSignificand educational examples completed!\n";
    std::cout << "\nKey takeaways:\n";
    std::cout << "1. blocksignificand provides multi-precision significand arithmetic\n";
    std::cout << "2. Template parameters: bit count and block type (no encoding parameter)\n";
    std::cout << "3. Radix point is managed internally based on operation context\n";
    std::cout << "4. Provides add, sub, mul, div operations optimized for floating-point\n";
    std::cout << "5. Block size choice affects performance and memory characteristics\n";
    std::cout << "6. Used as building block for all Universal floating-point types\n";
    std::cout << "7. Designed for in-place operations to minimize copying\n";

    return EXIT_SUCCESS;
}