// blocksignificand_operations.cpp: educational example for blocksignificand usage
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project
#include <universal/utility/directives.hpp>
#include <universal/native/ieee754.hpp>
#include <universal/internal/blocksignificand/blocksignificand.hpp>
#include <iostream>
#include <string>

using namespace sw::universal;

/*

A blocksignificand is a 2's complement binary encoding with a radix point that is aligned
with the hidden bit of the fraction encoding in a floating-point representation.

The main goal of the blocksignificand abstraction is to support arbitrary floating-point
number systems with a high-quality, high-performance arithmetic engine.

The expensive part in these abstractions is the need to receive, expand, and align
bit strings, so special attention is given to fast implementations using copies.
This is acceptable, and leads to cleaner code, for small representations. However,
for very large representations these copies become prohibitive, and for those situations
the blocksignificand is not a good solution.

*/

int main() {
    std::cout << "BlockSignificand Operations: Floating-Point Significand Management\n";
    std::cout << "==================================================================\n\n";

    // Example 1: Basic blocksignificand construction and properties
    {
        std::cout << "Example 1: Basic BlockSignificand Construction\n";
        std::cout << "----------------------------------------------\n";

		blocksignificand<32, uint32_t> sig1; sig1.setradix(23); // set up for single precision floating-point
		blocksignificand<64, uint32_t> sig2; sig2.setradix(52); // set up for double precision floating-point

        std::cout << "Significand of a single precision floating-point:\n";
        std::cout << "  Number of blocks: " << sig1.nrBlocks << std::endl;
        std::cout << "  Bits per block:   " << sig1.bitsInBlock << std::endl;
        std::cout << "  Radix point:      " << sig1.radix() << std::endl;

        std::cout << "Significand of a double precision floating-point:\n";
        std::cout << "  Number of blocks: " << sig2.nrBlocks << std::endl;
        std::cout << "  Bits per block:   " << sig2.bitsInBlock << std::endl;
        std::cout << "  Radix point:      " << sig2.radix() << std::endl;
        std::cout << std::endl;
    }

    // Example 2: Setting and getting bits
    {
        std::cout << "Example 2: Bit Manipulation\n";
        std::cout << "---------------------------\n";

		blocksignificand<32, uint32_t> significand; significand.setradix(23); // set up for single precision floating-point

        // Set some bits to create a pattern
		significand.setbits(0xFFFFFFU); // set to all fraction bits set (23 bits) and the hidden bit (1 bit)

        std::cout << "Bit pattern: " << to_binary(significand, true) << std::endl;
        std::cout << "As hex:      " << to_hex(significand, true) << std::endl;

        // Test individual bits
        std::cout << "Bit 31: " << (significand.test(31) ? "1" : "0") << std::endl;
        std::cout << "Bit 24: " << (significand.test(24) ? "1" : "0") << std::endl;
        std::cout << "Bit 23: " << (significand.test(23) ? "1" : "0") << std::endl;
        std::cout << "Bit 22: " << (significand.test(22) ? "1" : "0") << std::endl;
        std::cout << "Bit  4: " << (significand.test( 4) ? "1" : "0") << std::endl;
        std::cout << std::endl;
    }

    // Example 3: Addition operation
    {
        std::cout << "Example 3: Addition Operation\n";
        std::cout << "-----------------------------\n";

		blocksignificand<16, uint8_t> lhs, rhs, result; // set up for half precision floating-point
        constexpr int radix = 10;
        lhs.setbits(0x0440);   // 0b0000'01.00'0100'0000 = 1.0625 in 16-bit blocksignificand form
        lhs.setradix(radix);
		rhs.setbits(0x0400);   // 0b0000'01.00'0000'0000 = 1.0000 in 16-bit blocksignificand form
        rhs.setradix(radix);
        std::cout << to_binary(lhs) << " : " << lhs << '\n';
        std::cout << to_binary(rhs) << " : " << rhs << '\n';
        result.add(lhs , rhs);
		result.setradix(radix); // inputs are aligned and are normal, hence radix of output is one bit up
        std::cout << to_binary(result) << " : " << result << '\n';
        uint16_t fractionBits = static_cast<uint16_t>(result.fraction_ull());
        std::cout << to_binary(fractionBits, true, radix) << '\n';
        std::cout << std::endl;
    }

    // Example 4: Subtraction operation
    {
        std::cout << "Example 4: Subtraction Operation\n";
        std::cout << "--------------------------------\n";

        blocksignificand<16, uint8_t> lhs, rhs, result;

        constexpr int radix = 10;
        lhs.setbits(0x0440);   // 0b0000'01.00'0100'0000 = 1.0625 in 16-bit blocksignificand form
        lhs.setradix(radix);
        rhs.setbits(0x0400);   // 0b0000'01.00'0000'0000 = 1.0000 in 16-bit blocksignificand form
        rhs.setradix(radix);
        std::cout << to_binary(lhs) << " : " << lhs << '\n';
        std::cout << to_binary(rhs) << " : " << rhs << '\n';
        result.sub(lhs, rhs);
        result.setradix(radix); // inputs are aligned and result has lost its hidden bit
        std::cout << to_binary(result) << " : " << result << '\n';
        uint16_t fractionBits = static_cast<uint16_t>(result.fraction_ull());
        std::cout << to_binary(fractionBits, true, radix) << '\n';
        std::cout << std::endl;
    }

    // Example 5: Multiplication operation
    {
        std::cout << "Example 5: Multiplication Operation\n";
        std::cout << "-----------------------------------\n";

		constexpr unsigned fractionBits = 10; // half-precision floating-point has 10 fraction bits
		constexpr unsigned hiddenBit = 1;
		constexpr unsigned capacityBits = 2; // guard bits to capture overflow
		constexpr unsigned mantissaBits = fractionBits + hiddenBit;
		constexpr unsigned significandBits = 2 * mantissaBits + capacityBits; // multiplication doubles the number of mantissa bits
        blocksignificand<significandBits, uint8_t> lhs, rhs, result;

        constexpr int radix = fractionBits;
        lhs.setbits(0x0440);   // 0b0000'01.00'0100'0000 = 1.0625 in 16-bit blocksignificand form
        lhs.setradix(radix);
        rhs.setbits(0x0600);   // 0b0000'01.10'0000'0000 = 1.5000 in 16-bit blocksignificand form
        rhs.setradix(radix);
        std::cout << to_binary(lhs) << " : " << lhs << '\n';
        std::cout << to_binary(rhs) << " : " << rhs << '\n';
        result.mul(lhs, rhs);
		constexpr int resultRadix = 2 * radix;
        result.setradix(resultRadix); // multiplication doubles the number of fraction bits
        std::cout << to_binary(result) << " : " << result << '\n';
        std::cout << std::endl;
    }

    // Example 6: Division operation
    {
        std::cout << "Example 6: Division Operation\n";
        std::cout << "-----------------------------\n";

        constexpr unsigned fractionBits = 10; // half-precision floating-point has 10 fraction bits
        constexpr unsigned hiddenBit = 1;
        constexpr unsigned capacityBits = 2; // guard bits to capture overflow
        constexpr unsigned mantissaBits = fractionBits + hiddenBit;
        constexpr unsigned significandBits = 2 * mantissaBits + capacityBits; // division doubles the number of mantissa bits
        blocksignificand<significandBits, uint8_t> lhs, rhs, result;

        constexpr int radix = 10;
        lhs.setbits(0x0700);   // 0b0000'01.11'0000'0000 = 1.7500 in 16-bit blocksignificand form
        lhs.setradix(radix);
        rhs.setbits(0x0500);   // 0b0000'01.01'0000'0000 = 1.2500 in 16-bit blocksignificand form
        rhs.setradix(radix);
        std::cout << to_binary(lhs) << " : " << lhs << '\n';
        std::cout << to_binary(rhs) << " : " << rhs << '\n';
        result.div(lhs, rhs);
        constexpr int resultRadix = 2 * radix;
        result.setradix(resultRadix);
        std::cout << to_binary(result) << " : " << result << '\n';
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

        blocksignificand<32, uint32_t> sig1;
        [[maybe_unused]] blocksignificand<32, uint32_t> sig2;

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
        std::cout << "- blocktriple - for complete floating-point arithmetic\n";
        std::cout << "- cfloat - for IEEE-754 compatible operations\n";
        std::cout << "- posit - for posit arithmetic with variable precision\n";
        std::cout << "- areal - for adaptive precision floating-point\n";
        std::cout << "- Custom number systems requiring significand manipulation\n\n";

        std::cout << "Key design principles:\n";
        std::cout << "- Optimized for specific arithmetic operations\n";
        std::cout << "- Block-based storage for arbitrary precision\n";
        std::cout << "- Radix point management for proper scaling\n";
        std::cout << "- Efficient multi-limb arithmetic operations\n";
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
