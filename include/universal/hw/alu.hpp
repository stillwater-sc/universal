// alu.hpp: a generic module to model a hardware ALU
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

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

}} // namespace sw::universal