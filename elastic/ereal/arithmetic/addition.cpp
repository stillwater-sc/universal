// addition.cpp: Test ereal addition using expansion operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	/*
	The Verification Approach: targeted unit tests and aggressive property-based testing (fuzzing).
	
	Property-Based Testing:
	  - Write a fuzzer that generates random, valid Priest expansions of varying lengths, signs, and exponent ranges. 
	    For millions of iterations, feed A + B into your algorithm and assert both the Mathematical and Structural oracles. 
	
	Verify algebraic invariants:
	 - Commutativity: $A + B$ must yield the exact same expansion as $B + A$.
	 - Identity: $A + 0$ must yield exactly $A$.
	 - Inverses: $A + (-A)$ must perfectly collapse to a single $0.0$ component.
	 
	Isolating the Corner Cases:
	Random fuzzing is great, but floating-point errors hide in highly specific bit-patterns. 
	Manually construct unit tests for the following hostile scenarios:
	
	Catastrophic Cancellation: 
	  This is the ultimate stress test for Priest's renormalization step. 
	  - Add an expansion to its exact negation, plus a tiny epsilon.Test: $A + (-A + \epsilon)$.
	  - Verification: The algorithm must correctly cancel out the massive leading terms and promote 
		the tiny $\epsilon$ to the $z_1$ position without leaving leading zeros in the array.
		
	Round-to-Even Boundary Ties: 
	  Because Priest's strict condition relies on round-to-nearest, ties-to-even arithmetic, 
	  test additions that land exactly halfway between representable floating-point numbers. 
	  Ensure that the underlying 2Sum or Fast2Sum Error-Free Transformations push the error 
	  to the correct sign based on the even-bit rounding.
	
	Massive Exponent Gaps: * Test: Add $10^{300}$ and $10^{-300}$.
	  - Verification: Ensure merging and sorting logic handles non-overlapping numbers at 
	    opposite ends of the exponent range without attempting to fill the gap with millions 
		of zero components or dropping the tiny number.
		
	Complete Overlap: 
	  - Add two identical expansions: $A + A$. This tests the carrying logic, as every single 
	    component will overlap and generate an error term that must ripple up the expansion.
	
	Subnormal Boundaries: 
	  - As floating-point numbers cross into the subnormal (denormal) range, the ULP becomes a 
	    constant distance. This frequently breaks normalization algorithms that assume the ULP 
	    shrinks with the magnitude of the number. Test additions where the error term dips 
	    into subnormals.
	
	Special Constants: 
	  - Ensure the standard IEEE 754 propagation rules apply if you pass NaN, +Infinity, or -Infinity as components.
	*/

	// Test basic addition
	int VerifyErealAddition(bool reportTestCases) {
		using namespace sw::universal;
	    int nrOfFailedTestCases = 0;

		// Catastrophic Cancellation
	    if (reportTestCases) std::cout << "Testing catastrophic cancellation...\n";
		{
			ereal<16> a(1.0e100);
			ereal<16> tiny(1.0e-100);
			ereal<16> c = a + tiny + a;
		    ereal<16> d = a + a;
		
			// The expected result should be exactly 1.0e-15, as the large term should cancel out
		    if (tiny != c - d) {
			    if (reportTestCases)
				    std::cout << " FAIL " << tiny << " != " << c - d << '\n';
				++nrOfFailedTestCases;
			}

		}
		
		// Commutativity
	    if (reportTestCases) std::cout << "Testing commutativity...\n";
	    {
		    ereal<16> a(1.0e+15);
		    ereal<16> b(1.0);
		    ereal<16> c(1.0e-15);

		    ereal<16> result1 = a + b + c;
		    ereal<16> result2 = c + b + a;

			if (result1 != result2) {
			    if (reportTestCases) std::cout << " FAIL " << result1 << " != " << result2 << '\n';
			    std::cout << "result1 components: " << to_components(result1) << '\n';
			    std::cout << "result2 components: " << to_components(result2) << '\n';
			    ++nrOfFailedTestCases;
		    }
	    }

		// Associativity
	    if (reportTestCases) std::cout << "Testing associativity...\n";
		{
		    ereal<16> a(1.0e+30);
		    ereal<16> b(1.0);
		    ereal<16> c(1.0e-30);

			ereal<16> result1 = (a + b) + c;
			ereal<16> result2 = a + (b + c);
		    
			if (result1 != result2) {
			    if (reportTestCases) std::cout << " FAIL " << result1 << " != " << result2 << '\n';
			    ++nrOfFailedTestCases;
		    }
		}

		// Test 4: Identity
	    if (reportTestCases) std::cout << "Testing identity...\n";
		{
			ereal<16> a(1.0);
			ereal<16> zero(0.0);

			a += 1.0e-15;
		    a += 1.0e-30;
		    a += 1.0e-45;
			ereal<16> c = a + zero;

			if (c != a) {
			    if (reportTestCases) std::cout << " FAIL " << c << " != " << a << '\n';
				++nrOfFailedTestCases;
			}
		}

		return nrOfFailedTestCases;
	}

}  // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "ereal Arithmetic Tests (with expansion_ops)";
	std::string test_tag            = "addition";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(test_basic_addition(), "ereal", "addition");
	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors
#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyErealAddition(reportTestCases), "ereal", "addition");

#	endif

#	if REGRESSION_LEVEL_2

#	endif

#	if REGRESSION_LEVEL_3

#	endif

#	if REGRESSION_LEVEL_4

#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
