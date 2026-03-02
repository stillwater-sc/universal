// test_complete_workflow.cpp: end-to-end POP precision tuning workflow
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Demonstrates the complete POP workflow:
//   1. Profile with range_analyzer (dynamic analysis)
//   2. Build expression graph
//   3. Run LP-based optimal bit assignment
//   4. Refine carries
//   5. Generate mixed-precision code
//
// Uses the umbrella header <universal/mixedprecision/pop.hpp>.

#include <universal/utility/directives.hpp>
#include <universal/mixedprecision/pop.hpp>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>

namespace sw { namespace universal {

// Simulate profiling a dot product: result = sum(a[i] * b[i])
// This models a real workflow where range_analyzer tracks observed values
int TestDotProductWorkflow() {
	int nrOfFailedTestCases = 0;

	std::cout << "=== Dot Product Workflow ===\n\n";

	// Step 1: Profile with range_analyzer
	std::cout << "Step 1: Dynamic profiling with range_analyzer\n";

	range_analyzer<double> ra_a, ra_b, ra_product, ra_sum;

	// Simulate a dot product of 4 elements
	double a[] = { 1.5, -2.3, 0.7, 3.1 };
	double b[] = { 4.2,  1.8, 5.5, -0.9 };

	double sum = 0.0;
	for (int i = 0; i < 4; ++i) {
		ra_a.observe(a[i]);
		ra_b.observe(b[i]);
		double prod = a[i] * b[i];
		ra_product.observe(prod);
		sum += prod;
		ra_sum.observe(sum);
	}

	std::cout << "  a range: [" << static_cast<double>(ra_a.minValue()) << ", "
	          << static_cast<double>(ra_a.maxValue()) << "], ufp=" << ra_a.ufp() << "\n";
	std::cout << "  b range: [" << static_cast<double>(ra_b.minValue()) << ", "
	          << static_cast<double>(ra_b.maxValue()) << "], ufp=" << ra_b.ufp() << "\n";
	std::cout << "  product range: [" << static_cast<double>(ra_product.minValue()) << ", "
	          << static_cast<double>(ra_product.maxValue()) << "], ufp=" << ra_product.ufp() << "\n";
	std::cout << "  result = " << sum << "\n\n";

	// Step 2: Build expression graph
	std::cout << "Step 2: Build expression graph\n";

	ExprGraph g;

	// Model: result = a0*b0 + a1*b1 + a2*b2 + a3*b3
	int va = g.variable("a", ra_a);
	int vb = g.variable("b", ra_b);
	int prod = g.mul(va, vb);
	int accum = prod; // start accumulation

	// Chain addition (simplified model for dot product)
	for (int i = 1; i < 4; ++i) {
		std::string ai = "a" + std::to_string(i);
		std::string bi = "b" + std::to_string(i);
		int a_i = g.variable(ai, ra_a);
		int b_i = g.variable(bi, ra_b);
		int p_i = g.mul(a_i, b_i);
		accum = g.add(accum, p_i);
	}

	// Require 16 significant bits at output
	g.require_nsb(accum, 16);

	std::cout << "  Graph has " << g.size() << " nodes\n\n";

	// Step 3: LP-based optimal bit assignment
	std::cout << "Step 3: LP-based optimal bit assignment\n";

	PopSolver solver;
	bool ok = solver.solve(g);
	if (!ok) {
		std::cerr << "FAIL: LP solver failed" << std::endl;
		++nrOfFailedTestCases;
		return nrOfFailedTestCases;
	}

	solver.report(std::cout, g);
	std::cout << "\n";

	// Step 4: Carry refinement
	std::cout << "Step 4: Carry-bit refinement\n";

	double pre_total = solver.total_nsb();
	CarryAnalyzer ca;
	ca.refine(g);

	double post_total = 0;
	for (int i = 0; i < g.size(); ++i) post_total += g.get_nsb(i);

	ca.report(std::cout, g);
	std::cout << "  Pre-refinement total: " << pre_total << "\n";
	std::cout << "  Post-refinement total: " << post_total << "\n\n";

	// Step 5: Code generation
	std::cout << "Step 5: Code generation\n";

	TypeAdvisor advisor;
	PopCodeGenerator gen(g, advisor);

	std::cout << gen.generateReport() << "\n";
	std::cout << gen.generateHeader("DOT_PRODUCT_PRECISION_HPP") << "\n";

	// Verify output meets requirement
	if (g.get_nsb(accum) < 16) {
		std::cerr << "FAIL: output does not meet 16-bit requirement" << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Simpson integration workflow: model f(x) = x^2 integrated on [0,1]
// Simpson: (h/3) * (f(a) + 4*f(m) + f(b))
int TestSimpsonWorkflow() {
	int nrOfFailedTestCases = 0;

	std::cout << "=== Simpson Integration Workflow ===\n\n";

	ExprGraph g;

	// h = (b-a)/2 = 0.5
	int h = g.constant(0.5, "h");

	// f(a) = 0^2 = 0, f(m) = 0.5^2 = 0.25, f(b) = 1^2 = 1
	int fa = g.constant(0.0, "fa");
	int fm = g.variable("fm", 0.2, 0.3);  // f at midpoint
	int fb = g.variable("fb", 0.9, 1.1);  // f at endpoint

	// Simpson formula: (h/3) * (fa + 4*fm + fb)
	int four = g.constant(4.0, "four");
	int three = g.constant(3.0, "three");

	int fm4 = g.mul(four, fm);       // 4 * f(m)
	int sum1 = g.add(fa, fm4);       // f(a) + 4*f(m)
	int sum2 = g.add(sum1, fb);      // f(a) + 4*f(m) + f(b)
	int h_div_3 = g.div(h, three);   // h/3
	int result = g.mul(h_div_3, sum2); // (h/3) * (...)

	// Require 24 bits at the integration result
	g.require_nsb(result, 24);

	// Run full POP analysis
	PopSolver solver;
	bool ok = solver.solve(g);
	if (!ok) {
		std::cerr << "FAIL: Simpson LP solver failed" << std::endl;
		++nrOfFailedTestCases;
		return nrOfFailedTestCases;
	}

	// Carry refinement
	CarryAnalyzer ca;
	ca.refine(g);

	// Report
	TypeAdvisor advisor;
	g.report(std::cout, advisor);

	PopCodeGenerator gen(g, advisor);
	std::cout << gen.generateReport() << "\n";

	if (g.get_nsb(result) < 24) {
		std::cerr << "FAIL: Simpson result does not meet 24-bit requirement" << std::endl;
		++nrOfFailedTestCases;
	}

	std::cout << "Simpson integration: PASS\n\n";

	return nrOfFailedTestCases;
}

// Test iterative fixpoint (no LP) workflow using pop.hpp umbrella
int TestFixpointOnlyWorkflow() {
	int nrOfFailedTestCases = 0;

	std::cout << "=== Fixpoint-Only Workflow (no LP) ===\n\n";

	ExprGraph g;
	int x = g.variable("x", 1.0, 100.0);
	int y = g.variable("y", 1.0, 100.0);
	int z = g.mul(x, y);
	int w = g.add(z, x);

	g.require_nsb(w, 20);

	// Use iterative fixpoint analysis (Phase 2 only, no LP)
	g.analyze();

	std::cout << "Fixpoint analysis:\n";
	g.report(std::cout);

	if (g.get_nsb(w) < 20) {
		std::cerr << "FAIL: fixpoint output does not meet requirement" << std::endl;
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

	std::cout << "POP Complete Workflow Tests\n";
	std::cout << std::string(50, '=') << "\n\n";

	TEST_CASE("Dot product workflow", TestDotProductWorkflow());
	TEST_CASE("Simpson integration workflow", TestSimpsonWorkflow());
	TEST_CASE("Fixpoint-only workflow", TestFixpointOnlyWorkflow());

	std::cout << "\n";
	if (nrOfFailedTestCases == 0) {
		std::cout << "All complete workflow tests PASSED\n";
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
