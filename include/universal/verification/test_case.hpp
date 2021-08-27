#pragma once
//  test_case.cpp : functions to generate specific test cases
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>

namespace sw::universal {

	// generate specific test case that you can trace with the trace conditions in cfloat.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
	template<typename Number, typename Ty>
	void TestCaseAdd(Ty _a, Ty _b) {
		constexpr size_t nbits = Number::nbits;
		Number a, b, sum, ref;
		a = _a;
		b = _b;
		sum = a + b;
		// generate the reference
		Ty reference = _a + _b;
		ref = reference;

		std::cout << std::setprecision(10);
		//	std::cout << "native: " << std::setw(nbits) << _a << " + " << std::setw(nbits) << _b << " = " << std::setw(nbits) << reference << std::endl;
		Ty _c{ reference };
		std::cout << sw::universal::to_binary(_a) << " : " << _a << '\n';
		std::cout << sw::universal::to_binary(_b) << " : " << _b << '\n';
		std::cout << sw::universal::to_binary(_c) << " : " << _c << '\n';
		std::cout << a << " + " << b << " = " << sum << " (reference: " << ref << ")   ";
		std::cout << to_binary(a, true) << " + " << to_binary(b, true) << " = " << to_binary(sum, true) << " (reference: " << to_binary(ref, true) << ")   ";
		std::cout << (ref == sum ? "PASS" : "FAIL") << std::endl << std::endl;
		std::cout << std::setprecision(5);
	}

	// generate specific test case that you can trace with the trace conditions in cfloat.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
	template<typename Number, typename Ty>
	void TestCaseSub(Ty _a, Ty _b) {
		constexpr size_t nbits = Number::nbits;
		Number a, b, diff, ref;
		a = _a;
		b = _b;
		diff = a - b;
		// generate the reference
		Ty reference = _a - _b;
		ref = reference;

		std::cout << std::setprecision(10);
		//	std::cout << "native: " << std::setw(nbits) << _a << " + " << std::setw(nbits) << _b << " = " << std::setw(nbits) << reference << std::endl;
		Ty _c{ reference };
		std::cout << sw::universal::to_binary(_a) << " : " << _a << '\n';
		std::cout << sw::universal::to_binary(_b) << " : " << _b << '\n';
		std::cout << sw::universal::to_binary(_c) << " : " << _c << '\n';
		std::cout << a << " - " << b << " = " << diff << " (reference: " << ref << ")   ";
		std::cout << to_binary(a, true) << " - " << to_binary(b, true) << " = " << to_binary(diff, true) << " (reference: " << to_binary(ref, true) << ")   ";
		std::cout << (ref == diff ? "PASS" : "FAIL") << std::endl << std::endl;
		std::cout << std::setprecision(5);
	}

	// generate specific test case that you can trace with the trace conditions in cfloat.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
	template<typename Number, typename Ty>
	void TestCaseMul(Ty _a, Ty _b) {
		Number a, b, diff, ref;
		a = _a;
		b = _b;
		diff = a * b;
		// generate the reference
		Ty reference = _a * _b;
		ref = reference;

		std::cout << std::setprecision(10);
		//	constexpr size_t WIDTH = 10;
		//	std::cout << "native: " << std::setw(WIDTH) << _a << " * " << std::setw(WIDTH) << _b << " = " << std::setw(WIDTH) << reference << std::endl;
		Ty _c{ reference };
		std::cout << sw::universal::to_binary(_a) << " : " << _a << '\n';
		std::cout << sw::universal::to_binary(_b) << " : " << _b << '\n';
		std::cout << sw::universal::to_binary(_c) << " : " << _c << '\n';
		std::cout << a << " * " << b << " = " << diff << " (reference: " << ref << ")   ";
		std::cout << to_binary(a, true) << " * " << to_binary(b, true) << " = " << to_binary(diff, true) << " (reference: " << to_binary(ref, true) << ")   ";
		std::cout << (ref == diff ? "PASS" : "FAIL") << std::endl << std::endl;
		std::cout << std::setprecision(5);
	}
} // namespace sw::universal
