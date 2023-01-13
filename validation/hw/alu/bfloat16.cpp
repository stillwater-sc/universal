// bfloat16.cpp: testbench for a bfloat16 hardware ALU
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/hw/alu.hpp>
#include <universal/number/cfloat/cfloat.hpp>

#include <universal/verification/test_status.hpp> // ReportTestResult

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc != 4) {
		std::cerr << "Usage: hw_bfloat16 [add | sub | mul | div | sqrt] [a b]\n";
		std::cerr << "Example: hw_bfloat16 add 1.5 -1.5\n";
		return EXIT_SUCCESS;  // needed for regression test success
	}
	std::string op = argv[1];
	float fa = atof(argv[2]);
	float fb = atof(argv[3]);
	std::cout << op << " " << fa << ' ' << fb << '\n';

	using bfloat16 = cfloat<16, 8, std::uint16_t, false, false, false>;

	bfloat16 a, b, c;
	a = fa;
	b = fb;

	// decode the operation
	ALU_OPS alu_op;
	if (op == "add") {
		alu_op = ALU_OPS::ADD;
		c = ArithmeticLogicUnit(alu_op, a, b);
		std::cout << a << " + " << b << " = " << c << '\n';
		std::cout << to_binary(a, true) << " + " << to_binary(b, true) << " = " << to_binary(c, true) << '\n';
	}
	else if (op == "sub") {
		alu_op = ALU_OPS::SUB;
		c = ArithmeticLogicUnit(alu_op, a, b);
		std::cout << a << " - " << b << " = " << c << '\n';
		std::cout << to_binary(a, true) << " - " << to_binary(b, true) << " = " << to_binary(c, true) << '\n';
	}
	else if (op == "mul") {
		alu_op = ALU_OPS::MUL;
		c = ArithmeticLogicUnit(alu_op, a, b);
		std::cout << a << " * " << b << " = " << c << '\n';
		std::cout << to_binary(a, true) << " * " << to_binary(b, true) << " = " << to_binary(c, true) << '\n';
	}
	else if (op == "div") {
		alu_op = ALU_OPS::DIV;
		c = ArithmeticLogicUnit(alu_op, a, b);
		std::cout << a << " / " << b << " = " << c << '\n';
		std::cout << to_binary(a, true) << " / " << to_binary(b, true) << " = " << to_binary(c, true) << '\n';
	}
	else if (op == "sqrt") {
		alu_op = ALU_OPS::SQRT;
		c = ArithmeticLogicUnit(alu_op, a, b);
		std::cout << "sqrt(" << a << ") = " << c << '\n';
		std::cout << "sqrt(" << to_binary(a, true) << " = " << to_binary(c, true) << '\n';
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
