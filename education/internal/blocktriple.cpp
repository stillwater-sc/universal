// blocktriple_arithmetic.cpp: educational example for blocktriple arithmetic operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project
#include <universal/utility/directives.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <iostream>
#include <string>

using namespace sw::universal;

int main() {
    std::cout << "BlockTriple Arithmetic: Complete Floating-Point Operations\n";
    std::cout << "==========================================================\n\n";

    // Example 1: Basic blocktriple construction and components
    {
        std::cout << "Example 1: BlockTriple Construction and Components\n";
        std::cout << "--------------------------------------------------\n";

		std::cout << std::endl;
    }

    // Example 2: Addition with different scales
    {
        std::cout << "Example 2: Addition with Different Scales\n";
        std::cout << "-----------------------------------------\n";

        /*
        * The algorithm that blocktriple uses is to set up the arithmetic operation
		* such that the result has enough bits to capture all rounding and overflow information.
		* Thus the sequence is:
        *  1. 'normalize' the input arguments to fit in the target format.
		*  2. execute the arithmetic operation
		*  3. scale and round the result to the target format
        */
		constexpr unsigned fbits = 23; // number of fraction bits in the target floating-point format
        // BlockTripleOperator::ADD creates a normalized mantissa of 3 integer bits + fbits + 3 guard bits for rounding
		// the 3 integer bits are required to capture 2's complement results of overflow
		// addition and subtraction are the most complex operations as they require scale alignment
		// addition/subtraction is done in 2's complement
        blocktriple<fbits, BlockTripleOperator::ADD, uint32_t> a, b, sum;
		a.set(false, 0, 0x0400'0000ULL, false, false);  // 1.0
		b.set(false, -4, 0x0440'0000ULL, false, false); // 1.0625 * 2^-4 = 0.06640625
        std::cout << "a       : " << to_binary(a, true) << " : " << a << '\n';
        std::cout << "b       : " << to_binary(b, true) << " : " << b << '\n';
        sum.add(a, b);
        std::cout << "sum     : " << to_binary(sum, true) << " : " << sum << '\n';
		// convert to single precision floating-point
		// convert() would need to do all the heavy lifting of rounding and normalization
		// as reference, convert() for cfloat is about 200 lines of code
        std::cout << std::endl;
    }

    // Example 3: Multiplication scaling
    {
        std::cout << "Example 3: Multiplication Scaling\n";
        std::cout << "---------------------------------\n";

        constexpr unsigned fbits = 10; // number of fraction bits in the target half precision floating-point format
        // BlockTripleOperator::MUL creates a normalized mantissa of 2*fbits + 2 capacity bits for overflow
		// multiplication is done in 1's complement
        blocktriple<fbits, BlockTripleOperator::MUL, uint32_t> a, b, product;
		a.set(false, 0, 0x00'0400ULL, false, false); // 1.0  in half precision
		b.set(false, 0, 0x00'0440ULL, false, false); // 1.0625 in half precision
		std::cout << "a       : " << to_binary(a, true) << '\n';  // given the radix point, the 'value' of a is not 1.0
        std::cout << "b       : " << to_binary(b, true) << '\n';
        product.mul(a, b);
        std::cout << "product : " << to_binary(product, true) << " : " << product << '\n';
		// convert to half precision floating-point
		// convert() would need to do all the heavy lifting of rounding and normalization
		// as reference, convert() for cfloat is about 200 lines of code
        std::cout << std::endl;
    }

    // Example 4: Division with remainder handling
    {
        std::cout << "Example 4: Division with Remainder Handling\n";
        std::cout << "-------------------------------------------\n";

        blocktriple<23, BlockTripleOperator::DIV, uint32_t> dividend, divisor, quotient;

        std::cout << std::endl;
    }

    // Example 5: Special values (zero, infinity, NaN)
    {
        std::cout << "Example 5: Special Values\n";
        std::cout << "------------------------\n";

        blocktriple<23, BlockTripleOperator::ADD, uint32_t> zero, positive_inf, negative_inf;

        // Create special values
        zero.setzero(false);
        positive_inf.setinf(false);  // positive infinity
        negative_inf.setinf(true);   // negative infinity

        std::cout << "Zero: " << zero << " (iszero=" << zero.iszero() << ")\n";
        std::cout << "Positive infinity: " << positive_inf << " (isinf=" << positive_inf.isinf() << ")\n";
        std::cout << "Negative infinity: " << negative_inf << " (isinf=" << negative_inf.isinf() << ")\n";

        // Operations with special values
        blocktriple<23, BlockTripleOperator::ADD, uint32_t> normal, result;
        normal.set(false, 0, 0x80000000ULL, false, false);  // 1.0

        std::cout << "\nOperations with special values:\n";
		result.add(normal, zero);
        std::cout << "1.0 + 0 = " << result << std::endl;
        //std::cout << "1.0 * 0 = " << (normal * zero) << std::endl;
		result.add(normal, positive_inf);
        std::cout << "1.0 + inf = " << result << std::endl;
        std::cout << std::endl;
    }

    std::cout << "BlockTriple educational examples completed!\n";
    std::cout << "\nKey takeaways:\n";
    std::cout << "1. blocktriple provides complete floating-point arithmetic\n";
    std::cout << "2. Handles sign, exponent (scale), and significand together\n";
    std::cout << "3. Supports denormalized intermediate results\n";
    std::cout << "4. Manages scale alignment for addition/subtraction\n";
    std::cout << "5. Handles scale arithmetic for multiplication/division\n";
    std::cout << "6. Supports special values (zero, infinity, NaN)\n";
    std::cout << "7. Provides foundation for rounding and format conversion\n";
    std::cout << "8. Used internally by cfloat, posit, and custom floating-point types\n";
    std::cout << "9. Enables accurate intermediate calculations with extended precision\n";

    return EXIT_SUCCESS;
}