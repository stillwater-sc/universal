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


    }

    // Example 2: Addition with different scales
    {
        std::cout << "Example 2: Addition with Different Scales\n";
        std::cout << "-----------------------------------------\n";

        blocktriple<23, BlockTripleOperator::ADD, uint32_t> a, b, sum;

        // Create two numbers with different scales: 4.0 and 0.5
        a.set(false, 2, 0x80000000ULL, false, false);   // 1.0 * 2^2 = 4.0
        b.set(false, -1, 0x80000000ULL, false, false);  // 1.0 * 2^-1 = 0.5

        std::cout << "Number A: scale=" << a.scale() << ", significand=0x"
                  << std::hex << a.significand() << std::dec << " (represents 4.0)\n";
        std::cout << "Number B: scale=" << b.scale() << ", significand=0x"
                  << std::hex << b.significand() << std::dec << " (represents 0.5)\n";

        // Perform addition
        sum = a + b;

        std::cout << "Sum: scale=" << sum.scale() << ", significand=0x"
                  << std::hex << sum.significand() << std::dec << " (should represent 4.5)\n";
        std::cout << "Addition requires alignment of significands based on scale difference\n";
        std::cout << std::endl;
    }

    // Example 3: Multiplication scaling
    {
        std::cout << "Example 3: Multiplication Scaling\n";
        std::cout << "---------------------------------\n";

        blocktriple<23, uint32_t> multiplicand, multiplier, product;

        // 2.5 * 1.5 = 3.75
        multiplicand.set(false, 1, 0xA0000000ULL, false, false);  // 1.25 * 2^1 = 2.5
        multiplier.set(false, 0, 0xC0000000ULL, false, false);    // 1.5 * 2^0 = 1.5

        std::cout << "Multiplicand: scale=" << multiplicand.scale() << ", sig=0x"
                  << std::hex << multiplicand.significand() << std::dec << " (2.5)\n";
        std::cout << "Multiplier: scale=" << multiplier.scale() << ", sig=0x"
                  << std::hex << multiplier.significand() << std::dec << " (1.5)\n";

        // Perform multiplication
        product = multiplicand * multiplier;

        std::cout << "Product: scale=" << product.scale() << ", sig=0x"
                  << std::hex << product.significand() << std::dec << " (should be 3.75)\n";
        std::cout << "Multiplication: scales add, significands multiply\n";
        std::cout << std::endl;
    }

    // Example 4: Division with remainder handling
    {
        std::cout << "Example 4: Division with Remainder Handling\n";
        std::cout << "-------------------------------------------\n";

        blocktriple<23, uint32_t> dividend, divisor, quotient;

        // 7.0 / 2.0 = 3.5
        dividend.set(false, 2, 0xE0000000ULL, false, false);   // 1.75 * 2^2 = 7.0
        divisor.set(false, 1, 0x80000000ULL, false, false);    // 1.0 * 2^1 = 2.0

        std::cout << "Dividend: scale=" << dividend.scale() << ", sig=0x"
                  << std::hex << dividend.significand() << std::dec << " (7.0)\n";
        std::cout << "Divisor: scale=" << divisor.scale() << ", sig=0x"
                  << std::hex << divisor.significand() << std::dec << " (2.0)\n";

        // Perform division
        quotient = dividend / divisor;

        std::cout << "Quotient: scale=" << quotient.scale() << ", sig=0x"
                  << std::hex << quotient.significand() << std::dec << " (should be 3.5)\n";
        std::cout << "Division: scales subtract, significands divide\n";
        std::cout << std::endl;
    }

    // Example 5: Special values (zero, infinity, NaN)
    {
        std::cout << "Example 5: Special Values\n";
        std::cout << "------------------------\n";

        blocktriple<23, uint32_t> zero, positive_inf, negative_inf;

        // Create special values
        zero.setzero();
        positive_inf.setinf(false);  // positive infinity
        negative_inf.setinf(true);   // negative infinity

        std::cout << "Zero: " << zero << " (iszero=" << zero.iszero() << ")\n";
        std::cout << "Positive infinity: " << positive_inf << " (isinf=" << positive_inf.isinf() << ")\n";
        std::cout << "Negative infinity: " << negative_inf << " (isinf=" << negative_inf.isinf() << ")\n";

        // Operations with special values
        blocktriple<23, uint32_t> normal;
        normal.set(false, 0, 0x80000000ULL, false, false);  // 1.0

        std::cout << "\nOperations with special values:\n";
        std::cout << "1.0 + 0 = " << (normal + zero) << std::endl;
        std::cout << "1.0 * 0 = " << (normal * zero) << std::endl;
        std::cout << "1.0 + inf = " << (normal + positive_inf) << std::endl;
        std::cout << std::endl;
    }

    // Example 6: Denormalized (subnormal) number handling
    {
        std::cout << "Example 6: Denormalized Number Handling\n";
        std::cout << "---------------------------------------\n";

        blocktriple<23, uint32_t> normalized, denormalized;

        // Normal number: 1.0
        normalized.set(false, 0, 0x80000000ULL, false, false);

        // Create a denormalized form (ii.ffffff format from arithmetic)
        // This represents a result that needs normalization
        denormalized.set(false, -2, 0x30000000ULL, false, false);  // 0.11 * 2^-2

        std::cout << "Normalized: scale=" << normalized.scale() << ", sig=0x"
                  << std::hex << normalized.significand() << std::dec << std::endl;
        std::cout << "Denormalized: scale=" << denormalized.scale() << ", sig=0x"
                  << std::hex << denormalized.significand() << std::dec << std::endl;

        // The blocktriple can represent and work with denormalized forms
        // The final conversion to the target format handles normalization
        std::cout << "BlockTriple can handle denormalized intermediate results\n";
        std::cout << "Target format conversion will normalize as needed\n";
        std::cout << std::endl;
    }

    // Example 7: Rounding during format conversion
    {
        std::cout << "Example 7: Rounding During Format Conversion\n";
        std::cout << "--------------------------------------------\n";

        // High precision blocktriple
        blocktriple<52, uint32_t> high_precision;

        // Set a value that will need rounding when converted to lower precision
        high_precision.set(false, 0, 0x123456789ABCDEF0ULL, false, false);

        std::cout << "High precision: sig=0x" << std::hex << high_precision.significand() << std::dec << std::endl;

        // Simulate conversion to lower precision (extract upper bits)
        uint64_t sig_bits = high_precision.significand();
        uint32_t upper_bits = static_cast<uint32_t>(sig_bits >> 32);
        uint32_t lower_bits = static_cast<uint32_t>(sig_bits);

        blocktriple<23, uint32_t> lower_precision;
        lower_precision.set(high_precision.sign(), high_precision.scale(),
                           static_cast<uint64_t>(upper_bits) << 32, false, false);

        std::cout << "Converted: sig=0x" << std::hex << lower_precision.significand() << std::dec << std::endl;
        std::cout << "Lost bits: 0x" << std::hex << lower_bits << std::dec << std::endl;

        // Check if rounding is needed
        bool should_round = (lower_bits & 0x80000000U) != 0;
        std::cout << "Rounding needed: " << (should_round ? "YES" : "NO") << std::endl;
        std::cout << std::endl;
    }

    // Example 8: Comparison operations
    {
        std::cout << "Example 8: Comparison Operations\n";
        std::cout << "--------------------------------\n";

        blocktriple<23, uint32_t> a, b, c;

        // Create three values: 2.0, 2.0, 3.0
        a.set(false, 1, 0x80000000ULL, false, false);  // 2.0
        b.set(false, 1, 0x80000000ULL, false, false);  // 2.0
        c.set(false, 1, 0xC0000000ULL, false, false);  // 3.0

        std::cout << "a = 2.0, b = 2.0, c = 3.0\n";
        std::cout << "a == b: " << (a == b) << std::endl;
        std::cout << "a != c: " << (a != c) << std::endl;
        std::cout << "a < c:  " << (a < c) << std::endl;
        std::cout << "c > a:  " << (c > a) << std::endl;
        std::cout << "a <= b: " << (a <= b) << std::endl;
        std::cout << "b >= a: " << (b >= a) << std::endl;
        std::cout << std::endl;
    }

    // Example 9: Square root operation
    {
        std::cout << "Example 9: Square Root Operation\n";
        std::cout << "--------------------------------\n";

        blocktriple<23, uint32_t> value, sqrt_result;

        // Square root of 4.0 should be 2.0
        value.set(false, 2, 0x80000000ULL, false, false);  // 4.0

        std::cout << "Value: " << value << " (4.0)\n";

        // Perform square root
        sqrt_result = sqrt(value);

        std::cout << "Square root: scale=" << sqrt_result.scale() << ", sig=0x"
                  << std::hex << sqrt_result.significand() << std::dec << " (should be 2.0)\n";
        std::cout << std::endl;
    }

    // Example 10: Error propagation and exception handling
    {
        std::cout << "Example 10: Error Propagation\n";
        std::cout << "-----------------------------\n";

        blocktriple<23, uint32_t> zero, normal, result;

        zero.setzero();
        normal.set(false, 0, 0x80000000ULL, false, false);  // 1.0

        std::cout << "Testing division by zero:\n";
        result = normal / zero;

        std::cout << "1.0 / 0.0 = " << result << std::endl;
        std::cout << "Result is infinite: " << result.isinf() << std::endl;
        std::cout << "Result is NaN: " << result.isnan() << std::endl;

        std::cout << "\nTesting invalid operations:\n";
        blocktriple<23, uint32_t> inf, nan_result;
        inf.setinf(false);
        nan_result = inf - inf;  // infinity - infinity = NaN

        std::cout << "inf - inf = " << nan_result << std::endl;
        std::cout << "Result is NaN: " << nan_result.isnan() << std::endl;
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