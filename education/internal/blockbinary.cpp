// blockbinary_basics.cpp: educational example demonstrating blockbinary usage
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project
#include <universal/utility/directives.hpp>
#include <universal/native/integers.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <iostream>
#include <string>

using namespace sw::universal;

/*
 * blockbinary is an internal building block for multi-limb arithmetic types. 
 * It is not intended for direct use in applications.
 *
 * blockbinary provides a mechanism to manage bits in blocks of fixed size (8, 16, 32, 64 bits) 
 * to implement arithmetic and logic operators for other number systems.
 */
int main() {
    std::cout << "BlockBinary Basics: Multi-Limb Bit Storage\n";
    std::cout << "=================================================\n\n";

    // Example 1: Basic 128-bit signed integer
    {
        std::cout << "Example 1: 128-bit Signed Integer with 32-bit blocks\n";
        std::cout << "----------------------------------------------------\n";

        blockbinary<64, uint32_t, BinaryNumberType::Signed> a, b, sum, product;

        // Initialize with different values
        a = 1'234'567'890e7;  // Large number
        b =             2e0;

	// constexpr int WIDTH = 20;
        std::cout << "a     = " << to_binary(a) << '\n';
        std::cout << "    b = " << to_binary(b) << '\n';

        // Basic multi-limb integer arithmetic
        sum = a + b;
        product = a * b;

        std::cout << "a + b = " << to_binary(sum) << '\n';
        std::cout << "a * b = " << to_binary(product) << '\n';
        std::cout << std::endl;
    }

    // Example 2: Unsigned vs Signed behavior
    {
        std::cout << "Example 2: Unsigned vs Signed Arithmetic\n";
        std::cout << "----------------------------------------\n";

        // 64-bit numbers for easier visualization
        blockbinary<64, uint32_t, BinaryNumberType::Unsigned> uint64;
        blockbinary<64, uint32_t, BinaryNumberType::Signed> int64;

        // Set to same bit pattern (large positive number)
        uint64.setbits(0xFFFF'FFFF'FFFF'FFFFULL);
        int64.setbits(0xFFFF'FFFF'FFFF'FFFFULL);

        constexpr bool nibbleMarker = true;
		std::uint64_t max_uint64 = uint64.to_ull();
		std::cout << "Max uint64_t: " << max_uint64 << " : " << to_binary(max_uint64, nibbleMarker) << '\n';


        std::cout << "Same bit pattern:\n";
        std::cout << "Unsigned interpretation: " << uint64 << " : " << to_binary(uint64, nibbleMarker) << '\n';
        std::cout << "Signed interpretation:   " << int64 << " : " << to_binary(int64, nibbleMarker) << '\n';

        // Demonstrate overflow behavior
        blockbinary<8, uint8_t, BinaryNumberType::Unsigned> uint8;
        blockbinary<8, uint8_t, BinaryNumberType::Signed> int8;

        uint8 = 200;
        int8 = 100;

        std::cout << "\nOverflow demonstration:\n";
        std::cout << "Unsigned 200 + 100 = " << (uint8 + blockbinary<8, uint8_t, BinaryNumberType::Unsigned>(100)) << '\n';
        std::cout << "Signed 100 + 100 = " << (int8 + blockbinary<8, uint8_t, BinaryNumberType::Signed>(100)) << '\n';
        std::cout << std::endl;
    }

    // Example 3: Different block sizes
    {
        std::cout << "Example 3: Block Size Performance Considerations\n";
        std::cout << "-----------------------------------------------\n";

        // Same precision, different block sizes
        blockbinary<256, uint8_t, BinaryNumberType::Unsigned> blocks_8bit;
        blockbinary<256, uint16_t, BinaryNumberType::Unsigned> blocks_16bit;
        blockbinary<256, uint32_t, BinaryNumberType::Unsigned> blocks_32bit;
        //blockbinary<256, uint64_t, BinaryNumberType::Unsigned> blocks_64bit;

        std::cout << "256-bit number with different block sizes:\n";
        std::cout << "8-bit blocks:  " << blocks_8bit.nrBlocks << " blocks\n";
        std::cout << "16-bit blocks: " << blocks_16bit.nrBlocks << " blocks\n";
        std::cout << "32-bit blocks: " << blocks_32bit.nrBlocks << " blocks\n";
        //std::cout << "64-bit blocks: " << blocks_64bit.nrBlocks << " blocks\n";

        // Set same value in all
        uint64_t test_value = 0x123456789ABCDEF0ULL;
        blocks_8bit = test_value;
        blocks_16bit = test_value;
        blocks_32bit = test_value;
        //blocks_64bit = test_value;

        std::cout << "\nAll representations of 0x123456789ABCDEF0:\n";
        std::cout << "8-bit:  " << to_hex(blocks_8bit) << std::endl;
        std::cout << "16-bit: " << to_hex(blocks_16bit) << std::endl;
        std::cout << "32-bit: " << to_hex(blocks_32bit) << std::endl;
        //std::cout << "64-bit: " << to_hex(blocks_64bit) << std::endl;
        std::cout << std::endl;
    }

    // Example 4: Long division
    {
        std::cout << "Example 4: Long Division with Quotient and Remainder\n";
        std::cout << "----------------------------------------------------\n";

		// longdivision function takes two Signed integers
        blockbinary<64, uint32_t, BinaryNumberType::Signed> dividend, divisor;

        dividend = -1000000000000ULL;  // -1 trillion
        divisor  =      123456789ULL;  // ~123 million

        std::cout << "Dividend: " << dividend << std::endl;
        std::cout << "Divisor:  " << divisor << std::endl;

        // longdivision takes in two Signed integers and returns a struct with quotient and remainder
        auto result = longdivision(dividend, divisor);

        std::cout << "Quotient:  " << result.quo << std::endl;
        std::cout << "Remainder: " << result.rem << std::endl;

        // Verify: dividend = quotient * divisor + remainder
        auto verification = result.quo * divisor + result.rem;
        std::cout << "Verification (quo*div + rem): " << verification << std::endl;
        std::cout << "Matches dividend: " << (verification == dividend ? "YES" : "NO") << std::endl;
        std::cout << std::endl;
    }

    // Example 5: Bit manipulation
    {
        std::cout << "Example 5: Bit Manipulation Operations\n";
        std::cout << "--------------------------------------\n";

        blockbinary<64, uint32_t, BinaryNumberType::Unsigned> value;

        // Set alternating bits
        for (unsigned i = 0; i < 64; i += 2) {
            value.setbit(i, true);
        }

        std::cout << "Alternating bits: " << to_binary(value) << std::endl;
        std::cout << "Hex representation: " << to_hex(value) << std::endl;

        // Shift operations
        auto left_shifted = value << 4;
        auto right_shifted = value >> 4;

        std::cout << "Left shift 4:  " << to_hex(left_shifted) << std::endl;
        std::cout << "Right shift 4: " << to_hex(right_shifted) << std::endl;

        // Logical operations
        blockbinary<64, uint32_t, BinaryNumberType::Unsigned> mask;
        mask = 0xFFFF0000FFFF0000ULL;

        std::cout << "Original: " << to_hex(value) << std::endl;
        std::cout << "Mask:     " << to_hex(mask) << std::endl;
        //std::cout << "AND:      " << to_hex(value & mask) << std::endl;
        //std::cout << "OR:       " << to_hex(value | mask) << std::endl;
        //std::cout << "XOR:      " << to_hex(value ^ mask) << std::endl;
        std::cout << std::endl;
    }

    // Example 6: Maximum and minimum values
    {
        std::cout << "Example 6: Maximum and Minimum Values\n";
        std::cout << "-------------------------------------\n";

        blockbinary<16, uint16_t, BinaryNumberType::Signed> signed_16;
        blockbinary<16, uint16_t, BinaryNumberType::Unsigned> unsigned_16;

        // Maximum positive values
        maxpos(signed_16);
        maxpos(unsigned_16);

        std::cout << "16-bit signed max:   " << signed_16 << " (hex: " << to_hex(signed_16) << ")\n";
        std::cout << "16-bit unsigned max: " << unsigned_16 << " (hex: " << to_hex(unsigned_16) << ")\n";

        // Maximum negative (for signed only)
        maxneg(signed_16);
        std::cout << "16-bit signed min:   " << signed_16 << " (hex: " << to_hex(signed_16) << ")\n";

        // Zero
        signed_16.clear();
        unsigned_16.clear();
        std::cout << "Zero values: " << signed_16 << ", " << unsigned_16 << std::endl;
        std::cout << std::endl;
    }

    std::cout << "BlockBinary educational examples completed!\n";
    std::cout << "\nKey takeaways:\n";
    std::cout << "1. blockbinary provides arbitrary precision integer arithmetic\n";
    std::cout << "2. Block size affects memory usage and performance\n";
    std::cout << "3. Signed vs unsigned affects interpretation and overflow behavior\n";
    std::cout << "4. Comprehensive bit manipulation and arithmetic operations\n";
    std::cout << "5. Long division provides both quotient and remainder\n";

    return EXIT_SUCCESS;
}
