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

namespace sw { namespace universal {

// Test 1: simple 2-variable LP
// minimize  x + y
// subject to  x >= 3
//             y >= 5
// Solution: x=3, y=5, objective=8
int TestSimple2Var() {
	int nrOfFailedTestCases = 0;

	SimplexSolver lp;
	lp.set_num_vars(2);
	lp.set_objective({1.0, 1.0});

	lp.add_ge_constraint({1.0, 0.0}, 3.0);
	lp.add_ge_constraint({0.0, 1.0}, 5.0);

	LPStatus status = lp.solve();
	if (status != LPStatus::Optimal) {
		std::cerr << "FAIL: expected Optimal, got " << to_string(status) << std::endl;
		++nrOfFailedTestCases;
		return nrOfFailedTestCases;
	}

	double x = lp.get_value(0);
	double y = lp.get_value(1);

	if (std::abs(x - 3.0) > 0.01) {
		std::cerr << "FAIL: expected x=3, got " << x << std::endl;
		++nrOfFailedTestCases;
	}
	if (std::abs(y - 5.0) > 0.01) {
		std::cerr << "FAIL: expected y=5, got " << y << std::endl;
		++nrOfFailedTestCases;
	}
	if (std::abs(lp.objective_value() - 8.0) > 0.01) {
		std::cerr << "FAIL: expected obj=8, got " << lp.objective_value() << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test 2: LP with relational constraints
// minimize  2x + 3y
// subject to  x + y >= 10
//             x >= 2
//             y >= 3
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
	if (status != LPStatus::Optimal) {
		std::cerr << "FAIL: expected Optimal, got " << to_string(status) << std::endl;
		++nrOfFailedTestCases;
		return nrOfFailedTestCases;
	}

	double x = lp.get_value(0);
	double y = lp.get_value(1);

	if (std::abs(x - 7.0) > 0.01) {
		std::cerr << "FAIL: expected x=7, got " << x << std::endl;
		++nrOfFailedTestCases;
	}
	if (std::abs(y - 3.0) > 0.01) {
		std::cerr << "FAIL: expected y=3, got " << y << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test 3: constraints that mimic POP transfer functions
// minimize  a + b + z
// subject to  a - z >= 1    (backward mul: nsb(a) >= nsb(z) + 1)
//             b - z >= 1    (backward mul: nsb(b) >= nsb(z) + 1)
//             z >= 10       (output requirement)
// Solution: a=11, b=11, z=10, objective=32
int TestPopLikeConstraints() {
	int nrOfFailedTestCases = 0;

	SimplexSolver lp;
	lp.set_num_vars(3);
	lp.set_objective({1.0, 1.0, 1.0});

	// a >= z + 1  =>  a - z >= 1
	lp.add_ge_constraint({1.0, 0.0, -1.0}, 1.0);
	// b >= z + 1  =>  b - z >= 1
	lp.add_ge_constraint({0.0, 1.0, -1.0}, 1.0);
	// z >= 10
	lp.add_ge_constraint({0.0, 0.0, 1.0}, 10.0);
	// a,b,z >= 1
	lp.add_ge_constraint({1.0, 0.0, 0.0}, 1.0);
	lp.add_ge_constraint({0.0, 1.0, 0.0}, 1.0);

	LPStatus status = lp.solve();
	if (status != LPStatus::Optimal) {
		std::cerr << "FAIL: expected Optimal, got " << to_string(status) << std::endl;
		++nrOfFailedTestCases;
		return nrOfFailedTestCases;
	}

	double a = lp.get_value(0);
	double b = lp.get_value(1);
	double z = lp.get_value(2);

	if (std::abs(a - 11.0) > 0.01) {
		std::cerr << "FAIL: expected a=11, got " << a << std::endl;
		++nrOfFailedTestCases;
	}
	if (std::abs(b - 11.0) > 0.01) {
		std::cerr << "FAIL: expected b=11, got " << b << std::endl;
		++nrOfFailedTestCases;
	}
	if (std::abs(z - 10.0) > 0.01) {
		std::cerr << "FAIL: expected z=10, got " << z << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

// Test 4: 3-variable LP with mixed constraints
// minimize  x + y + z
// subject to  x + y >= 5
//             y + z >= 7
//             x >= 1
//             z >= 1
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
	if (status != LPStatus::Optimal) {
		std::cerr << "FAIL: expected Optimal, got " << to_string(status) << std::endl;
		++nrOfFailedTestCases;
		return nrOfFailedTestCases;
	}

	double x = lp.get_value(0);
	double y = lp.get_value(1);
	double z = lp.get_value(2);
	double obj = lp.objective_value();

	// x=1, y=4, z=3
	if (std::abs(obj - 8.0) > 0.01) {
		std::cerr << "FAIL: expected obj=8, got " << obj
		          << " (x=" << x << ", y=" << y << ", z=" << z << ")" << std::endl;
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

	std::cout << "POP Simplex Solver Tests\n";
	std::cout << std::string(40, '=') << "\n\n";

	TEST_CASE("Simple 2-var LP", TestSimple2Var());
	TEST_CASE("Relational constraints", TestRelational());
	TEST_CASE("POP-like constraints", TestPopLikeConstraints());
	TEST_CASE("Three-variable LP", TestThreeVar());

	std::cout << "\n";
	if (nrOfFailedTestCases == 0) {
		std::cout << "All simplex solver tests PASSED\n";
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
