#pragma once
// test_suite.hpp: reusable test suite for small number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>

#include <universal/native/manipulators.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/test_case.hpp>
#include <universal/verification/test_formats.hpp>
#include <universal/verification/test_suite_exceptions.hpp>
#include <universal/verification/test_suite_conversion.hpp>
#include <universal/verification/test_suite_logic.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
// test_suite_random depends on a number systems math library
// so cannot be included here as this include needs to be used
// for number systems that do not have a math library.
//#include <universal/verification/test_suite_random.hpp>

namespace sw {
	namespace universal {

		// test triviality of an arithmetic type, trivially constructible, copyable, copy assignable
		template<typename TestType>
		void ReportTrivialityOfType() {
			std::string testType = sw::universal::type_tag(TestType());

			bool isTrivial = bool(std::is_trivial<TestType>());
			static_assert(std::is_trivial<TestType>(), " should be trivial but failed the assertion");
			std::cout << (isTrivial ? testType + std::string("  is trivial") : testType + std::string("  failed trivial: FAIL")) << '\n';

			bool isTriviallyConstructible = bool(std::is_trivially_constructible<TestType>());
			static_assert(std::is_trivially_constructible<TestType>(), " should be trivially constructible but failed the assertion");
			std::cout << (isTriviallyConstructible ? testType + std::string("  is trivial constructible") : testType + std::string("  failed trivial constructible: FAIL")) << '\n';

			bool isTriviallyCopyable = bool(std::is_trivially_copyable<TestType>());
			static_assert(std::is_trivially_copyable<TestType>(), " should be trivially copyable but failed the assertion");
			std::cout << (isTriviallyCopyable ? testType + std::string("  is trivially copyable") : testType + std::string("  failed trivially copyable: FAIL")) << '\n';

			bool isTriviallyCopyAssignable = bool(std::is_trivially_copy_assignable<TestType>());
			static_assert(std::is_trivially_copy_assignable<TestType>(), " should be trivially copy-assignable but failed the assertion");
			std::cout << (isTriviallyCopyAssignable ? testType + std::string("  is trivially copy-assignable") : testType + std::string("  failed trivially copy-assignable: FAIL")) << '\n';
		}

		// test the arithmetic operators on a test type
		template<typename TestType>
		void ArithmeticOperators(TestType a, TestType b) {
			using namespace sw::universal;
			TestType c;

			c = a + b;
			ReportBinaryOperation(a, "+", b, c);
			c = a - b;
			ReportBinaryOperation(a, "-", b, c);
			c = a * b;
			ReportBinaryOperation(a, "*", b, c);
			c = a / b;
			ReportBinaryOperation(a, "/", b, c);

			// negation
			ReportUnaryOperation(" -()", c, -c);

			// ULP manipulations through increment and decrement operators
			// This is Universal specific behavior of Real types.
			// In Universal, increment and decrement will operate on the encoding bits
			// and manipulate the unit in last position.

			// prefix operators
			a = 1;
			b = 1; --b;
			ReportUnaryOperation("--()", a, b);
			b = 1; ++b;
			ReportUnaryOperation("++()", a, b);

			// postfix operators
			a = 1;
			b = 1; b--;
			ReportUnaryOperation("()--", a, b);
			b = 1; ++b;
			ReportUnaryOperation("()++", a, b);
		}


		template<typename TestType>
		void ExtremeValues() {
			TestType tt; // uninitialized

			tt.maxpos();
			std::cout << type_tag(tt) << " maxpos : " << to_binary(tt) << " : " << tt << '\n';
			tt.minpos();
			std::cout << type_tag(tt) << " minpos : " << to_binary(tt) << " : " << tt << '\n';
			tt.zero();
			std::cout << type_tag(tt) << " zero   : " << to_binary(tt) << " : " << tt << '\n';
			tt.minneg();
			std::cout << type_tag(tt) << " minneg : " << to_binary(tt) << " : " << tt << '\n';
			tt.maxneg();
			std::cout << type_tag(tt) << " maxneg : " << to_binary(tt) << " : " << tt << '\n';
		}

	}
} // namespace sw::universal