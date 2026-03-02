// test_simplex.cpp: validate the embedded simplex LP solver
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

#include <universal/utility/directives.hpp>
#include <universal/mixedprecision/simplex.hpp>
#include <iostream>
#include <string>
#include <cmath>

// Single-line assertion: failure branch is one line for coverage purposes
#define VERIFY(cond, msg) do { if (!(cond)) { std::cerr << "FAIL: " << msg << std::endl; ++nrOfFailedTestCases; } } while(0)
#define VERIFY_NEAR(val, expected, tol, msg) VERIFY(std::abs((val) - (expected)) <= (tol), msg << ": expected " << (expected) << ", got " << (val))
#define VERIFY_STATUS(status, expected) VERIFY((status) == (expected), "expected " << to_string(expected) << ", got " << to_string(status))

namespace sw { namespace universal {

// Test 1: simple 2-variable LP
// minimize  x + y  subject to  x >= 3, y >= 5
// Solution: x=3, y=5, objective=8
int TestSimple2Var() {
	int nrOfFailedTestCases = 0;
	SimplexSolver lp;
	lp.set_num_vars(2);
	lp.set_objective({1.0, 1.0});
	lp.add_ge_constraint({1.0, 0.0}, 3.0);
	lp.add_ge_constraint({0.0, 1.0}, 5.0);
	LPStatus status = lp.solve();
	VERIFY_STATUS(status, LPStatus::Optimal);
	if (status != LPStatus::Optimal) return nrOfFailedTestCases;
	VERIFY_NEAR(lp.get_value(0), 3.0, 0.01, "x");
	VERIFY_NEAR(lp.get_value(1), 5.0, 0.01, "y");
	VERIFY_NEAR(lp.objective_value(), 8.0, 0.01, "obj");
	return nrOfFailedTestCases;
}

// Test 2: LP with relational constraints
// minimize  2x + 3y  subject to  x + y >= 10, x >= 2, y >= 3
// Solution: x=7, y=3, objective=23
int TestRelational() {
	int nrOfFailedTestCases = 0;
	SimplexSolver lp;
	lp.set_num_vars(2);
	lp.set_objective({2.0, 3.0});
	lp.add_ge_constraint({1.0, 1.0}, 10.0);
	lp.add_ge_constraint({1.0, 0.0}, 2.0);
	lp.add_ge_constraint({0.0, 1.0}, 3.0);
	LPStatus status = lp.solve();
	VERIFY_STATUS(status, LPStatus::Optimal);
	if (status != LPStatus::Optimal) return nrOfFailedTestCases;
	VERIFY_NEAR(lp.get_value(0), 7.0, 0.01, "x");
	VERIFY_NEAR(lp.get_value(1), 3.0, 0.01, "y");
	return nrOfFailedTestCases;
}

// Test 3: constraints that mimic POP transfer functions
// minimize  a + b + z  subject to  a - z >= 1, b - z >= 1, z >= 10, a,b >= 1
// Solution: a=11, b=11, z=10, objective=32
int TestPopLikeConstraints() {
	int nrOfFailedTestCases = 0;
	SimplexSolver lp;
	lp.set_num_vars(3);
	lp.set_objective({1.0, 1.0, 1.0});
	lp.add_ge_constraint({1.0, 0.0, -1.0}, 1.0);
	lp.add_ge_constraint({0.0, 1.0, -1.0}, 1.0);
	lp.add_ge_constraint({0.0, 0.0, 1.0}, 10.0);
	lp.add_ge_constraint({1.0, 0.0, 0.0}, 1.0);
	lp.add_ge_constraint({0.0, 1.0, 0.0}, 1.0);
	LPStatus status = lp.solve();
	VERIFY_STATUS(status, LPStatus::Optimal);
	if (status != LPStatus::Optimal) return nrOfFailedTestCases;
	VERIFY_NEAR(lp.get_value(0), 11.0, 0.01, "a");
	VERIFY_NEAR(lp.get_value(1), 11.0, 0.01, "b");
	VERIFY_NEAR(lp.get_value(2), 10.0, 0.01, "z");
	return nrOfFailedTestCases;
}

// Test 4: 3-variable LP with mixed constraints
// minimize  x + y + z  subject to  x + y >= 5, y + z >= 7, x >= 1, z >= 1
// Solution: x=1, y=4, z=3, objective=8
int TestThreeVar() {
	int nrOfFailedTestCases = 0;
	SimplexSolver lp;
	lp.set_num_vars(3);
	lp.set_objective({1.0, 1.0, 1.0});
	lp.add_ge_constraint({1.0, 1.0, 0.0}, 5.0);
	lp.add_ge_constraint({0.0, 1.0, 1.0}, 7.0);
	lp.add_ge_constraint({1.0, 0.0, 0.0}, 1.0);
	lp.add_ge_constraint({0.0, 0.0, 1.0}, 1.0);
	LPStatus status = lp.solve();
	VERIFY_STATUS(status, LPStatus::Optimal);
	if (status != LPStatus::Optimal) return nrOfFailedTestCases;
	VERIFY_NEAR(lp.objective_value(), 8.0, 0.01, "obj");
	return nrOfFailedTestCases;
}

// Test 5: Infeasible LP
// minimize  x + y  subject to  x + y >= 10, x + y <= 5
int TestInfeasible() {
	int nrOfFailedTestCases = 0;
	SimplexSolver lp;
	lp.set_num_vars(2);
	lp.set_objective({1.0, 1.0});
	lp.add_ge_constraint({1.0, 1.0}, 10.0);
	lp.add_le_constraint({1.0, 1.0}, 5.0);
	LPStatus status = lp.solve();
	VERIFY_STATUS(status, LPStatus::Infeasible);
	VERIFY(std::isnan(lp.objective_value()), "expected NaN objective for infeasible");
	return nrOfFailedTestCases;
}

// Test 6: Unbounded LP
// minimize  -x  subject to  x >= 1
int TestUnbounded() {
	int nrOfFailedTestCases = 0;
	SimplexSolver lp;
	lp.set_num_vars(1);
	lp.set_objective({-1.0});
	lp.add_ge_constraint({1.0}, 1.0);
	LPStatus status = lp.solve();
	VERIFY_STATUS(status, LPStatus::Unbounded);
	return nrOfFailedTestCases;
}

// Test 7: MaxIterations
// minimize  x + y  subject to  x >= 3, y >= 5  with max_iterations=1
int TestMaxIterations() {
	int nrOfFailedTestCases = 0;
	SimplexSolver lp;
	lp.set_num_vars(2);
	lp.set_objective({1.0, 1.0});
	lp.add_ge_constraint({1.0, 0.0}, 3.0);
	lp.add_ge_constraint({0.0, 1.0}, 5.0);
	LPStatus status = lp.solve(1);
	// Small problem might solve in 1 iteration; both Optimal and MaxIterations are acceptable
	VERIFY(status == LPStatus::Optimal || status == LPStatus::MaxIterations,
	       "expected Optimal or MaxIterations, got " << to_string(status));
	return nrOfFailedTestCases;
}

// Test 8: Empty LP (no constraints or variables)
int TestEmptyLP() {
	int nrOfFailedTestCases = 0;
	{ // Zero variables
		SimplexSolver lp;
		lp.set_num_vars(0);
		VERIFY(lp.solve() != LPStatus::Optimal, "empty LP should not be Optimal");
	}
	{ // Variables but no constraints
		SimplexSolver lp;
		lp.set_num_vars(2);
		lp.set_objective({1.0, 1.0});
		VERIFY(lp.solve() != LPStatus::Optimal, "LP with no constraints should not be Optimal");
	}
	return nrOfFailedTestCases;
}

// Test 9: Single variable LP
// minimize  x  subject to  x >= 7
int TestSingleVar() {
	int nrOfFailedTestCases = 0;
	SimplexSolver lp;
	lp.set_num_vars(1);
	lp.set_objective({1.0});
	lp.add_ge_constraint({1.0}, 7.0);
	LPStatus status = lp.solve();
	VERIFY_STATUS(status, LPStatus::Optimal);
	if (status != LPStatus::Optimal) return nrOfFailedTestCases;
	VERIFY_NEAR(lp.get_value(0), 7.0, 0.01, "x");
	return nrOfFailedTestCases;
}

// Test 10: equality constraint
// minimize  x + y  subject to  x + y == 10, x >= 3
// Solution: x=3, y=7
int TestLeAndEqConstraints() {
	int nrOfFailedTestCases = 0;
	SimplexSolver lp;
	lp.set_num_vars(2);
	lp.set_objective({1.0, 1.0});
	lp.add_eq_constraint({1.0, 1.0}, 10.0);
	lp.add_ge_constraint({1.0, 0.0}, 3.0);
	LPStatus status = lp.solve();
	VERIFY_STATUS(status, LPStatus::Optimal);
	if (status != LPStatus::Optimal) return nrOfFailedTestCases;
	VERIFY_NEAR(lp.get_value(0), 3.0, 0.01, "x");
	VERIFY_NEAR(lp.get_value(1), 7.0, 0.01, "y");
	VERIFY_NEAR(lp.objective_value(), 10.0, 0.01, "obj");
	return nrOfFailedTestCases;
}

// Test 11: Negative RHS normalization
// minimize  x + y  subject to  -x - y >= -10, x >= 3, y >= 5
// Solution: x=3, y=5, objective=8
int TestNegativeRHS() {
	int nrOfFailedTestCases = 0;
	SimplexSolver lp;
	lp.set_num_vars(2);
	lp.set_objective({1.0, 1.0});
	lp.add_ge_constraint({-1.0, -1.0}, -10.0);
	lp.add_ge_constraint({1.0, 0.0}, 3.0);
	lp.add_ge_constraint({0.0, 1.0}, 5.0);
	LPStatus status = lp.solve();
	VERIFY_STATUS(status, LPStatus::Optimal);
	if (status != LPStatus::Optimal) return nrOfFailedTestCases;
	VERIFY_NEAR(lp.objective_value(), 8.0, 0.01, "obj");
	return nrOfFailedTestCases;
}

// Test 12: LPStatus to_string coverage
int TestLPStatusStrings() {
	int nrOfFailedTestCases = 0;
	VERIFY(std::string(to_string(LPStatus::Optimal)) == "Optimal", "Optimal string");
	VERIFY(std::string(to_string(LPStatus::Infeasible)) == "Infeasible", "Infeasible string");
	VERIFY(std::string(to_string(LPStatus::Unbounded)) == "Unbounded", "Unbounded string");
	VERIFY(std::string(to_string(LPStatus::MaxIterations)) == "MaxIterations", "MaxIterations string");
	return nrOfFailedTestCases;
}

}} // namespace sw::universal

#define TEST_CASE(name, func) do { int f_ = func; if (f_) { std::cout << name << ": FAIL (" << f_ << " errors)\n"; nrOfFailedTestCases += f_; } else { std::cout << name << ": PASS\n"; } } while(0)

int main()
try {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;
	std::cout << "POP Simplex Solver Tests\n" << std::string(40, '=') << "\n\n";

	TEST_CASE("Simple 2-var LP", TestSimple2Var());
	TEST_CASE("Relational constraints", TestRelational());
	TEST_CASE("POP-like constraints", TestPopLikeConstraints());
	TEST_CASE("Three-variable LP", TestThreeVar());
	TEST_CASE("Infeasible LP", TestInfeasible());
	TEST_CASE("Unbounded LP", TestUnbounded());
	TEST_CASE("MaxIterations LP", TestMaxIterations());
	TEST_CASE("Empty LP", TestEmptyLP());
	TEST_CASE("Single variable LP", TestSingleVar());
	TEST_CASE("LE and EQ constraints", TestLeAndEqConstraints());
	TEST_CASE("Negative RHS", TestNegativeRHS());
	TEST_CASE("LPStatus strings", TestLPStatusStrings());

	std::cout << "\n" << (nrOfFailedTestCases == 0 ? "All simplex solver tests PASSED" : std::to_string(nrOfFailedTestCases) + " test(s) FAILED") << "\n";
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const char* msg) { std::cerr << "Caught exception: " << msg << std::endl; return EXIT_FAILURE; }
catch (...) { std::cerr << "Caught unknown exception" << std::endl; return EXIT_FAILURE; }
