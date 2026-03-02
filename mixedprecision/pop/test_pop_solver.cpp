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

#define VERIFY(cond, msg) do { if (!(cond)) { std::cerr << "FAIL: " << msg << std::endl; ++nrOfFailedTestCases; } } while(0)

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
	VERIFY(ok, "LP solver returned " << to_string(solver.status()));
	if (!ok) return nrOfFailedTestCases;
	VERIFY(g.get_nsb(z) == 10, "z nsb expected 10, got " << g.get_nsb(z));
	VERIFY(g.get_nsb(x) >= 11, "x nsb expected >= 11, got " << g.get_nsb(x));
	VERIFY(g.get_nsb(y) >= 11, "y nsb expected >= 11, got " << g.get_nsb(y));
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
	VERIFY(ok, "LP solver returned " << to_string(solver.status()));
	if (!ok) return nrOfFailedTestCases;
	VERIFY(g.get_nsb(det) >= 20, "det nsb expected >= 20, got " << g.get_nsb(det));
	std::cout << "Determinant LP solution (total=" << solver.total_nsb() << "):\n";
	solver.report(std::cout, g);

	// Compare LP vs fixpoint
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
	for (int i = 0; i < g2.size(); ++i) fixpoint_total += g2.get_nsb(i);
	std::cout << "Fixpoint total: " << fixpoint_total << ", LP total: " << solver.total_nsb() << "\n";
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
	VERIFY(ok, "chain LP solver failed");
	if (!ok) return nrOfFailedTestCases;
	VERIFY(g.get_nsb(z) >= 12, "chain z expected >= 12, got " << g.get_nsb(z));
	std::cout << "Chain LP solution:\n";
	solver.report(std::cout, g);
	return nrOfFailedTestCases;
}

// Test empty graph returns false
int TestEmptyGraph() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	PopSolver solver;
	VERIFY(!solver.solve(g), "empty graph should return false");
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
	VERIFY(ok, "variables-only LP should succeed");
	if (!ok) return nrOfFailedTestCases;
	VERIFY(g.get_nsb(a) >= 8, "a expected >= 8, got " << g.get_nsb(a));
	VERIFY(g.get_nsb(b) >= 12, "b expected >= 12, got " << g.get_nsb(b));
	return nrOfFailedTestCases;
}

// Test with requirement on intermediate node
int TestIntermediateRequirement() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int a = g.variable("a", 1.0, 10.0);
	int b = g.variable("b", 1.0, 10.0);
	int sum = g.add(a, b);
	int result = g.mul(sum, a);
	g.require_nsb(sum, 15);
	g.require_nsb(result, 10);
	PopSolver solver;
	bool ok = solver.solve(g);
	VERIFY(ok, "intermediate requirement LP failed");
	if (!ok) return nrOfFailedTestCases;
	VERIFY(g.get_nsb(sum) >= 15, "intermediate sum expected >= 15, got " << g.get_nsb(sum));
	VERIFY(g.get_nsb(result) >= 10, "result expected >= 10, got " << g.get_nsb(result));
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
	VERIFY(solver.status() == LPStatus::Optimal, "status expected Optimal, got " << to_string(solver.status()));
	VERIFY(solver.total_nsb() >= 32.0, "total_nsb expected >= 32, got " << solver.total_nsb());
	std::ostringstream oss;
	solver.report(oss, g);
	VERIFY(!oss.str().empty(), "report output should not be empty");
	return nrOfFailedTestCases;
}

// Test unary operations through PopSolver
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
	VERIFY(ok, "unary ops LP failed");
	if (!ok) return nrOfFailedTestCases;
	VERIFY(g.get_nsb(result) >= 10, "unary result expected >= 10, got " << g.get_nsb(result));
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
	VERIFY(ok, "division LP failed");
	if (!ok) return nrOfFailedTestCases;
	VERIFY(g.get_nsb(z) >= 14, "div z expected >= 14, got " << g.get_nsb(z));
	return nrOfFailedTestCases;
}

}} // namespace sw::universal

#define TEST_CASE(name, func) do { int f_ = func; if (f_) { std::cout << name << ": FAIL (" << f_ << " errors)\n"; nrOfFailedTestCases += f_; } else { std::cout << name << ": PASS\n"; } } while(0)

int main()
try {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;
	std::cout << "POP LP Solver Tests\n" << std::string(40, '=') << "\n\n";

	TEST_CASE("Simple multiplication LP", TestSimpleMul());
	TEST_CASE("Determinant LP", TestDeterminant());
	TEST_CASE("Chain LP", TestChain());
	TEST_CASE("Empty graph", TestEmptyGraph());
	TEST_CASE("Variables only", TestVariablesOnly());
	TEST_CASE("Intermediate requirement", TestIntermediateRequirement());
	TEST_CASE("Solver accessors", TestSolverAccessors());
	TEST_CASE("Unary ops solver", TestUnaryOpsSolver());
	TEST_CASE("Division solver", TestDivisionSolver());

	std::cout << "\n" << (nrOfFailedTestCases == 0 ? "All POP solver tests PASSED" : std::to_string(nrOfFailedTestCases) + " test(s) FAILED") << "\n";
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const char* msg) { std::cerr << "Caught exception: " << msg << std::endl; return EXIT_FAILURE; }
catch (...) { std::cerr << "Caught unknown exception" << std::endl; return EXIT_FAILURE; }
