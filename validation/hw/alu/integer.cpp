// integer.cpp: testbench for an integer hardware ALU
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/hw/alu.hpp>
#include <universal/number/integer/integer.hpp>

#include <universal/verification/test_status.hpp> // ReportTestResult

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc != 4) {
		std::cerr << "Usage: hw_int [add | sub | mul | div | sqrt] a b\n";
		std::cerr << "Example: hw_int add 1 -1\n";
		return EXIT_SUCCESS;  // needed for regression test success
	}
	std::string op = argv[1];
	int ia = atoi(argv[2]);
	int ib = atoi(argv[3]);
	std::cout << op << " " << ia << " and " << ib << '\n';

	integer<8, std::uint8_t, IntegerNumberType::IntegerNumber> a, b, c;
	a = ia;
	b = ib;

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
