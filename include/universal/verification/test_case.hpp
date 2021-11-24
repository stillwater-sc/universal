#pragma once
//  test_case.cpp : functions to generate specific test cases
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>

namespace sw::universal {

	enum class TestCaseOperator { ADD, SUB, MUL, DIV };  // basic arithmetic operators supported by all number systems

	// generate an arithmetic test case
	template<typename Number, typename Ty>
	void TestCase(TestCaseOperator _operator, Ty _a, Ty _b) {
		constexpr size_t nbits = Number::nbits;
		Number c(0);
		Ty _c(0);
		std::string op, opName;
		Number a = _a;
		Number b = _b;
		switch (_operator) {
		case TestCaseOperator::ADD:
			c = a + b;
			_c = _a + _b;
			op     = " + ";
			opName = "ADD";
			break;
		case TestCaseOperator::SUB:
			c = a - b;
			_c = _a - _b;
			op = " - ";
			opName = "SUB";
			break;
		case TestCaseOperator::MUL:
			c = a * b;
			_c = _a * _b;
			op = " * ";
			opName = "MUL";
			break;
		case TestCaseOperator::DIV:
			c = a / b;
			_c = _a / _b;
			op = " / ";
			opName = "DIV";
			break;
		default:
			std::cout << "Unknown operator: exiting\n";
			return;
		}
		// sample the reference into the target Number to be the golden value
		Number reference = _c;

		auto oldprecision = std::cout.precision();
		std::cout << std::setprecision(10);
		std::cout << "+--------  Test Case: " << opName << "\ninput operands : " << typeid(Ty).name() << '\n';
		std::cout << std::setw(nbits) << _a << op << std::setw(nbits) << _b << " = " << std::setw(nbits) << _c << std::endl;
		std::cout << to_binary(_a) << " : " << _a << '\n';
		std::cout << to_binary(_b) << " : " << _b << '\n';
		std::cout << to_binary(_c) << " : " << _c << '\n';
		std::cout << "+--------\ntarget number : " << typeid(Number).name() << '\n';
		std::cout << a << op << b << " = " << c << " (reference: " << reference << ")\n";
		std::cout << to_binary(a, true) << op << to_binary(b, true) << " = " << to_binary(c, true) << " (reference: " << to_binary(reference, true) << ")   ";
		std::cout << (reference == c ? "PASS" : "FAIL");
		std::cout << "\n+--------  Test Case: Done\n";
		std::cout << std::setprecision(oldprecision);
	}

} // namespace sw::universal
