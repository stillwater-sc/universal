// posit.cpp: testbench for a posit hardware ALU
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/hw/alu.hpp>
#include <universal/number/posit/posit.hpp>

#include <universal/verification/test_status.hpp> // ReportTestResult

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc != 4) {
		std::cerr << "Usage: hw_posit [add | sub | mul | div | sqrt] a b\n";
		std::cerr << "Example: hw_posit add 1.5 -1.5\n";
		return EXIT_SUCCESS;  // needed for regression test success
	}
	std::string op = argv[1];
	float fa = atof(argv[2]);
	float fb = atof(argv[3]);
	std::cout << op << " " << fa << " and " << fb << '\n';

	posit<8, 2> a, b, c;
	a = fa;
	b = fb;

	// decode the operation
	ALU_OPS alu_op;
	if (op == "add") {
		alu_op = ALU_OPS::ADD;
		c = ArithmeticLogicUnit(alu_op, a, b);
		std::cout << a << " + " << b << " = " << c << '\n';
		std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(c) << '\n';
	}
	else if (op == "sub") {
		alu_op = ALU_OPS::SUB;
		c = ArithmeticLogicUnit(alu_op, a, b);
		std::cout << a << " - " << b << " = " << c << '\n';
		std::cout << to_binary(a) << " - " << to_binary(b) << " = " << to_binary(c) << '\n';
	}
	else if (op == "mul") {
		alu_op = ALU_OPS::MUL;
		c = ArithmeticLogicUnit(alu_op, a, b);
		std::cout << a << " * " << b << " = " << c << '\n';
		std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << '\n';
	}
	else if (op == "div") {
		alu_op = ALU_OPS::DIV;
		c = ArithmeticLogicUnit(alu_op, a, b);
		std::cout << a << " / " << b << " = " << c << '\n';
		std::cout << to_binary(a) << " / " << to_binary(b) << " = " << to_binary(c) << '\n';
	}
	else if (op == "sqrt") {
		alu_op = ALU_OPS::SQRT;
		c = ArithmeticLogicUnit(alu_op, a, b);
		std::cout << "sqrt(" << a << ") = " << c << '\n';
		std::cout << "sqrt(" << to_binary(a) << " = " << to_binary(c) << '\n';
	}

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
