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

#define VERIFY(cond, msg) do { if (!(cond)) { std::cerr << "FAIL: " << msg << std::endl; ++nrOfFailedTestCases; } } while(0)

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
	VERIFY(iters < 10, "carry analysis did not converge");
	VERIFY(g.get_nsb(z) >= 10, "z requirement not met after carry analysis");
	return nrOfFailedTestCases;
}

// Test that carry refinement can reduce total bits
int TestBitReduction() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int a = g.variable("a", 8.0, 12.0);
	int b = g.variable("b", 8.0, 12.0);
	int c = g.variable("c", 8.0, 12.0);
	int d = g.variable("d", 8.0, 12.0);
	int ad = g.mul(a, d);
	int bc = g.mul(b, c);
	int det = g.sub(ad, bc);
	g.require_nsb(det, 20);

	PopSolver conservative;
	{ // Solve with conservative carries
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

	CarryAnalyzer ca;
	ca.refine(g);
	double refined_total = 0;
	for (int i = 0; i < g.size(); ++i) refined_total += g.get_nsb(i);
	std::cout << "Refined total: " << refined_total << "\n";
	ca.report(std::cout, g);
	VERIFY(refined_total <= conservative.total_nsb() + 1, "refined total exceeds conservative");
	VERIFY(g.get_nsb(det) >= 20, "det requirement not met after carry refinement");
	return nrOfFailedTestCases;
}

// Test chain with addition (carry analysis most effective here)
int TestAdditionChain() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int a = g.variable("a", 1000.0, 2000.0);
	int b = g.variable("b", 0.001, 0.002);
	int c = g.variable("c", 1000.0, 2000.0);
	int ab = g.add(a, b);
	int z = g.add(ab, c);
	g.require_nsb(z, 12);
	CarryAnalyzer ca;
	int iters = ca.refine(g);
	std::cout << "Addition chain converged in " << iters << " iterations\n";
	ca.report(std::cout, g);
	VERIFY(g.get_nsb(z) >= 12, "addition chain z requirement not met");
	return nrOfFailedTestCases;
}

// Test variables-only graph (no operations to refine)
int TestVariablesOnly() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int a = g.variable("a", 1.0, 10.0);
	int b = g.variable("b", 1.0, 10.0);
	g.require_nsb(a, 8);
	g.require_nsb(b, 12);
	CarryAnalyzer ca;
	int iters = ca.refine(g);
	std::cout << "Variables-only converged in " << iters << " iterations\n";
	VERIFY(g.get_nsb(a) >= 8, "a requirement not met");
	VERIFY(g.get_nsb(b) >= 12, "b requirement not met");
	return nrOfFailedTestCases;
}

// Test repeated refine() calls (idempotent)
int TestRepeatedRefine() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int x = g.variable("x", 1.0, 8.0);
	int y = g.variable("y", 1.0, 8.0);
	int z = g.mul(x, y);
	g.require_nsb(z, 10);
	CarryAnalyzer ca;
	ca.refine(g);
	int nsb_x1 = g.get_nsb(x), nsb_y1 = g.get_nsb(y), nsb_z1 = g.get_nsb(z);
	CarryAnalyzer ca2;
	ca2.refine(g);
	VERIFY(g.get_nsb(x) == nsb_x1 && g.get_nsb(y) == nsb_y1 && g.get_nsb(z) == nsb_z1, "repeated refine changed nsb values");
	return nrOfFailedTestCases;
}

// Test carry analysis with division (always carry=1)
int TestDivisionCarry() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int a = g.variable("a", 10.0, 100.0);
	int b = g.variable("b", 1.0, 10.0);
	int z = g.div(a, b);
	g.require_nsb(z, 14);
	CarryAnalyzer ca;
	int iters = ca.refine(g);
	std::cout << "Division carry converged in " << iters << " iterations\n";
	ca.report(std::cout, g);
	VERIFY(g.get_node(z).carry == 1, "division carry should remain 1, got " << g.get_node(z).carry);
	VERIFY(g.get_nsb(z) >= 14, "div z requirement not met");
	return nrOfFailedTestCases;
}

// Test carry analysis with sqrt
int TestSqrtCarry() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int x = g.variable("x", 4.0, 100.0);
	int z = g.sqrt(x);
	g.require_nsb(z, 12);
	CarryAnalyzer ca;
	int iters = ca.refine(g);
	std::cout << "Sqrt carry converged in " << iters << " iterations\n";
	VERIFY(g.get_nsb(z) >= 12, "sqrt z requirement not met");
	return nrOfFailedTestCases;
}

// Test iterations() accessor
int TestIterationsAccessor() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int x = g.variable("x", 1.0, 8.0);
	int y = g.variable("y", 1.0, 8.0);
	int z = g.mul(x, y);
	g.require_nsb(z, 10);
	CarryAnalyzer ca;
	int iters = ca.refine(g);
	VERIFY(ca.iterations() == iters, "iterations() != refine() return value");
	return nrOfFailedTestCases;
}

}} // namespace sw::universal

#define TEST_CASE(name, func) do { int f_ = func; if (f_) { std::cout << name << ": FAIL (" << f_ << " errors)\n"; nrOfFailedTestCases += f_; } else { std::cout << name << ": PASS\n"; } } while(0)

int main()
try {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;
	std::cout << "POP Carry Analysis Tests\n" << std::string(40, '=') << "\n\n";

	TEST_CASE("Convergence", TestConvergence());
	TEST_CASE("Bit reduction", TestBitReduction());
	TEST_CASE("Addition chain", TestAdditionChain());
	TEST_CASE("Variables only", TestVariablesOnly());
	TEST_CASE("Repeated refine", TestRepeatedRefine());
	TEST_CASE("Division carry", TestDivisionCarry());
	TEST_CASE("Sqrt carry", TestSqrtCarry());
	TEST_CASE("Iterations accessor", TestIterationsAccessor());

	std::cout << "\n" << (nrOfFailedTestCases == 0 ? "All carry analysis tests PASSED" : std::to_string(nrOfFailedTestCases) + " test(s) FAILED") << "\n";
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const char* msg) { std::cerr << "Caught exception: " << msg << std::endl; return EXIT_FAILURE; }
catch (...) { std::cerr << "Caught unknown exception" << std::endl; return EXIT_FAILURE; }
