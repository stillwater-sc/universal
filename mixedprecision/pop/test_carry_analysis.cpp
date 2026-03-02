// test_carry_analysis.cpp: validate carry-bit refinement via policy iteration
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

#include <universal/utility/directives.hpp>
#include <universal/mixedprecision/carry_analysis.hpp>
#include <iostream>
#include <string>
#include <cmath>

namespace sw { namespace universal {

// Test that carry analysis converges
int TestConvergence() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int x = g.variable("x", 1.0, 8.0);
	int y = g.variable("y", 1.0, 8.0);
	int z = g.mul(x, y);

	g.require_nsb(z, 10);

	CarryAnalyzer ca;
	int iters = ca.refine(g);

	std::cout << "Simple mul converged in " << iters << " iterations\n";
	ca.report(std::cout, g);

	// Should converge (not exceed max iterations)
	if (iters >= 10) {
		std::cerr << "FAIL: carry analysis did not converge" << std::endl;
		++nrOfFailedTestCases;
	}

	// z should still meet its requirement
	if (g.get_nsb(z) < 10) {
		std::cerr << "FAIL: z requirement not met after carry analysis" << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test that carry refinement can reduce total bits
int TestBitReduction() {
	int nrOfFailedTestCases = 0;

	// Build graph: det = a*d - b*c
	ExprGraph g;
	int a = g.variable("a", 8.0, 12.0);
	int b = g.variable("b", 8.0, 12.0);
	int c = g.variable("c", 8.0, 12.0);
	int d = g.variable("d", 8.0, 12.0);
	int ad = g.mul(a, d);
	int bc = g.mul(b, c);
	int det = g.sub(ad, bc);

	g.require_nsb(det, 20);

	// Solve with conservative carries (all 1)
	PopSolver conservative;
	{
		ExprGraph g2;
		int a2 = g2.variable("a", 8.0, 12.0);
		int b2 = g2.variable("b", 8.0, 12.0);
		int c2 = g2.variable("c", 8.0, 12.0);
		int d2 = g2.variable("d", 8.0, 12.0);
		int ad2 = g2.mul(a2, d2);
		int bc2 = g2.mul(b2, c2);
		int det2 = g2.sub(ad2, bc2);
		g2.require_nsb(det2, 20);
		conservative.solve(g2);
		std::cout << "Conservative total: " << conservative.total_nsb() << "\n";
	}

	// Solve with carry refinement
	CarryAnalyzer ca;
	ca.refine(g);

	double refined_total = 0;
	for (int i = 0; i < g.size(); ++i) {
		refined_total += g.get_nsb(i);
	}

	std::cout << "Refined total: " << refined_total << "\n";
	ca.report(std::cout, g);

	// Refined should be <= conservative
	if (refined_total > conservative.total_nsb() + 1) {
		std::cerr << "FAIL: refined total exceeds conservative" << std::endl;
		++nrOfFailedTestCases;
	}

	// Output should still meet requirement
	if (g.get_nsb(det) < 20) {
		std::cerr << "FAIL: det requirement not met after carry refinement" << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test chain with addition (where carry analysis is most effective)
int TestAdditionChain() {
	int nrOfFailedTestCases = 0;

	// z = (a + b) + c with values of very different magnitudes
	ExprGraph g;
	int a = g.variable("a", 1000.0, 2000.0); // ufp ~= 10
	int b = g.variable("b", 0.001, 0.002);    // ufp ~= -10
	int c = g.variable("c", 1000.0, 2000.0);  // ufp ~= 10

	int ab = g.add(a, b);
	int z = g.add(ab, c);

	g.require_nsb(z, 12);

	CarryAnalyzer ca;
	int iters = ca.refine(g);

	std::cout << "Addition chain converged in " << iters << " iterations\n";
	ca.report(std::cout, g);

	if (g.get_nsb(z) < 12) {
		std::cerr << "FAIL: addition chain z requirement not met" << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

}} // namespace sw::universal

#define TEST_CASE(name, func) \
	do { \
		int fails = func; \
		if (fails) { \
			std::cout << name << ": FAIL (" << fails << " errors)" << std::endl; \
			nrOfFailedTestCases += fails; \
		} else { \
			std::cout << name << ": PASS" << std::endl; \
		} \
	} while(0)

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::cout << "POP Carry Analysis Tests\n";
	std::cout << std::string(40, '=') << "\n\n";

	TEST_CASE("Convergence", TestConvergence());
	TEST_CASE("Bit reduction", TestBitReduction());
	TEST_CASE("Addition chain", TestAdditionChain());

	std::cout << "\n";
	if (nrOfFailedTestCases == 0) {
		std::cout << "All carry analysis tests PASSED\n";
	} else {
		std::cout << nrOfFailedTestCases << " test(s) FAILED\n";
	}

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const char* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
