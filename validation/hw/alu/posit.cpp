// posit.cpp: testbench for a posit hardware ALU
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>

#include <universal/number/posit/posit.hpp>

#include <universal/verification/test_status.hpp> // ReportTestResult

enum ALU_OPS {
	ALU_OPS_NOP,
	ALU_OPS_ADD,
	ALU_OPS_SUB,
	ALU_OPS_MUL,
	ALU_OPS_DIV,
	ALU_OPS_SQRT
};

template<size_t nbits, size_t es>
sw::universal::posit<nbits, es> ArithmeticLogicUnit(ALU_OPS op, const sw::universal::posit<nbits, es>& a, const sw::universal::posit<nbits, es>& b) {
	using namespace sw::universal;
	using Posit = posit<nbits, es>;

	Posit c;
	switch (op) {
	default:
	case ALU_OPS_NOP:
		break;
	case ALU_OPS_ADD:
		c = a + b;
		break;
	case ALU_OPS_SUB:
		c = a - b;
		break;
	case ALU_OPS_MUL:
		c = a * b;
		break;
	case ALU_OPS_DIV:
		c = a / b;
		break;
	case ALU_OPS_SQRT:
		c = sqrt(a);
		break;
	}
	return c;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc != 4) {
		std::cerr << "Usage: hw_alu_posit [add | sub | mul | div | sqrt] a b\n";
		std::cerr << "Example: hw_alu_posit add 1.5 -1.5\n";
		return EXIT_FAILURE;
	}
	string op = argv[1];
	float fa = atof(argv[2]);
	float fb = atof(argv[3]);
	cout << op << " " << fa << " and " << fb << '\n';

	posit<8, 2> a, b, c;
	a = fa;
	b = fb;

	// decode the operation
	ALU_OPS alu_op;
	if (op == "add") {
		alu_op = ALU_OPS_ADD;
		c = ArithmeticLogicUnit(alu_op, a, b);
		cout << a << " + " << b << " = " << c << '\n';
		cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(c) << '\n';
	}
	else if (op == "sub") {
		alu_op = ALU_OPS_SUB;
		c = ArithmeticLogicUnit(alu_op, a, b);
		cout << a << " - " << b << " = " << c << '\n';
		cout << to_binary(a) << " - " << to_binary(b) << " = " << to_binary(c) << '\n';
	}
	else if (op == "mul") {
		alu_op = ALU_OPS_MUL;
		c = ArithmeticLogicUnit(alu_op, a, b);
		cout << a << " * " << b << " = " << c << '\n';
		cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << '\n';
	}
	else if (op == "div") {
		alu_op = ALU_OPS_DIV;
		c = ArithmeticLogicUnit(alu_op, a, b);
		cout << a << " / " << b << " = " << c << '\n';
		cout << to_binary(a) << " / " << to_binary(b) << " = " << to_binary(c) << '\n';
	}
	else if (op == "sqrt") {
		alu_op = ALU_OPS_SQRT;
		c = ArithmeticLogicUnit(alu_op, a, b);
		cout << "sqrt(" << a << ") = " << c << '\n';
		cout << "sqrt(" << to_binary(a) << " = " << to_binary(c) << '\n';
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
