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

namespace sw { namespace universal {

// Test basic graph construction
int TestGraphConstruction() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int a = g.variable("a", 1.0, 10.0);
	int b = g.variable("b", 1.0, 10.0);
	int c = g.add(a, b);

	if (g.size() != 3) {
		std::cerr << "FAIL: expected 3 nodes, got " << g.size() << std::endl;
		++nrOfFailedTestCases;
	}

	auto& node_c = g.get_node(c);
	if (node_c.op != OpKind::Add) {
		std::cerr << "FAIL: expected Add op" << std::endl;
		++nrOfFailedTestCases;
	}
	if (node_c.lhs != a || node_c.rhs != b) {
		std::cerr << "FAIL: wrong input edges" << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test the determinant example: det = a*d - b*c
// With high accuracy requirement on det, backward analysis should
// propagate higher precision requirements to inputs
int TestDeterminantAnalysis() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;

	// Matrix entries: all in [8, 12] range (nearly singular)
	int a = g.variable("a", 8.0, 12.0);   // ufp = 3
	int b = g.variable("b", 8.0, 12.0);   // ufp = 3
	int c = g.variable("c", 8.0, 12.0);   // ufp = 3
	int d = g.variable("d", 8.0, 12.0);   // ufp = 3

	// Products: a*d in [64, 144], b*c in [64, 144]
	int ad = g.mul(a, d);   // ufp ~ 7
	int bc = g.mul(b, c);   // ufp ~ 7

	// Determinant: det = a*d - b*c, range [-80, 80], ufp ~ 6
	int det = g.sub(ad, bc);

	// Require 20 significant bits at the output
	g.require_nsb(det, 20);

	// Run analysis
	g.analyze();

	// The determinant should get at least 20 bits
	if (g.get_nsb(det) < 20) {
		std::cerr << "FAIL: det nsb should be >= 20, got " << g.get_nsb(det) << std::endl;
		++nrOfFailedTestCases;
	}

	// The products should need more bits than the output (due to subtraction)
	if (g.get_nsb(ad) < g.get_nsb(det)) {
		std::cerr << "FAIL: ad should need >= det bits due to cancellation" << std::endl;
		++nrOfFailedTestCases;
	}

	// The input variables should need even more (mul adds carry)
	if (g.get_nsb(a) < g.get_nsb(ad)) {
		std::cerr << "FAIL: input a should need >= ad bits" << std::endl;
		++nrOfFailedTestCases;
	}

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

	// Backward through mul: nsb(x) >= nsb(z) + carry = 11
	if (g.get_nsb(x) < 11) {
		std::cerr << "FAIL: mul backward x expected >= 11, got " << g.get_nsb(x) << std::endl;
		++nrOfFailedTestCases;
	}
	if (g.get_nsb(y) < 11) {
		std::cerr << "FAIL: mul backward y expected >= 11, got " << g.get_nsb(y) << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test with range_analyzer integration
int TestRangeAnalyzerIntegration() {
	int nrOfFailedTestCases = 0;

	// Simulate: we observed values in [0.5, 100.0]
	range_analyzer<double> ra;
	ra.observe(0.5);
	ra.observe(100.0);
	ra.observe(50.0);
	ra.observe(75.0);

	ExprGraph g;
	int x = g.variable("x", ra);

	auto& node = g.get_node(x);
	// lo should be 0.5, hi should be 100.0
	if (node.lo != 0.5 || node.hi != 100.0) {
		std::cerr << "FAIL: range_analyzer bridge lo/hi mismatch" << std::endl;
		++nrOfFailedTestCases;
	}

	// ufp should match range_analyzer
	if (node.ufp != ra.ufp()) {
		std::cerr << "FAIL: ufp mismatch: node=" << node.ufp << ", analyzer=" << ra.ufp() << std::endl;
		++nrOfFailedTestCases;
	}

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

	// With 10 nsb required, posit<16,1> (12 fraction bits) should suffice
	std::cout << "Type recommendation for z (nsb=" << g.get_nsb(z) << "): " << rec << std::endl;

	// The recommendation should not be empty
	if (rec.empty()) {
		std::cerr << "FAIL: empty type recommendation" << std::endl;
		++nrOfFailedTestCases;
	}

	// Print full report with types
	g.report(std::cout, advisor);

	return nrOfFailedTestCases;
}

// Test chain of operations: y = sqrt(a*a + b*b)
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

	// result should have at least 16 bits
	if (g.get_nsb(result) < 16) {
		std::cerr << "FAIL: pythagorean result should have >= 16 bits" << std::endl;
		++nrOfFailedTestCases;
	}

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

	auto& node_z = g.get_node(z);
	if (node_z.op != OpKind::Div) {
		std::cerr << "FAIL: expected Div op" << std::endl;
		++nrOfFailedTestCases;
	}

	if (g.get_nsb(z) < 12) {
		std::cerr << "FAIL: div z expected >= 12, got " << g.get_nsb(z) << std::endl;
		++nrOfFailedTestCases;
	}

	// Backward through div: inputs should need more bits than output
	if (g.get_nsb(x) < g.get_nsb(z)) {
		std::cerr << "FAIL: div input x should need >= z bits" << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test unary operations: neg, abs, sqrt
int TestUnaryOps() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int x = g.variable("x", 1.0, 10.0);
	int nx = g.neg(x);
	int ax = g.abs(x);

	auto& node_neg = g.get_node(nx);
	if (node_neg.op != OpKind::Neg) {
		std::cerr << "FAIL: expected Neg op" << std::endl;
		++nrOfFailedTestCases;
	}

	auto& node_abs = g.get_node(ax);
	if (node_abs.op != OpKind::Abs) {
		std::cerr << "FAIL: expected Abs op" << std::endl;
		++nrOfFailedTestCases;
	}

	// Neg range: [-10, -1]
	if (node_neg.lo != -10.0 || node_neg.hi != -1.0) {
		std::cerr << "FAIL: neg range mismatch: [" << node_neg.lo << ", " << node_neg.hi << "]" << std::endl;
		++nrOfFailedTestCases;
	}

	// Abs of positive range should be unchanged
	if (node_abs.lo != 1.0 || node_abs.hi != 10.0) {
		std::cerr << "FAIL: abs range mismatch" << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test abs with range spanning zero
int TestAbsSpanningZero() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int x = g.variable("x", -5.0, 10.0);
	int ax = g.abs(x);

	auto& node = g.get_node(ax);
	if (node.lo != 0.0) {
		std::cerr << "FAIL: abs spanning zero lo expected 0, got " << node.lo << std::endl;
		++nrOfFailedTestCases;
	}
	if (node.hi != 10.0) {
		std::cerr << "FAIL: abs spanning zero hi expected 10, got " << node.hi << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test abs with negative range
int TestAbsNegativeRange() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int x = g.variable("x", -10.0, -2.0);
	int ax = g.abs(x);

	auto& node = g.get_node(ax);
	// abs([-10, -2]) = [2, 10]
	if (node.lo != 2.0 || node.hi != 10.0) {
		std::cerr << "FAIL: abs negative range: [" << node.lo << ", " << node.hi
		          << "], expected [2, 10]" << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test multi-consumer node: a shared input used by two different ops
int TestMultiConsumer() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int x = g.variable("x", 1.0, 10.0);
	int y1 = g.mul(x, x);    // x*x
	int y2 = g.add(x, x);    // x+x

	g.require_nsb(y1, 16);
	g.require_nsb(y2, 8);
	g.analyze();

	// x should satisfy the more demanding consumer (y1 needs 16+1=17)
	if (g.get_nsb(x) < 17) {
		std::cerr << "FAIL: multi-consumer x expected >= 17 (from mul), got " << g.get_nsb(x) << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test constant node
int TestConstantNode() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int c = g.constant(3.14);

	auto& node = g.get_node(c);
	if (node.op != OpKind::Constant) {
		std::cerr << "FAIL: expected Constant op" << std::endl;
		++nrOfFailedTestCases;
	}
	if (node.lo != 3.14 || node.hi != 3.14) {
		std::cerr << "FAIL: constant range mismatch" << std::endl;
		++nrOfFailedTestCases;
	}
	if (node.ufp != 1) { // floor(log2(3.14)) = 1
		std::cerr << "FAIL: constant ufp expected 1, got " << node.ufp << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test zero constant
int TestZeroConstant() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int c = g.constant(0.0);

	auto& node = g.get_node(c);
	if (node.ufp != 0) {
		std::cerr << "FAIL: zero constant ufp expected 0, got " << node.ufp << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test graph with no requirements (no backward propagation)
int TestNoRequirements() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int a = g.variable("a", 1.0, 10.0);
	int b = g.variable("b", 1.0, 10.0);
	int c = g.add(a, b);
	(void)c;

	// No requirements set - analyze should still work
	g.analyze();

	// All nodes should have nsb_final >= 1
	for (int i = 0; i < g.size(); ++i) {
		if (g.get_nsb(i) < 1) {
			std::cerr << "FAIL: node " << i << " nsb < 1 with no requirements" << std::endl;
			++nrOfFailedTestCases;
		}
	}

	return nrOfFailedTestCases;
}

// Test OpKind to_string coverage
int TestOpKindStrings() {
	int nrOfFailedTestCases = 0;

	if (std::string(to_string(OpKind::Constant)) != "const") ++nrOfFailedTestCases;
	if (std::string(to_string(OpKind::Variable)) != "var") ++nrOfFailedTestCases;
	if (std::string(to_string(OpKind::Add)) != "+") ++nrOfFailedTestCases;
	if (std::string(to_string(OpKind::Sub)) != "-") ++nrOfFailedTestCases;
	if (std::string(to_string(OpKind::Mul)) != "*") ++nrOfFailedTestCases;
	if (std::string(to_string(OpKind::Div)) != "/") ++nrOfFailedTestCases;
	if (std::string(to_string(OpKind::Neg)) != "neg") ++nrOfFailedTestCases;
	if (std::string(to_string(OpKind::Abs)) != "abs") ++nrOfFailedTestCases;
	if (std::string(to_string(OpKind::Sqrt)) != "sqrt") ++nrOfFailedTestCases;

	return nrOfFailedTestCases;
}

// Test division range estimation with divisor spanning zero
int TestDivByZeroRange() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int x = g.variable("x", 1.0, 10.0);
	int y = g.variable("y", -1.0, 1.0); // spans zero
	int z = g.div(x, y);

	auto& node = g.get_node(z);
	// Should get very large range due to division by zero potential
	if (node.lo > -1e50 || node.hi < 1e50) {
		std::cerr << "FAIL: div-by-zero range not expanded: [" << node.lo << ", " << node.hi << "]" << std::endl;
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

	std::cout << "POP Expression Graph Tests\n";
	std::cout << std::string(40, '=') << "\n\n";

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

	std::cout << "\n";
	if (nrOfFailedTestCases == 0) {
		std::cout << "All expression graph tests PASSED\n";
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
