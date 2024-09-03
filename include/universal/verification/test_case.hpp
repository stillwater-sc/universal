#pragma once
//  test_case.hpp : functions to generate specific test cases
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>

namespace sw { namespace universal {

	enum class TestCaseOperator { ADD, SUB, MUL, DIV };  // basic arithmetic operators supported by all number systems

	// generate an arithmetic test case
	template<typename Number, typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type >
	int TestCase(TestCaseOperator _operator, Real _a, Real _b) {
		Number a = _a;
		Number b = _b;
		return ExecuteTestCase(_a, a, _operator, _b, b);
	}

	// generate test case for binary string representations of the input values
	template<typename Number, typename Real>
	int TestCase(TestCaseOperator _operator, const std::string& aBits, const std::string& bBits) {
		Number a, b;
		a.assign(aBits);
		Real _a = Real(a);
		b.assign(bBits);
		Real _b = Real(b);
		return ExecuteTestCase(_a, a, _operator, _b, b);
	}

	template<typename Number, typename Real>
	int ExecuteTestCase(Real _a, const Number& a, const TestCaseOperator _operator, Real _b, const Number& b) {
		constexpr size_t nbits = Number::nbits;
		std::string op, opName;
		Number c;
		Real _c(0);
		std::cerr << to_binary(a) << " : " << a << " vs " << _a << '\n';
		std::cerr << to_binary(b) << " : " << b << " vs " << _b << '\n';
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
			return 1;
		}
		// sample the reference into the target Number to be the golden value
		Number reference = _c;

		auto oldprecision = std::cout.precision();
		std::cout << std::setprecision(10);
		std::cout << "+--------  Test Case: " << opName << "  ---------------------------------------------------\n";
		std::cout << "  input operands : " << type_tag(_a) << '\n';
		std::cout << std::setw(nbits) << _a << op << std::setw(nbits) << _b << " = " << std::setw(nbits) << _c << std::endl;
		std::cout << "a    " << to_binary(_a) << " : " << _a << '\n';
		std::cout << "b    " << to_binary(_b) << " : " << _b << '\n';
		std::cout << "c    " << to_binary(_c) << " : " << _c << '\n';
		std::cout << "+-------- Test Case:\n";
		std::cout << "  target type    : " << type_tag(a) << '\n';
		std::cout << std::setw(nbits) << a << op << std::setw(nbits) << b << " = " << std::setw(nbits) << c << " (reference: " << reference << ")\n";
		std::cout << "a    " << to_binary(a, true) << op << '\n';
		std::cout << "b    " << to_binary(b, true) << " =\n";
		std::cout << "c    " << to_binary(c, true) << '\n';
		std::cout << "ref  " << to_binary(reference, true) << "   ";
		std::cout << (reference == c ? "PASS" : "FAIL");
		std::cout << "\n+--------  Test Case: Done ---------------------------------------------------\n\n";
		std::cout << std::setprecision(oldprecision);
		return (reference == c) ? 0 : 1;  // return 1 to indicate 1 test failure
	}

	template<typename TestType>
	void TestArithmeticBinaryOperation(double da, double db, TestCaseOperator _operator) {
		std::string op;
		TestType a, b, c;
		double dc;
		a = da;
		b = db;
		switch (_operator) {
		case TestCaseOperator::ADD:
			c = a + b;
			dc = da + db;
			op = " + ";
			break;
		case TestCaseOperator::SUB:
			c = a - b;
			dc = da - db;
			op = " - ";
			break;
		case TestCaseOperator::MUL:
			c = a * b;
			dc = da * db;
			op = " * ";
			break;
		case TestCaseOperator::DIV:
			c = a / b;
			dc = da / db;
			op = " / ";
			break;
		default:
			std::cout << "Unknown operator: exiting\n";
			return;
		}
		ReportBinaryOperation(a, "/", b, c);
		TestType ref(dc);
		ReportBinaryOperation(a, "/", b, ref);
		if (c != ref) std::cout << "FAIL\n";
	}

	template<typename TestType>
	void ReportValue(const TestType& a, const std::string& label = "", unsigned labelWidth = 20, unsigned precision = 7) {
		auto defaultPrecision = std::cout.precision();
		std::cout << std::setprecision(precision);
		std::cout << std::setw(labelWidth) << label << " : " << to_binary(a, true) << " : " << a << '\n';
		std::cout << std::setprecision(defaultPrecision);
	}

	template<typename TestType>
	void ReportUnaryOperation(const std::string& op, const TestType& a, const TestType& c) {
		std::cout << op << ' ' << to_binary(a) << " -> " << to_binary(c) << '\n';
		std::cout << op << ' ' << a << " -> " << c << '\n';
	}

	template<typename TestType>
	void ReportBinaryOperation(const TestType& a, const std::string& op, const TestType& b, const TestType& c) {
		std::cout << to_binary(a) << ' ' << op << ' ' << to_binary(b) << " = " << to_binary(c) << '\n';
		std::cout << a << ' ' << op << ' ' << b << " = " << c << '\n';
	}

	template<typename TestType>
	void ReportBinaryOperationVertically(const TestType& a, const std::string& op, const TestType& b, const TestType& c, unsigned labelWidth = 20) {
		std::cout << std::setw(labelWidth) << "a" << " : " << to_binary(a) << " : " << a << '\n';
		std::cout << std::setw(labelWidth) << "b" << " : " << to_binary(b) << " : " << b << ' ' << op << '\n';
		std::cout << std::setw(labelWidth) << "c" << " : " << to_binary(c) << " : " << c << '\n';
	}

}} // namespace sw::universal
