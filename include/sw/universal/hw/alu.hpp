// alu.hpp: a generic module to model a hardware ALU
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/shared/specific_value_encoding.hpp>

namespace sw { namespace universal {

enum class ALU_OPS {
	NOP,
	ADD,
	SUB,
	MUL,
	DIV,
	SQRT
};

template<typename NumberSystemType>
NumberSystemType ArithmeticLogicUnit(ALU_OPS op, const NumberSystemType& a, const NumberSystemType& b) {
	NumberSystemType c;
	switch (op) {
	default:
	case ALU_OPS::NOP:
		c = 0;
		break;
	case ALU_OPS::ADD:
		c = a + b;
		break;
	case ALU_OPS::SUB:
		c = a - b;
		break;
	case ALU_OPS::MUL:
		c = a * b;
		break;
	case ALU_OPS::DIV:
		c = a / b;
		break;
	case ALU_OPS::SQRT:
		c = sqrt(a);
		break;
	}
	return c;
}

template<typename Real>
void ExecuteOp(const std::string& op, float fa, float fb) {
	Real a, b, c;
	a = fa;
	b = fb;

	// decode the operation
	ALU_OPS alu_op;
	std::string opSymbol;
	if (op == "add") {
		alu_op = ALU_OPS::ADD;
		c = ArithmeticLogicUnit(alu_op, a, b);
		opSymbol = " + ";
	}
	else if (op == "sub") {
		alu_op = ALU_OPS::SUB;
		c = ArithmeticLogicUnit(alu_op, a, b);
		opSymbol = " - ";
	}
	else if (op == "mul") {
		alu_op = ALU_OPS::MUL;
		c = ArithmeticLogicUnit(alu_op, a, b);
		opSymbol = " * ";
	}
	else if (op == "div") {
		alu_op = ALU_OPS::DIV;
		c = ArithmeticLogicUnit(alu_op, a, b);
		opSymbol = " / ";
	}
	else if (op == "sqrt") {
		alu_op = ALU_OPS::SQRT;
		c = ArithmeticLogicUnit(alu_op, a, b);
		std::cout << "sqrt(" << a << ") = " << c << '\n';
		std::cout << "sqrt(" << to_binary(a, true) << " = " << to_binary(c, true) << '\n';
	}
	if (op != "sqrt") {
		std::cout << a << opSymbol << b << " = " << c << '\n';
		std::cout << to_binary(a, true) << opSymbol << to_binary(b, true) << " = " << to_binary(c, true) << '\n';
	}
}

template<typename NumberType>
void GenerateUnaryOpTestVectors(std::ostream& ostr, const std::string& op) {
	constexpr unsigned nbits = NumberType::nbits;
	constexpr unsigned NR_ENCODINGS = (1u << nbits);

	NumberType a, c;

	if (op == "sqrt") {
		for (unsigned i = 0; i < NR_ENCODINGS; ++i) {
			a.setbits(i);
			c = sqrt(a);
			ostr << "sqrt(" << to_binary(a, true) << " = " << to_binary(c, true) << '\n';
		}
	}
}

template<typename NumberType>
void GenerateBinaryOpTestVectors(std::ostream& ostr, const std::string& op) {
	//constexpr unsigned nbits = NumberType::nbits;
	NumberType maxneg(SpecificValue::maxneg), maxpos(SpecificValue::maxpos);
	NumberType a, b, c;

	if (op == "add") {
		a = maxneg;
		while (a <= maxpos) {
			b = maxneg;
			while (b <= maxpos) {
				c = a + b;
				ostr << to_binary(a, true) << " + " << to_binary(b, true) << " = " << to_binary(c, true) << " : " << c << '\n';
				++b;
			}
			++a;
		}
	}
	else if (op == "sub") {
		a = maxneg;
		while (a <= maxpos) {
			b = maxneg;
			while (b <= maxpos) {
				c = a - b;
				ostr << to_binary(a, true) << " - " << to_binary(b, true) << " = " << to_binary(c, true) << '\n';
			}
		}
	}
	else if (op == "mul") {
		a = maxneg;
		while (a <= maxpos) {
			b = maxneg;
			while (b <= maxpos) {
				c = a * b;
				ostr << to_binary(a, true) << " * " << to_binary(b, true) << " = " << to_binary(c, true) << '\n';
			}
		}
	}
	else if (op == "add") {
		a = maxneg;
		while (a <= maxpos) {
			b = maxneg;
			while (b <= maxpos) {
				c = a / b;
				ostr << to_binary(a, true) << " / " << to_binary(b, true) << " = " << to_binary(c, true) << '\n';
			}
		}
	}
}

}} // namespace sw::universal
