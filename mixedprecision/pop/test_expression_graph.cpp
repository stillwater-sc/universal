// test_expression_graph.cpp: validate ExprGraph construction and analysis
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Tests the expression graph DAG builder, forward/backward analysis,
// and integration with TypeAdvisor for type recommendations.

#include <universal/utility/directives.hpp>
#include <universal/mixedprecision/expression_graph.hpp>
#include <iostream>
#include <string>
#include <cmath>

#define VERIFY(cond, msg) do { if (!(cond)) { std::cerr << "FAIL: " << msg << std::endl; ++nrOfFailedTestCases; } } while(0)

namespace sw { namespace universal {

// Test basic graph construction
int TestGraphConstruction() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int a = g.variable("a", 1.0, 10.0);
	int b = g.variable("b", 1.0, 10.0);
	int c = g.add(a, b);
	VERIFY(g.size() == 3, "expected 3 nodes, got " << g.size());
	auto& node_c = g.get_node(c);
	VERIFY(node_c.op == OpKind::Add, "expected Add op");
	VERIFY(node_c.lhs == a && node_c.rhs == b, "wrong input edges");
	return nrOfFailedTestCases;
}

// Test determinant: det = a*d - b*c with 20-bit requirement
int TestDeterminantAnalysis() {
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
	g.analyze();
	VERIFY(g.get_nsb(det) >= 20, "det nsb should be >= 20, got " << g.get_nsb(det));
	VERIFY(g.get_nsb(ad) >= g.get_nsb(det), "ad should need >= det bits due to cancellation");
	VERIFY(g.get_nsb(a) >= g.get_nsb(ad), "input a should need >= ad bits");
	std::cout << "Determinant example analysis:\n";
	g.report(std::cout);
	return nrOfFailedTestCases;
}

// Test simple multiplication chain: z = x * y, require 10 bits at z
int TestSimpleMulBackward() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int x = g.variable("x", 1.0, 8.0);
	int y = g.variable("y", 1.0, 8.0);
	int z = g.mul(x, y);
	g.require_nsb(z, 10);
	g.analyze();
	VERIFY(g.get_nsb(x) >= 11, "mul backward x expected >= 11, got " << g.get_nsb(x));
	VERIFY(g.get_nsb(y) >= 11, "mul backward y expected >= 11, got " << g.get_nsb(y));
	return nrOfFailedTestCases;
}

// Test with range_analyzer integration
int TestRangeAnalyzerIntegration() {
	int nrOfFailedTestCases = 0;
	range_analyzer<double> ra;
	ra.observe(0.5);
	ra.observe(100.0);
	ra.observe(50.0);
	ra.observe(75.0);
	ExprGraph g;
	int x = g.variable("x", ra);
	auto& node = g.get_node(x);
	VERIFY(node.lo == 0.5 && node.hi == 100.0, "range_analyzer bridge lo/hi mismatch");
	VERIFY(node.ufp == ra.ufp(), "ufp mismatch: node=" << node.ufp << ", analyzer=" << ra.ufp());
	return nrOfFailedTestCases;
}

// Test TypeAdvisor integration
int TestTypeRecommendation() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int x = g.variable("x", 0.1, 100.0);
	int y = g.variable("y", 0.1, 100.0);
	int z = g.mul(x, y);
	g.require_nsb(z, 10);
	g.analyze();
	TypeAdvisor advisor;
	std::string rec = g.recommended_type(z, advisor);
	std::cout << "Type recommendation for z (nsb=" << g.get_nsb(z) << "): " << rec << std::endl;
	VERIFY(!rec.empty(), "empty type recommendation");
	g.report(std::cout, advisor);
	return nrOfFailedTestCases;
}

// Test chain: y = sqrt(a*a + b*b)
int TestPythagoreanAnalysis() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int a = g.variable("a", 1.0, 10.0);
	int b = g.variable("b", 1.0, 10.0);
	int a2 = g.mul(a, a);
	int b2 = g.mul(b, b);
	int sum = g.add(a2, b2);
	int result = g.sqrt(sum);
	g.require_nsb(result, 16);
	g.analyze();
	std::cout << "Pythagorean analysis (require 16 bits at sqrt):\n";
	g.report(std::cout);
	VERIFY(g.get_nsb(result) >= 16, "pythagorean result should have >= 16 bits");
	return nrOfFailedTestCases;
}

// Test division operation
int TestDivisionOp() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int x = g.variable("x", 2.0, 10.0);
	int y = g.variable("y", 1.0, 4.0);
	int z = g.div(x, y);
	g.require_nsb(z, 12);
	g.analyze();
	VERIFY(g.get_node(z).op == OpKind::Div, "expected Div op");
	VERIFY(g.get_nsb(z) >= 12, "div z expected >= 12, got " << g.get_nsb(z));
	VERIFY(g.get_nsb(x) >= g.get_nsb(z), "div input x should need >= z bits");
	return nrOfFailedTestCases;
}

// Test unary operations: neg, abs
int TestUnaryOps() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int x = g.variable("x", 1.0, 10.0);
	int nx = g.neg(x);
	int ax = g.abs(x);
	VERIFY(g.get_node(nx).op == OpKind::Neg, "expected Neg op");
	VERIFY(g.get_node(ax).op == OpKind::Abs, "expected Abs op");
	auto& node_neg = g.get_node(nx);
	VERIFY(node_neg.lo == -10.0 && node_neg.hi == -1.0, "neg range mismatch: [" << node_neg.lo << ", " << node_neg.hi << "]");
	auto& node_abs = g.get_node(ax);
	VERIFY(node_abs.lo == 1.0 && node_abs.hi == 10.0, "abs range mismatch");
	return nrOfFailedTestCases;
}

// Test abs with range spanning zero
int TestAbsSpanningZero() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int x = g.variable("x", -5.0, 10.0);
	int ax = g.abs(x);
	auto& node = g.get_node(ax);
	VERIFY(node.lo == 0.0, "abs spanning zero lo expected 0, got " << node.lo);
	VERIFY(node.hi == 10.0, "abs spanning zero hi expected 10, got " << node.hi);
	return nrOfFailedTestCases;
}

// Test abs with negative range
int TestAbsNegativeRange() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int x = g.variable("x", -10.0, -2.0);
	int ax = g.abs(x);
	auto& node = g.get_node(ax);
	VERIFY(node.lo == 2.0 && node.hi == 10.0, "abs negative range: [" << node.lo << ", " << node.hi << "], expected [2, 10]");
	return nrOfFailedTestCases;
}

// Test multi-consumer node
int TestMultiConsumer() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int x = g.variable("x", 1.0, 10.0);
	g.mul(x, x);    // consumer 1: x*x needs 17 bits
	g.add(x, x);    // consumer 2: x+x needs less
	g.require_nsb(0 + 1, 16); // y1 = mul node (id=1)
	g.require_nsb(0 + 2, 8);  // y2 = add node (id=2)
	g.analyze();
	VERIFY(g.get_nsb(x) >= 17, "multi-consumer x expected >= 17 (from mul), got " << g.get_nsb(x));
	return nrOfFailedTestCases;
}

// Test constant node
int TestConstantNode() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int c = g.constant(3.14);
	auto& node = g.get_node(c);
	VERIFY(node.op == OpKind::Constant, "expected Constant op");
	VERIFY(node.lo == 3.14 && node.hi == 3.14, "constant range mismatch");
	VERIFY(node.ufp == 1, "constant ufp expected 1, got " << node.ufp);
	return nrOfFailedTestCases;
}

// Test zero constant
int TestZeroConstant() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int c = g.constant(0.0);
	VERIFY(g.get_node(c).ufp == 0, "zero constant ufp expected 0, got " << g.get_node(c).ufp);
	return nrOfFailedTestCases;
}

// Test graph with no requirements
int TestNoRequirements() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int a = g.variable("a", 1.0, 10.0);
	int b = g.variable("b", 1.0, 10.0);
	g.add(a, b);
	g.analyze();
	for (int i = 0; i < g.size(); ++i) {
		VERIFY(g.get_nsb(i) >= 1, "node " << i << " nsb < 1 with no requirements");
	}
	return nrOfFailedTestCases;
}

// Test OpKind to_string coverage
int TestOpKindStrings() {
	int nrOfFailedTestCases = 0;
	VERIFY(std::string(to_string(OpKind::Constant)) == "const", "Constant string");
	VERIFY(std::string(to_string(OpKind::Variable)) == "var", "Variable string");
	VERIFY(std::string(to_string(OpKind::Add)) == "+", "Add string");
	VERIFY(std::string(to_string(OpKind::Sub)) == "-", "Sub string");
	VERIFY(std::string(to_string(OpKind::Mul)) == "*", "Mul string");
	VERIFY(std::string(to_string(OpKind::Div)) == "/", "Div string");
	VERIFY(std::string(to_string(OpKind::Neg)) == "neg", "Neg string");
	VERIFY(std::string(to_string(OpKind::Abs)) == "abs", "Abs string");
	VERIFY(std::string(to_string(OpKind::Sqrt)) == "sqrt", "Sqrt string");
	return nrOfFailedTestCases;
}

// Test division range estimation with divisor spanning zero
int TestDivByZeroRange() {
	int nrOfFailedTestCases = 0;
	ExprGraph g;
	int x = g.variable("x", 1.0, 10.0);
	int y = g.variable("y", -1.0, 1.0);
	int z = g.div(x, y);
	auto& node = g.get_node(z);
	VERIFY(node.lo < -1e50 && node.hi > 1e50, "div-by-zero range not expanded: [" << node.lo << ", " << node.hi << "]");
	return nrOfFailedTestCases;
}

}} // namespace sw::universal

#define TEST_CASE(name, func) do { int f_ = func; if (f_) { std::cout << name << ": FAIL (" << f_ << " errors)\n"; nrOfFailedTestCases += f_; } else { std::cout << name << ": PASS\n"; } } while(0)

int main()
try {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;
	std::cout << "POP Expression Graph Tests\n" << std::string(40, '=') << "\n\n";

	TEST_CASE("Graph construction", TestGraphConstruction());
	TEST_CASE("Simple mul backward", TestSimpleMulBackward());
	TEST_CASE("Determinant analysis", TestDeterminantAnalysis());
	TEST_CASE("Range analyzer integration", TestRangeAnalyzerIntegration());
	TEST_CASE("Type recommendation", TestTypeRecommendation());
	TEST_CASE("Pythagorean analysis", TestPythagoreanAnalysis());
	TEST_CASE("Division operation", TestDivisionOp());
	TEST_CASE("Unary operations", TestUnaryOps());
	TEST_CASE("Abs spanning zero", TestAbsSpanningZero());
	TEST_CASE("Abs negative range", TestAbsNegativeRange());
	TEST_CASE("Multi-consumer", TestMultiConsumer());
	TEST_CASE("Constant node", TestConstantNode());
	TEST_CASE("Zero constant", TestZeroConstant());
	TEST_CASE("No requirements", TestNoRequirements());
	TEST_CASE("OpKind strings", TestOpKindStrings());
	TEST_CASE("Div-by-zero range", TestDivByZeroRange());

	std::cout << "\n" << (nrOfFailedTestCases == 0 ? "All expression graph tests PASSED" : std::to_string(nrOfFailedTestCases) + " test(s) FAILED") << "\n";
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const char* msg) { std::cerr << "Caught exception: " << msg << std::endl; return EXIT_FAILURE; }
catch (...) { std::cerr << "Caught unknown exception" << std::endl; return EXIT_FAILURE; }
