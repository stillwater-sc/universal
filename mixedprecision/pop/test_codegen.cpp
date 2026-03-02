// test_codegen.cpp: validate POP code generation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

#include <universal/utility/directives.hpp>
#include <universal/mixedprecision/pop.hpp>
#include <iostream>
#include <string>

namespace sw { namespace universal {

// Test header generation
int TestHeaderGeneration() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int a = g.variable("a", 1.0, 10.0);
	int b = g.variable("b", 1.0, 10.0);
	int c = g.mul(a, b);
	(void)a; (void)b;

	g.require_nsb(c, 10);

	PopSolver solver;
	solver.solve(g);

	PopCodeGenerator gen(g);
	std::string header = gen.generateHeader();

	// Should contain #pragma once and type aliases
	if (header.find("#pragma once") == std::string::npos) {
		std::cerr << "FAIL: header missing #pragma once" << std::endl;
		++nrOfFailedTestCases;
	}
	if (header.find("type_a") == std::string::npos) {
		std::cerr << "FAIL: header missing type_a alias" << std::endl;
		++nrOfFailedTestCases;
	}
	if (header.find("type_b") == std::string::npos) {
		std::cerr << "FAIL: header missing type_b alias" << std::endl;
		++nrOfFailedTestCases;
	}
	if (header.find("sw::universal::") == std::string::npos) {
		std::cerr << "FAIL: header missing sw::universal:: prefix" << std::endl;
		++nrOfFailedTestCases;
	}

	std::cout << "Generated header:\n" << header << "\n";

	return nrOfFailedTestCases;
}

// Test report generation
int TestReportGeneration() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int a = g.variable("a", 1.0, 10.0);
	int b = g.variable("b", 1.0, 10.0);
	int c = g.mul(a, b);
	(void)a; (void)b;

	g.require_nsb(c, 10);

	PopSolver solver;
	solver.solve(g);

	PopCodeGenerator gen(g);
	std::string report = gen.generateReport();

	if (report.find("POP Precision Analysis Report") == std::string::npos) {
		std::cerr << "FAIL: report missing title" << std::endl;
		++nrOfFailedTestCases;
	}
	if (report.find("Bit savings") == std::string::npos) {
		std::cerr << "FAIL: report missing savings calculation" << std::endl;
		++nrOfFailedTestCases;
	}

	std::cout << report << "\n";

	return nrOfFailedTestCases;
}

// Test example code generation
int TestExampleCodeGeneration() {
	int nrOfFailedTestCases = 0;

	ExprGraph g;
	int a = g.variable("a", 1.0, 10.0);
	int b = g.variable("b", 1.0, 10.0);
	int c = g.mul(a, b);
	(void)a; (void)b;

	g.require_nsb(c, 10);

	PopSolver solver;
	solver.solve(g);

	PopCodeGenerator gen(g);
	std::string code = gen.generateExampleCode("multiply");

	if (code.find("auto multiply(") == std::string::npos) {
		std::cerr << "FAIL: code missing function signature" << std::endl;
		++nrOfFailedTestCases;
	}
	if (code.find("a * b") == std::string::npos) {
		std::cerr << "FAIL: code missing multiplication" << std::endl;
		++nrOfFailedTestCases;
	}
	if (code.find("return") == std::string::npos) {
		std::cerr << "FAIL: code missing return statement" << std::endl;
		++nrOfFailedTestCases;
	}

	std::cout << "Generated code:\n" << code << "\n";

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

	std::cout << "POP Code Generation Tests\n";
	std::cout << std::string(40, '=') << "\n\n";

	TEST_CASE("Header generation", TestHeaderGeneration());
	TEST_CASE("Report generation", TestReportGeneration());
	TEST_CASE("Example code generation", TestExampleCodeGeneration());

	std::cout << "\n";
	if (nrOfFailedTestCases == 0) {
		std::cout << "All code generation tests PASSED\n";
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
