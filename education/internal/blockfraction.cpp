// blockfraction_usage.cpp: educational example for blockfraction usage
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <universal/internal/blockfraction/blockfraction.hpp>
#include <universal/verification/test_suite.hpp>

using namespace sw::universal;

/*
 * floating-point fractions are by definition unsigned, and typically use a 1's complement representation.
 * As arithmetic operations on fractions introduce additional bits, the radix point is controllable.
 * 
 * The blockfraction class is a low-level building block for floating-point types such as cfloat and posit.
 * Anytime we need to manipulate the fraction bits of a floating-point number, we use blockfraction.
 * 
 * A secondary abstraction, blocksignificand, is used to represent the fraction bits of a floating-point number
 * including the hidden bit. Anytime we need to manipulate the significand of a floating-point number, 
 * we use blocksignificand.
 */
int main() {
    std::cout << "BlockFraction Usage: Floating-Point Fraction Management\n";
    std::cout << "=======================================================\n\n";

    // Example 1: Basic blockfraction creation and manipulation
    {
        std::cout << "Example 1: Basic BlockFraction Construction and Radix manipulation\n";
        std::cout << "-----------------------------------------\n";

        blockfraction<11, uint32_t> sp; // default creates a 11 bit fraction of the format .fff...ff, that is, radix point after bit 26
        std::cout << to_binary(sp, true) << " : " << sp << '\n';
        sp.setradix(10); // bring the radix point in to 0.fffff
        std::cout << to_binary(sp, true) << " : " << sp << '\n';
        sp.setradix(9); // bring the radix point in to 00.f'ffff'ffff
        std::cout << to_binary(sp, true) << " : " << sp << '\n';
        sp.setradix(8); // bring the radix point in to 000.ffff'ffff
        std::cout << to_binary(sp, true) << " : " << sp << '\n';

        sp.setbit(7); // with radix at bit 7, set value to 0.5
        std::cout << to_binary(sp, true) << " : " << sp << '\n';
		std::cout << std::endl;
	}

	// Example 2: Basic arithmetic operations
    {
        std::cout << "Example 2: Basic BlockFraction Operations\n";
        std::cout << "-----------------------------------------\n";

		// construct a blockfraction with 26 bits using uint32_t as the underlying block type
        blockfraction<26, uint32_t> frac1, frac2, result;

		// let's set the radix point to be after the 23rd bit to emulate a single precision fraction
		frac1.setradix(23);
		frac2.setradix(23);
		result.setradix(23);

		frac1.setbits(0x040'0000U);  // 0.5 in single precision floating-point format
		frac2.setbits(0x020'0000U);  // 0.25 in single precision floating-point format

        std::cout << "Fraction 1: " << to_binary(frac1, true) << " : " << frac1 << '\n'; // represents 0.5
        std::cout << "Fraction 2: " << to_binary(frac2, true) << " : " << frac2 << '\n'; // represents 0.25

        // Basic arithmetic operations
        result.add(frac1, frac2);
        std::cout << "Addition:   " << to_binary(result, true) << " : " << result << '\n'; // should equal 0.75

        result.sub(frac1, frac2);
        std::cout << "Subtraction:" << to_binary(result, true) << " : " << result << '\n'; // should equal 0.25
        std::cout << std::endl;
    }

    // Example 3: Radix point positioning and interpretation
    {
        std::cout << "Example 3: Radix Point Positioning\n";
        std::cout << "-----------------------------------\n";

        blockfraction<16, uint16_t> fraction;

        // Same bit pattern, different interpretations based on radix point
        fraction.setbits(0xC000u);  // 11000000...

        std::cout << "Bit pattern: " << to_binary(fraction, true) << '\n';
        std::cout << "Different radix point interpretations:\n";
        std::cout << "   Radix after bit 16:  .11000... = 0.75\n";
        ReportValue(fraction);
        std::cout << "   Radix after bit 15:   1.1000... = 1.5\n";
		fraction.setradix(15);
        ReportValue(fraction);
        std::cout << "   Radix after bit 14:   11.000... = 3.0\n";
        fraction.setradix(14);
        ReportValue(fraction);
        std::cout << "   Radix after bit 13:   110.00... = 6.0\n";
        fraction.setradix(13);
        ReportValue(fraction);
        std::cout << "   Radix after bit 12:   1100.0... = 12.0\n";
        fraction.setradix(12);
        ReportValue(fraction);
		std::cout << std::endl;
    }

	// Example 4: Extraction of fraction bits from floating-point
    {
        std::cout << "Example 4: Extract fraction bits from floating-point\n";
        std::cout << "---------------------------------------------\n";

        blockfraction<32, uint32_t> frac1, frac2;
		frac1.setradix(23); // emulate single precision floating-point fraction
		frac2.setradix(23);

        // Simulate single precision floating-point fractions that need alignment due to different exponents
        float f1{ 1.25f }, f2{ 24.0f };
        frac1.setbits(fractionBits(f1));  // 0.25 (0.01 in binary)
        frac2.setbits(fractionBits(f2));  // 0.5  (0.1 in binary)

        std::cout << "Fraction of (1.25 * 2^0) : " << to_binary(frac1, true) << std::endl;
        std::cout << "Fraction of (1.50 * 2^4) : " << to_binary(frac2, true) << std::endl;
        std::cout << std::endl;
    }

    // Example 5: Normalization after arithmetic
    {
        std::cout << "Example 5: Normalization After Arithmetic\n";
        std::cout << "-----------------------------------------\n";

        blockfraction<32, uint32_t> unnormalized, normalized;
		unnormalized.setradix(23); // emulate single precision floating-point fraction
		normalized.setradix(23);

        // Simulate a result that needs normalization (no leading 1)
        unnormalized.setbits(0x0030'0000U);  // 0.11 (needs left shift to get leading 1)

        std::cout << "Unnormalized   : " << to_binary(unnormalized, true) << std::endl;

        // Normalize by shifting until we get a leading 1
        normalized = unnormalized;
        int left_shifts = 0;
        while (!normalized.test(23) && left_shifts < 32) {
            normalized <<= 1;
            ++left_shifts;
        }

        std::cout << "Normalized     :   " << to_binary(normalized, true) << std::endl;
        std::cout << "Shifts needed  : " << left_shifts << std::endl;
        std::cout << "This shift count can be used to adjust the exponent in the final result\n";
        std::cout << std::endl;
    }


    std::cout << "BlockFraction educational examples!\n";
    std::cout << "\nKey takeaways:\n";
    std::cout << "1. blockfraction manages floating-point fraction bits efficiently\n";
    std::cout << "2. Fraction arithmetic operations\n";
    std::cout << "3. Radix point interpretation depends on arithmetic operation context\n";
    std::cout << "4. Floating-point fraction extraction is provided\n";
    std::cout << "5. Fraction normalization\n";

    return EXIT_SUCCESS;
}