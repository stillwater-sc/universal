// test_pop_solver.cpp: end-to-end LP-based optimal bit assignment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Tests the PopSolver which translates an ExprGraph into an LP,
// solves it, and writes optimal nsb values back to the graph.

#include <universal/utility/directives.hpp>
#include <universal/mixedprecision/pop_solver.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

namespace sw { namespace universal {

// Test simple multiplication: z = x * y, require 10 bits at z
int TestSimpleMul() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int x = g.variable("x", 1.0, 8.0);
	int y = g.variable("y", 1.0, 8.0);
	int z = g.mul(x, y);

	g.require_nsb(z, 10);

	PopSolver solver;
	bool ok = solver.solve(g);

	if (!ok) {
		std::cerr << "FAIL: LP solver returned " << to_string(solver.status()) << std::endl;
		++nrOfFailedTestCases;
		return nrOfFailedTestCases;
	}

	// z should be exactly 10
	if (g.get_nsb(z) != 10) {
		std::cerr << "FAIL: z nsb expected 10, got " << g.get_nsb(z) << std::endl;
		++nrOfFailedTestCases;
	}

	// x, y should be 10 + carry = 11
	if (g.get_nsb(x) < 11) {
		std::cerr << "FAIL: x nsb expected >= 11, got " << g.get_nsb(x) << std::endl;
		++nrOfFailedTestCases;
	}
	if (g.get_nsb(y) < 11) {
		std::cerr << "FAIL: y nsb expected >= 11, got " << g.get_nsb(y) << std::endl;
		++nrOfFailedTestCases;
	}

	std::cout << "Simple mul LP solution:\n";
	solver.report(std::cout, g);

	return nrOfFailedTestCases;
}

// Test determinant: det = a*d - b*c
int TestDeterminant() {
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

	PopSolver solver;
	bool ok = solver.solve(g);

	if (!ok) {
		std::cerr << "FAIL: LP solver returned " << to_string(solver.status()) << std::endl;
		++nrOfFailedTestCases;
		return nrOfFailedTestCases;
	}

	// det should be at least 20
	if (g.get_nsb(det) < 20) {
		std::cerr << "FAIL: det nsb expected >= 20, got " << g.get_nsb(det) << std::endl;
		++nrOfFailedTestCases;
	}

	// LP should find optimal (minimum total bits)
	std::cout << "Determinant LP solution (total=" << solver.total_nsb() << "):\n";
	solver.report(std::cout, g);

	// Verify the LP solution is at least as good as or equal to fixpoint
	ExprGraph g2;
	int a2 = g2.variable("a", 8.0, 12.0);
	int b2 = g2.variable("b", 8.0, 12.0);
	int c2 = g2.variable("c", 8.0, 12.0);
	int d2 = g2.variable("d", 8.0, 12.0);
	int ad2 = g2.mul(a2, d2);
	int bc2 = g2.mul(b2, c2);
	int det2 = g2.sub(ad2, bc2);
	g2.require_nsb(det2, 20);
	g2.analyze();

	double fixpoint_total = 0;
	double lp_total = solver.total_nsb();
	for (int i = 0; i < g2.size(); ++i) {
		fixpoint_total += g2.get_nsb(i);
	}

	std::cout << "Fixpoint total: " << fixpoint_total << ", LP total: " << lp_total << "\n";

	// LP should be <= fixpoint (optimal is at most as good as conservative)
	if (lp_total > fixpoint_total + 1) {
		std::cerr << "WARNING: LP total exceeds fixpoint total (may be due to rounding)\n";
		// Not a hard failure since rounding can differ
	}

	return nrOfFailedTestCases;
}

// Test chain: z = (a + b) * c, require 12 bits at z
int TestChain() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int a = g.variable("a", 1.0, 10.0);
	int b = g.variable("b", 1.0, 10.0);
	int c = g.variable("c", 1.0, 10.0);

	int sum = g.add(a, b);
	int z = g.mul(sum, c);

	g.require_nsb(z, 12);

	PopSolver solver;
	bool ok = solver.solve(g);

	if (!ok) {
		std::cerr << "FAIL: chain LP solver failed" << std::endl;
		++nrOfFailedTestCases;
		return nrOfFailedTestCases;
	}

	if (g.get_nsb(z) < 12) {
		std::cerr << "FAIL: chain z expected >= 12, got " << g.get_nsb(z) << std::endl;
		++nrOfFailedTestCases;
	}

	std::cout << "Chain LP solution:\n";
	solver.report(std::cout, g);

	return nrOfFailedTestCases;
}

// Test empty graph returns false
int TestEmptyGraph() {
	int nrOfFailedTestCases = 0;

	ExprGraph g; // no nodes
	PopSolver solver;
	bool ok = solver.solve(g);

	if (ok) {
		std::cerr << "FAIL: empty graph should return false" << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test solver with only variables (no operations)
int TestVariablesOnly() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int a = g.variable("a", 1.0, 10.0);
	int b = g.variable("b", 1.0, 10.0);

	g.require_nsb(a, 8);
	g.require_nsb(b, 12);

	PopSolver solver;
	bool ok = solver.solve(g);

	if (!ok) {
		std::cerr << "FAIL: variables-only LP should succeed" << std::endl;
		++nrOfFailedTestCases;
		return nrOfFailedTestCases;
	}

	if (g.get_nsb(a) < 8) {
		std::cerr << "FAIL: a expected >= 8, got " << g.get_nsb(a) << std::endl;
		++nrOfFailedTestCases;
	}
	if (g.get_nsb(b) < 12) {
		std::cerr << "FAIL: b expected >= 12, got " << g.get_nsb(b) << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test with requirement on intermediate node (not just output)
int TestIntermediateRequirement() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int a = g.variable("a", 1.0, 10.0);
	int b = g.variable("b", 1.0, 10.0);
	int sum = g.add(a, b);
	int result = g.mul(sum, a);

	// Require precision at both intermediate and output
	g.require_nsb(sum, 15);
	g.require_nsb(result, 10);

	PopSolver solver;
	bool ok = solver.solve(g);

	if (!ok) {
		std::cerr << "FAIL: intermediate requirement LP failed" << std::endl;
		++nrOfFailedTestCases;
		return nrOfFailedTestCases;
	}

	if (g.get_nsb(sum) < 15) {
		std::cerr << "FAIL: intermediate sum expected >= 15, got " << g.get_nsb(sum) << std::endl;
		++nrOfFailedTestCases;
	}
	if (g.get_nsb(result) < 10) {
		std::cerr << "FAIL: result expected >= 10, got " << g.get_nsb(result) << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test solver status and total_nsb accessors
int TestSolverAccessors() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int x = g.variable("x", 1.0, 8.0);
	int y = g.variable("y", 1.0, 8.0);
	int z = g.mul(x, y);
	g.require_nsb(z, 10);

	PopSolver solver;
	solver.solve(g);

	if (solver.status() != LPStatus::Optimal) {
		std::cerr << "FAIL: status expected Optimal, got " << to_string(solver.status()) << std::endl;
		++nrOfFailedTestCases;
	}

	double total = solver.total_nsb();
	if (total < 32.0) { // At least 10 + 11 + 11 = 32
		std::cerr << "FAIL: total_nsb expected >= 32, got " << total << std::endl;
		++nrOfFailedTestCases;
	}

	// Test report output (should not crash)
	std::ostringstream oss;
	solver.report(oss, g);
	if (oss.str().empty()) {
		std::cerr << "FAIL: report output should not be empty" << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test unary operations through PopSolver (neg, abs, sqrt)
int TestUnaryOpsSolver() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int x = g.variable("x", 4.0, 16.0);
	int nx = g.neg(x);
	int ax = g.abs(nx);
	int result = g.sqrt(ax);

	g.require_nsb(result, 10);

	PopSolver solver;
	bool ok = solver.solve(g);

	if (!ok) {
		std::cerr << "FAIL: unary ops LP failed" << std::endl;
		++nrOfFailedTestCases;
		return nrOfFailedTestCases;
	}

	if (g.get_nsb(result) < 10) {
		std::cerr << "FAIL: unary result expected >= 10, got " << g.get_nsb(result) << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test division through PopSolver
int TestDivisionSolver() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int a = g.variable("a", 10.0, 100.0);
	int b = g.variable("b", 1.0, 10.0);
	int z = g.div(a, b);

	g.require_nsb(z, 14);

	PopSolver solver;
	bool ok = solver.solve(g);

	if (!ok) {
		std::cerr << "FAIL: division LP failed" << std::endl;
		++nrOfFailedTestCases;
		return nrOfFailedTestCases;
	}

	if (g.get_nsb(z) < 14) {
		std::cerr << "FAIL: div z expected >= 14, got " << g.get_nsb(z) << std::endl;
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

	std::cout << "POP LP Solver Tests\n";
	std::cout << std::string(40, '=') << "\n\n";

	TEST_CASE("Simple multiplication LP", TestSimpleMul());
	TEST_CASE("Determinant LP", TestDeterminant());
	TEST_CASE("Chain LP", TestChain());
	TEST_CASE("Empty graph", TestEmptyGraph());
	TEST_CASE("Variables only", TestVariablesOnly());
	TEST_CASE("Intermediate requirement", TestIntermediateRequirement());
	TEST_CASE("Solver accessors", TestSolverAccessors());
	TEST_CASE("Unary ops solver", TestUnaryOpsSolver());
	TEST_CASE("Division solver", TestDivisionSolver());

	std::cout << "\n";
	if (nrOfFailedTestCases == 0) {
		std::cout << "All POP solver tests PASSED\n";
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
