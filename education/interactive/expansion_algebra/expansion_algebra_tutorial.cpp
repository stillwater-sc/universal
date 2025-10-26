// expansion_algebra_tutorial.cpp : Interactive educational tutorial on expansion algebra
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Tutorial Goals:
// - Understand why standard floating-point arithmetic loses precision
// - Learn error-free transformations (two_sum, fast_two_sum)
// - Grasp the concept of multi-component expansions
// - See why naive compression fails and how proper compression works
// - Appreciate the algorithms behind dd, td, and qd cascade types

#include <universal/native/manipulators.hpp>  // valueRepresentation, to_binary() for native types
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/internal/floatcascade/floatcascade.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>
#include <bitset>

using namespace sw::universal;

// ==================== UTILITY FUNCTIONS ====================

// Wait for user to press Enter
void waitForUser() {
    std::cout << "\n[Press Enter to continue...]";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Print a section header
void printHeader(const std::string& title) {
    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(80, '=') << "\n";
}

// Print a subsection header
void printSubHeader(const std::string& title) {
    std::cout << "\n" << std::string(80, '-') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(80, '-') << "\n";
}

// Print IEEE-754 double representation
void printIEEE754(double value) {
	valueRepresentations(value);

    //std::cout << "  Value:     " << std::scientific << std::setprecision(17) << value << '\n';
	//std::cout << "  Bits       : " << to_binary(value, true) << '\n';
	//std::cout << "             : " << color_print(value) << '\n';
	//std::cout << "  Sign       : " << sign(value) << '\n';
	//std::cout << "  Scale      : " << exponent(value) << '\n';
	//std::cout << "  Signficant : " << fraction(value) << '\n';
}

// Two-sum algorithm (Dekker 1971) - Error-free transformation
void two_sum_demo(double a, double b, double& sum, double& error) {
    sum = a + b;
    double b_virtual = sum - a;
    double a_virtual = sum - b_virtual;
    double b_roundoff = b - b_virtual;
    double a_roundoff = a - a_virtual;
    error = a_roundoff + b_roundoff;
}

// Fast two-sum (Knuth 1974) - For when |a| >= |b|
void fast_two_sum_demo(double a, double b, double& sum, double& error) {
    sum = a + b;
    error = b - (sum - a);
}

// ==================== LESSON 1: ROUNDING ERROR PROBLEM ====================

void lesson1_rounding_errors() {
    printHeader("LESSON 1: The Rounding Error Problem");

    std::cout << "\nWhy do we need expansion algebra?\n";
    std::cout << "Standard IEEE-754 floating-point arithmetic loses information due to rounding.\n";
    std::cout << "Let's see this in action...\n";

    waitForUser();

    printSubHeader("Example 1a: The Classic 0.1 + 0.2 != 0.3");

    double a = 0.1;
    double b = 0.2;
    double sum = a + b;
    double expected = 0.3;

    std::cout << std::setprecision(17);
    std::cout << "\na     = " << a << "\n";
    std::cout << "b     = " << b << "\n";
    std::cout << "a + b = " << sum << "\n";
    std::cout << "0.3   = " << expected << "\n";
    std::cout << "\nDifference: " << (sum - expected) << "\n";
    std::cout << "\nWhy? Because 0.1, 0.2, and 0.3 cannot be represented exactly in binary!\n";

    waitForUser();

    printSubHeader("Example 1b: Catastrophic Cancellation");

    double large = 1.0e16;
    double small = 1.0;

    std::cout << "\nLet's try: (1e16 + 1.0) - 1e16\n";
    std::cout << "\nIntuitively, this should equal 1.0, right?\n";

    double result = (large + small) - large;

    std::cout << "\nlarge           = " << large << "\n";
    std::cout << "small           = " << small << "\n";
    std::cout << "large + small   = " << (large + small) << "\n";
    std::cout << "result          = " << result << "\n";
    std::cout << "\nWe expected 1.0, but got " << result << "!\n";
    std::cout << "The small value was completely lost because the mantissa has only 53 bits.\n";

    waitForUser();

    printSubHeader("Example 1c: Error Accumulation in Summation");

    std::cout << "\nSum many small values vs one large addition:\n";

    double sum1 = 0.0;
    for (int i = 0; i < 10000000; ++i) {
        sum1 += 0.1;
    }
    double sum2 = 10000000.0 * 0.1;

    std::cout << "\nSum of 10,000,000 additions of 0.1: " << sum1 << "\n";
    std::cout << "Single multiplication 10,000,000 * 0.1: " << sum2 << "\n";
    std::cout << "Difference: " << (sum1 - sum2) << "\n";
    std::cout << "\nEach addition lost a tiny bit of precision, and it accumulated!\n";

    waitForUser();

    printSubHeader("Key Takeaway");
    std::cout << "\nStandard floating-point arithmetic LOSES information through rounding.\n";
    std::cout << "Question: Can we capture what's being lost?\n";
	std::cout << "Answer: YES!\n\n";
	std::cout << "IEEE-754 has the property that the error can also be faithfully represented in IEEE-754.\n";
	std::cout << "\nThis leads us to Error-Free Transformations (EFT)!\n";
}

// ==================== LESSON 2: ERROR-FREE TRANSFORMATIONS ====================

void lesson2_error_free_transformations() {
    printHeader("LESSON 2: Error-Free Transformations (EFT)");

    std::cout << "\nThe key insight: We can perform addition EXACTLY using TWO doubles!\n";
    std::cout << "The first double holds the sum, the second holds the rounding error.\n";
    std::cout << "\nThis is called an Error-Free Transformation (EFT).\n";

    waitForUser();

    printSubHeader("Dekker's two_sum Algorithm (1971)");

    std::cout << "\nFor any two doubles a and b:\n";
    std::cout << "  a + b = sum + error\n\n";
    std::cout << "This is computed as follows:\n";

	std::cout << "// Two-sum algorithm (Dekker 1971) - Error-free transformation\n";
	std::cout << "void two_sum(double a, double b, double& sum, double& error) {\n";
	std::cout << "	sum               = a + b;\n";
	std::cout << "	double b_virtual  = sum - a;\n";
	std::cout << "	double a_virtual  = sum - b_virtual;\n";
	std::cout << "	double b_roundoff = b - b_virtual;\n";
	std::cout << "	double a_roundoff = a - a_virtual;\n";
	std::cout << "	error             = a_roundoff + b_roundoff;\n";
	std::cout << "}\n";

    std::cout << "\nLet's revisit Example 1b with two_sum:\n";

    double a = 1.0e16;
    double b = 1.0;
    double sum, error;

    two_sum_demo(a, b, sum, error);

    std::cout << std::setprecision(17);
    std::cout << "\na     = " << a << "\n";
    std::cout << "b     = " << b << "\n";
    std::cout << "sum   = " << sum << " (what we got from floating-point addition)\n";
    std::cout << "error = " << error << " (the lost bits, RECOVERED!)\n";

    std::cout << "\nNotice: error = " << error << " = b!\n";
    std::cout << "The small value wasn't lost - it's in the error term!\n";

    waitForUser();

    printSubHeader("Visualizing Where the Error Comes From");

    std::cout << "\nLet's see the bits:\n\n";
    std::cout << "a (1e16):\n";
    printIEEE754(a);
    std::cout << "\nb (1.0):\n";
    printIEEE754(b);
    std::cout << "\nsum (a+b in floating-point):\n";
    printIEEE754(sum);
    std::cout << "\nerror (recovered bits):\n";
    printIEEE754(error);

    std::cout << "\nThe error term captures the bits that couldn't fit in the sum!\n";

    waitForUser();

    printSubHeader("Knuth's fast_two_sum (1974)");

    std::cout << "\nWhen we KNOW that |a| >= |b|, we can use a faster algorithm:\n";
    std::cout << "  sum = a + b\n";
    std::cout << "  error = b - (sum - a)\n";
    std::cout << "\nThis is computationally cheaper.\n";
	std::cout << "\nLet's demonstrate this in single precision:\n";
    float a2 = 1.0e16;
    float b2 = 1.0;
    float sum2, error2;
	sum2   = a2 + b2;
	error2 = b2 - (sum2 - a2);

    std::cout << "\nExample:\n";
	std::cout << "a     = " << to_binary(a2) << " : " << a2 << '\n';
	std::cout << "b     = " << to_binary(b2) << " : " << b2 << '\n';
	std::cout << "sum   = " << to_binary(sum2) << " : " << sum2 << '\n';
	std::cout << "error = " << to_binary(error2) << " : " << error2 << '\n';

    std::cout << "\nAnd now let's verify that sum + error = a + b exactly by doing the computation in double precision:\n";
    double sum_ext = static_cast<double>(a2) + static_cast<double>(b2);
	double sum_with_error = static_cast<double>(sum2) + static_cast<double>(error2);
    std::cout << "\nVerification: sum + error:\n";
	std::cout << "  sum + error = " << to_binary(sum_with_error) << " : " << sum_with_error << "\n";
	std::cout << "  a + b       = " << to_binary(sum_ext) << " : " << sum_ext << "\n";
    std::cout << "QED!\n";

    waitForUser();

    printSubHeader("Key Takeaway");
    std::cout << "\nWe can perform addition EXACTLY using the two_sum algorithm:\n";
    std::cout << "  a + b = sum + error (mathematically exact!)\n";
	std::cout << "We have captured the lost bits in the error term!\n";
	std::cout << "\nWe would like to leverage this property to build higher precision arithmetic.\n";
    std::cout << "\nThis is the foundation of expansion algebra!\n";
}

// ==================== LESSON 3: MULTI-COMPONENT EXPANSIONS ====================

void lesson3_expansions() {
    printHeader("LESSON 3: Multi-Component Expansions");

    std::cout << "\nAn expansion is an unevaluated sum of IEEE-754 doubles:\n";
    std::cout << "  x = e[0] + e[1] + e[2] + ... + e[n-1]\n";
    std::cout << "\nKey properties:\n";
    std::cout << "  1. Non-overlapping: e[i] + e[i+1] produces no rounding error\n";
    std::cout << "  2. Decreasing magnitude: |e[i]| > |e[i+1]|\n";
    std::cout << "  3. Precision gain: Each component adds ~53 bits of precision\n";

    waitForUser();

    printSubHeader("Double-Double (dd): 2 Components = 106 Bits");

    std::cout << "\nLet's create a double-double value:\n";

    dd_cascade dd1(1.5, 1.5e-17);

    std::cout << std::setprecision(17);
    std::cout << "\ndd[0] = " << dd1[0] << " (most significant component)\n";
    std::cout << "dd[1] = " << dd1[1] << " (least significant component)\n";
    std::cout << "\nValue = dd[0] + dd[1] = " << (double(dd1[0]) + double(dd1[1])) << "\n";

    std::cout << "\nIMPORTANT: When we add dd[0] + dd[1] in floating-point, we lose precision!\n";
    std::cout << "But as an EXPANSION (unevaluated sum), the full 106 bits are preserved.\n";

    waitForUser();

    printSubHeader("Verifying the Non-Overlapping Property");

    std::cout << "\nNon-overlapping means: e[i] and e[i+1] occupy different bit positions.\n";
    std::cout << "Let's check our dd value:\n\n";

    std::cout << "dd[0]:\n";
    printIEEE754(dd1[0]);
    std::cout << "\ndd[1]:\n";
    printIEEE754(dd1[1]);

    double sum_test, error_test;
    fast_two_sum_demo(dd1[0], dd1[1], sum_test, error_test);

    std::cout << "\nTesting non-overlapping property with fast_two_sum:\n";
    std::cout << "sum   = " << sum_test << "\n";
    std::cout << "error = " << error_test << "\n";

    double ulpd = ulp(sum_test);
	std::cout << "\nThe ULP of the sum is: " << ulpd << "\n";
	std::cout << "If |error| < ULP, then the components are non-overlapping.\n";
    if (std::abs(error_test) < ulpd) {
        std::cout << "\n VERIFIED: No rounding error! The components are non-overlapping.\n";
    } else {
        std::cout << "\n Components overlap (error = " << error_test << ")\n";
    }

    waitForUser();

    printSubHeader("Higher Precision: Triple-Double and Quad-Double");

    std::cout << "\nWe can extend this to more components:\n\n";
    std::cout << "  Double-Double (dd):  2 components = 106 bits (2 x 53)\n";
    std::cout << "  Triple-Double (td):  3 components = 159 bits (3 x 53)\n";
    std::cout << "  Quad-Double (qd):    4 components = 212 bits (4 x 53)\n";

    std::cout << "\nCompare to standard types:\n";
    std::cout << "  float:               24 bits\n";
    std::cout << "  double:              53 bits\n";
    std::cout << "  long double (x86):   64 bits\n";

    std::cout << "\nQuad-double gives us 4x the precision of IEEE-754 double!\n";

    waitForUser();

    printSubHeader("Key Takeaway");
    std::cout << "\nMultiple components = Arbitrary precision using standard IEEE-754 hardware!\n";
    std::cout << "The expansion x = e[0] + e[1] + ... preserves ALL significant bits.\n";
    std::cout << "\nNext question: How do we ADD two expansions?\n";
}

// ==================== LESSON 4: EXPANSION ADDITION ====================

void lesson4_expansion_addition() {
    printHeader("LESSON 4: Expansion Addition");

    std::cout << "\nWhen we add two expansions, the number of components will expand.\n";
    std::cout << "Example: 2-component + 2-component = 4-component expansion\n";
	std::cout << "Example: 3-component + 3-component = 6-component expansion\n";
	std::cout << "Example: 4-component + 4-component = 8-component expansion\n";

    waitForUser();

    printSubHeader("Adding Two Double-Doubles");

    dd_cascade a(1.5, 1.5e-17);
    dd_cascade b(0.5, 5.0e-18);

    std::cout << std::setprecision(17);
    std::cout << "\na = [" << a[0] << ", " << a[1] << "]\n";
    std::cout << "b = [" << b[0] << ", " << b[1] << "]\n";

    std::cout << "\nComponent-wise addition using two_sum:\n";
    std::cout << "  Step 1: Add a[0] + b[0] -> produces sum and error\n";
    std::cout << "  Step 2: Add a[1] + b[1] -> produces sum and error\n";
    std::cout << "  Step 3: Combine and sort by magnitude\n";
    std::cout << "  Result: 4 components\n";

    // Demonstrate manual expansion addition
    double s0, e0, s1, e1;
    two_sum_demo(a[0], b[0], s0, e0);
    two_sum_demo(a[1], b[1], s1, e1);

    std::cout << "\nAfter component-wise two_sum:\n";
    std::cout << "  From a[0] + b[0]: sum = " << s0 << ", error = " << e0 << "\n";
    std::cout << "  From a[1] + b[1]: sum = " << s1 << ", error = " << e1 << "\n";

    std::cout << "\nThese 4 values form a 4-component expansion!\n";
    std::cout << "(After sorting and renormalization)\n";

    waitForUser();

    printSubHeader("The Growth Problem");

    std::cout << "\nNotice the pattern:\n";
    std::cout << "  dd + dd = 4-component expansion\n";
    std::cout << "  td + td = 6-component expansion\n";
    std::cout << "  qd + qd = 8-component expansion\n";

    std::cout << "\nAfter multiple operations, we'd have hundreds of components!\n";
    std::cout << "This is impractical for computation.\n";

    std::cout << "\nSolution: COMPRESSION\n";
    std::cout << "We need to compress the result back to the original size.\n";
    std::cout << "  - 4 components -> 2 components (for dd)\n";
    std::cout << "  - 6 components -> 3 components (for td)\n";
    std::cout << "  - 8 components -> 4 components (for qd)\n";

    waitForUser();

    printSubHeader("Key Takeaway");
    std::cout << "\nExpansion addition is exact but causes growth in component count.\n";
    std::cout << "We need compression to maintain a fixed size.\n";
    std::cout << "\nNext question: How do we compress WITHOUT losing precision?\n";
}

// ==================== LESSON 5: NAIVE COMPRESSION TRAP ====================

void lesson5_naive_compression() {
    printHeader("LESSON 5: The Naive Compression Trap");

    std::cout << "\nIntuitively, you might compress like this:\n";
    std::cout << "  compressed[0] = result[0]\n";
    std::cout << "  compressed[1] = result[1] + result[2] + result[3]\n";

    std::cout << "\nThis \"naive compression\" will lose precision.\n";

    waitForUser();

    printSubHeader("Why Naive Compression Fails");

    std::cout << "\nProblem: result[1] + result[2] + result[3] uses floating-point addition.\n";
    std::cout << "Each '+' operation introduces rounding errors!\n";
    std::cout << "We worked hard to capture those error bits, and now we're throwing them away.\n";

    std::cout << "\nLet's see this failure with a concrete example:\n";

    waitForUser();

    printSubHeader("The Identity Test: (a+b)-a = b");

    std::cout << "\nThis should ALWAYS be true, right?\n";
    std::cout << "Let's test it with naive compression:\n";

    // Simulate naive compression behavior (for demonstration)
    dd_cascade a(1.5, 1.5e-17);
    dd_cascade b(0.5, 5.0e-18);

    std::cout << std::setprecision(17);
    std::cout << "\na = [" << a[0] << ", " << a[1] << "]\n";
    std::cout << "b = [" << b[0] << ", " << b[1] << "]\n";

    // This uses PROPER compression
    dd_cascade sum = a + b;
    dd_cascade recovered_b = sum - a;

    std::cout << "\nUsing PROPER compression:\n";
    std::cout << "sum = a + b           = [" << sum[0] << ", " << sum[1] << "]\n";
    std::cout << "recovered_b = sum - a = [" << recovered_b[0] << ", " << recovered_b[1] << "]\n";
    std::cout << "original b            = [" << b[0] << ", " << b[1] << "]\n";

    double diff0 = std::abs(recovered_b[0] - b[0]);
    double diff1 = std::abs(recovered_b[1] - b[1]);

    std::cout << "\nDifference in [0]: " << diff0 << "\n";
    std::cout << "Difference in [1]: " << diff1 << "\n";

    if (diff0 < 1e-15 && diff1 < 1e-25) {
        std::cout << "\n SUCCESS: Identity holds with proper compression!\n";
    }

    std::cout << "\nWith NAIVE compression:\n";
    std::cout << "The identity test FAILED with errors like:\n";
    std::cout << "  Expected: 5.0e-18\n";
    std::cout << "  Got:      -1.5e-51 (WRONG SIGN AND MAGNITUDE!)\n";

    //waitForUser();

    //printSubHeader("The Bug We Fixed");

    //std::cout << "\nThe original qd_cascade code had:\n";
    //std::cout << "  compressed[3] = result[3] + result[4] + result[5] + result[6] + result[7];\n";

    //std::cout << "\nThis naive sum:\n";
    //std::cout << "  - Lost cumulative rounding errors across 4 additions\n";
    //std::cout << "  - Destroyed the 212-bit precision we worked to build\n";
    //std::cout << "  - Failed the identity test spectacularly\n";

    //std::cout << "\nThe fix: Use the proper two-phase compression algorithm!\n";

    waitForUser();

    printSubHeader("Key Takeaway");
    std::cout << "\nNaive compression (floating-point sum) DESTROYS precision!\n";
    std::cout << "We need an algorithm that preserves the error terms.\n";
    std::cout << "\nNext: The proper compression algorithm from the QD library.\n";
}

// ==================== LESSON 6: PROPER COMPRESSION ====================

void lesson6_proper_compression() {
    printHeader("LESSON 6: Proper Compression Algorithm");

    std::cout << "\nThe Hida-Li-Bailey QD library uses a sophisticated two-phase algorithm:\n";
    std::cout << "  Phase 1: Bottom-up accumulation using fast_two_sum\n";
    std::cout << "  Phase 2: Conditional extraction of non-overlapping components\n";

    waitForUser();

    printSubHeader("Phase 1: Bottom-Up Accumulation");

    std::cout << "\nIdea: Accumulate from least significant to most significant.\n";
    std::cout << "Each step uses fast_two_sum to capture errors.\n";

    std::cout << "\nExample: Compress 4 components to 2\n";
    std::cout << "  Input: [r0, r1, r2, r3] (4 components)\n";
    std::cout << "\n  Step 1: fast_two_sum(r2, r3) -> updates r2, pushes error to r3\n";
    std::cout << "  Step 2: fast_two_sum(r1, r2) -> updates r1, pushes error to r2\n";
    std::cout << "  Step 3: fast_two_sum(r0, r1) -> updates r0, pushes error to r1\n";

    std::cout << "\nAfter Phase 1, we have a renormalized expansion.\n";
    std::cout << "All errors have been pushed into the representation.\n";

    waitForUser();

    printSubHeader("Phase 2: Conditional Extraction");

    std::cout << "\nIdea: Extract exactly N non-overlapping components.\n";
    std::cout << "Use conditional logic to handle zeros and overlaps.\n";

    std::cout << "\nPseudocode for 4-2 compression:\n";
    std::cout << "  s0, s1 = fast_two_sum(r0, r1)\n";
    std::cout << "  if s1 != 0:\n";
    std::cout << "    s1, s2 = fast_two_sum(s1, r2)\n";
    std::cout << "    if s2 != 0:\n";
    std::cout << "      s2 += r3  // Fold remaining into s2\n";
    std::cout << "    else:\n";
    std::cout << "      s1 += r3  // Fold into s1 if s2 is zero\n";
    std::cout << "  else:\n";
    std::cout << "    s0, s1 = fast_two_sum(s0, r2)  // Skip zero s1\n";
    std::cout << "    ...\n";

    std::cout << "\nThe conditional logic ensures we extract meaningful components.\n";

    waitForUser();

    printSubHeader("Why This Works");

    std::cout << "\nKey insights:\n";
    std::cout << "  1. fast_two_sum NEVER loses information (error-free)\n";
    std::cout << "  2. Bottom-up accumulation gathers all errors\n";
    std::cout << "  3. Conditional extraction handles edge cases (zeros)\n";
    std::cout << "  4. Final fold (+=) adds any remaining tiny bits\n";

    std::cout << "\nThe algorithm guarantees:\n";
    std::cout << "  - All significant bits are preserved\n";
    std::cout << "  - Non-overlapping property is maintained\n";
    std::cout << "  - Result has exactly N components\n";

    waitForUser();

    printSubHeader("Implementation in Universal");

    std::cout << "\nYou can find this in floatcascade.hpp:\n";
    std::cout << "  - compress_4to2() for double-double\n";
    std::cout << "  - compress_6to3() for triple-double\n";
    std::cout << "  - compress_8to4() for quad-double\n";

    std::cout << "\nThese functions include extensive commentary explaining:\n";
    std::cout << "  - Why naive compression fails\n";
    std::cout << "  - How the two-phase algorithm works\n";
    std::cout << "  - Testing insights from the identity test\n";

    std::cout << "\nRecommended reading:\n";
    std::cout << "  include/sw/universal/internal/floatcascade/floatcascade.hpp\n";
    std::cout << "  Lines 469-700+ (compression functions with full commentary)\n";

    waitForUser();

    printSubHeader("Key Takeaway");
    std::cout << "\nProper compression uses error-free transformations throughout.\n";
    std::cout << "Two-phase algorithm: bottom-up accumulation + conditional extraction.\n";
    std::cout << "Result: Full precision is preserved!\n";
}

// ==================== LESSON 7: SCALING TO HIGHER PRECISION ====================

void lesson7_scaling() {
    printHeader("LESSON 7: Scaling to Higher Precision");

    std::cout << "\nThe same principles scale to arbitrary precision:\n";
    std::cout << "  Double-Double (dd): 2 components,  106 bits\n";
    std::cout << "  Triple-Double (td): 3 components,  159 bits\n";
    std::cout << "  Quad-Double   (qd): 4 components,  212 bits\n";

    std::cout << "\nPattern:\n";
    std::cout << "  N components -> N x 53 bits of precision (approximately)\n";

    waitForUser();

    printSubHeader("Compression Pattern Recognition");

    std::cout << "\nThe compression algorithms follow the same pattern:\n\n";
    std::cout << "compress_4to2 (dd + dd):\n";
    std::cout << "  Input:  4 components [r0, r1, r2, r3]\n";
    std::cout << "  Output: 2 components [s0, s1]\n";
    std::cout << "  Phase 1: 3 fast_two_sum operations (bottom-up)\n";
    std::cout << "  Phase 2: Conditional extraction of 2 components\n\n";

    std::cout << "compress_6to3 (td + td):\n";
    std::cout << "  Input:  6 components [r0, r1, r2, r3, r4, r5]\n";
    std::cout << "  Output: 3 components [s0, s1, s2]\n";
    std::cout << "  Phase 1: 5 fast_two_sum operations\n";
    std::cout << "  Phase 2: Conditional extraction of 3 components\n\n";

    std::cout << "compress_8to4 (qd + qd):\n";
    std::cout << "  Input:  8 components [r0, ..., r7]\n";
    std::cout << "  Output: 4 components [s0, s1, s2, s3]\n";
    std::cout << "  Phase 1: 7 fast_two_sum operations\n";
    std::cout << "  Phase 2: Conditional extraction of 4 components\n";

    waitForUser();

    printSubHeader("Precision Demonstration");

    std::cout << "\nLet's compute pi using different precisions:\n";

    // Use Machin's formula: π/4 = 4*arctan(1/5) - arctan(1/239)
    // (We'd need actual implementations of arctan, so we'll show the concept)

    std::cout << std::setprecision(17);

    double pi_double = 3.141592653589793;
    std::cout << "\npi (double, 53 bits):     " << pi_double << "\n";

    std::cout << "\npi (dd, 106 bits):        " << "3.14159265358979323846264338327950288...\n";
    std::cout << "pi (td, 159 bits):        " << "3.14159265358979323846264338327950288419716939937510...\n";
    std::cout << "pi (qd, 212 bits):        " << "3.14159265358979323846264338327950288419716939937510582097494459230781...\n";

    std::cout << "\nEach additional component gives ~15-17 more decimal digits!\n";

    waitForUser();

    printSubHeader("Computational Cost vs Precision Tradeoff");

    std::cout << "\nMore components = higher precision BUT slower computation:\n\n";
    std::cout << "Operation costs (relative to double):\n";
    std::cout << "  double       :   1x (baseline)\n";
    std::cout << "  double-double:  ~6x slower\n";
    std::cout << "  triple-double: ~12x slower\n";
    std::cout << "  quad-double  : ~20x slower\n";

    std::cout << "\nWhen to use each:\n";
    std::cout << "  dd:  General-purpose extended precision\n";
    std::cout << "  td:  High-precision scientific computing\n";
    std::cout << "  qd:  Extreme precision requirements (cryptography, etc.)\n";

    waitForUser();

    printSubHeader("Key Takeaway");
    std::cout << "\nThe same algorithm pattern scales to arbitrary precision!\n";
    std::cout << "Choose precision level based on your accuracy requirements and budget.\n";
}

// ==================== LESSON 8: REAL-WORLD APPLICATIONS ====================

void lesson8_applications() {
    printHeader("LESSON 8: Real-World Applications");

    std::cout << "\nWhy does expansion algebra matter in practice?\n";
    std::cout << "Let's see some real-world applications...\n";

    waitForUser();

    printSubHeader("Application 1: Reproducible Linear Algebra");

    std::cout << "\nProblem: Floating-point operations are not associative:\n";
    std::cout << "  (a + b) + c != a + (b + c) in general\n";

    std::cout << "\nThis causes reproducibility issues:\n";
    std::cout << "  - Parallel reductions may give different results\n";
    std::cout << "  - Rerunning the same code may produce different answers\n";
    std::cout << "  - Hard to debug numerical algorithms\n";

    std::cout << "\nSolution: Quire accumulator (expansion-based)\n";
    std::cout << "  - Captures ALL rounding errors\n";
    std::cout << "  - Exact dot products\n";
    std::cout << "  - Reproducible results regardless of execution order\n";

    waitForUser();

    printSubHeader("Application 2: Ill-Conditioned Problems");

    std::cout << "\nIll-conditioned matrices lose precision in standard arithmetic.\n";
    std::cout << "\nExample: Hilbert matrix H[i,j] = 1/(i+j+1)\n";
    std::cout << "  - Condition number grows exponentially with size\n";
    std::cout << "  - Standard double precision fails for n > 10\n";
    std::cout << "  - Double-double extends this to n ~ 20\n";
    std::cout << "  - Quad-double handles n ~ 30\n";

    std::cout << "\nExpansion arithmetic provides enough precision to compute reliable solutions!\n";

    waitForUser();

    printSubHeader("Application 3: Accurate Polynomial Evaluation");

    std::cout << "\nEvaluating polynomials near roots requires high precision.\n";

    std::cout << "\nExample: p(x) = (x - 1)^10 near x = 1\n";
    std::cout << "  - Expanded form has large coefficients\n";
    std::cout << "  - Catastrophic cancellation occurs\n";
    std::cout << "  - Standard double precision loses all accuracy\n";

    std::cout << "\nWith quad-double:\n";
    std::cout << "  - Horner's method with qd arithmetic\n";
    std::cout << "  - Maintains accuracy even near roots\n";
    std::cout << "  - Enables reliable root finding\n";

    waitForUser();

    printSubHeader("Application 4: Iterative Refinement");

    std::cout << "\nIterative refinement improves solution accuracy:\n";
    std::cout << "  1. Solve Ax = b in standard precision\n";
    std::cout << "  2. Compute residual r = b - Ax in extended precision\n";
    std::cout << "  3. Solve correction equation: A·delta = r\n";
    std::cout << "  4. Update: x := x + delta\n";
    std::cout << "  5. Repeat until convergence\n";

    std::cout << "\nExpansion arithmetic in step 2 captures the full residual.\n";
    std::cout << "This enables convergence to full extended-precision accuracy!\n";

    waitForUser();

    printSubHeader("Application 5: Deep Learning Training");

    std::cout << "\nDeep neural networks accumulate gradients over millions of examples.\n";

    std::cout << "\nChallenges:\n";
    std::cout << "  - Tiny gradients can be lost in accumulation\n";
    std::cout << "  - Non-reproducible training across GPUs\n";
    std::cout << "  - Numerical instabilities in large models\n";

    std::cout << "\nExpansion algebra benefits:\n";
    std::cout << "  - Exact gradient accumulation\n";
    std::cout << "  - Reproducible training\n";
    std::cout << "  - Better convergence\n";

    std::cout << "\n(Note: Still research area due to performance costs)\n";

    waitForUser();

    printSubHeader("Key Takeaway");
    std::cout << "\nExpansion algebra enables reliable numerical computing!\n";
    std::cout << "\nApplications include:\n";
    std::cout << "  - Reproducible linear algebra\n";
    std::cout << "  - Ill-conditioned systems\n";
    std::cout << "  - Polynomial evaluation\n";
    std::cout << "  - Iterative refinement\n";
    std::cout << "  - Scientific computing requiring extended precision\n";

    std::cout << "\nThe performance cost is worth it when correctness matters!\n";
}

// ==================== MAIN MENU ====================

void showMenu() {
    std::cout << "\n\n";
    printHeader("EXPANSION ALGEBRA TUTORIAL");
    std::cout << "\nSelect a lesson:\n\n";
    std::cout << "  1. The Rounding Error Problem\n";
    std::cout << "  2. Error-Free Transformations (two_sum, fast_two_sum)\n";
    std::cout << "  3. Multi-Component Expansions\n";
    std::cout << "  4. Expansion Addition\n";
    std::cout << "  5. The Naive Compression Trap\n";
    std::cout << "  6. Proper Compression Algorithm\n";
    std::cout << "  7. Scaling to Higher Precision\n";
    std::cout << "  8. Real-World Applications\n";
    std::cout << "\n  9. Run all lessons sequentially\n";
    std::cout << "  0. Exit\n";
    std::cout << "\nChoice: ";
}

int main()
try {
    std::cout << std::setprecision(17) << std::scientific;

    std::cout << "\n";
    std::cout << "========================================================================\n";
    std::cout << "      UNDERSTANDING EXPANSION ALGEBRA\n";
    std::cout << "      Interactive Tutorial on Multi-Component Arithmetic\n";
    std::cout << "========================================================================\n";
    std::cout << "\nThis tutorial will teach you:\n";
    std::cout << "  - Why standard floating-point loses precision\n";
    std::cout << "  - How error-free transformations work\n";
    std::cout << "  - What expansion algebra is and why it matters\n";
    std::cout << "  - How compression algorithms preserve precision\n";
    std::cout << "  - Real-world applications of extended precision\n";
    std::cout << "\nLessons build progressively - start with Lesson 1 if you're new!\n";

    bool running = true;
    while (running) {
        showMenu();

        int choice;
        std::cin >> choice;

        // Clear the newline after the choice
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1:
                lesson1_rounding_errors();
                break;
            case 2:
                lesson2_error_free_transformations();
                break;
            case 3:
                lesson3_expansions();
                break;
            case 4:
                lesson4_expansion_addition();
                break;
            case 5:
                lesson5_naive_compression();
                break;
            case 6:
                lesson6_proper_compression();
                break;
            case 7:
                lesson7_scaling();
                break;
            case 8:
                lesson8_applications();
                break;
            case 9:
                lesson1_rounding_errors();
                lesson2_error_free_transformations();
                lesson3_expansions();
                lesson4_expansion_addition();
                lesson5_naive_compression();
                lesson6_proper_compression();
                lesson7_scaling();
                lesson8_applications();
                std::cout << "\n\n";
                printHeader("TUTORIAL COMPLETE!");
                std::cout << "\nCongratulations! You've completed all lessons.\n";
                std::cout << "You now understand the fundamentals of expansion algebra!\n";
                std::cout << "\nNext steps:\n";
                std::cout << "  - Explore the cascade type implementations in Universal\n";
                std::cout << "  - Read the compression functions in floatcascade.hpp\n";
                std::cout << "  - Try using dd, td, or qd in your own applications\n";
                std::cout << "  - Check out the test suites for more examples\n";
                break;
            case 0:
                running = false;
                std::cout << "\nThank you for learning about expansion algebra!\n";
                break;
            default:
                std::cout << "\nInvalid choice. Please select 0-9.\n";
        }
    }

    return EXIT_SUCCESS;
}
catch (char const* msg) {
    std::cerr << "Error: " << msg << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
    return EXIT_FAILURE;
}
