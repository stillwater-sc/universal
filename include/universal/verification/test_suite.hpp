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
		void LogicalOperators(TestType a, TestType b) {
			bool c;

			// comparison operators
			c = a == b;
			ReportComparisonOperation(a, "==", b, c);
			c = a != b;
			ReportComparisonOperation(a, "!=", b, c);
			c = a <= b;
			ReportComparisonOperation(a, "<=", b, c);
			c = a < b;
			ReportComparisonOperation(a, "<", b, c);
			c = a >= b;
			ReportComparisonOperation(a, ">=", b, c);
			c = a > b;
			ReportComparisonOperation(a, ">", b, c);

			c = a <= a;
			ReportComparisonOperation(a, "<=", a, c);
			c = a < a;
			ReportComparisonOperation(a, "<", a, c);
			c = a >= a;
			ReportComparisonOperation(a, ">=", a, c);
			c = a > a;
			ReportComparisonOperation(a, ">", a, c);
		}

		template<typename TestType>
		void ExplicitConversions(TestType a) {
			std::cout << "Explicit conversions for " << type_tag(a) << '\n';
			std::cout << to_binary(a) << " : " << a << '\n';
			uint8_t u8 = char(a); // conversion to uint8_t
			std::cout << "uint8_t  conversion : " << to_binary(u8) << " : " << u8 << '\n';
			uint16_t u16 = (unsigned short)(a); // conversion to uint16_t
			std::cout << "uint16_t conversion : " << to_binary(u16) << " : " << u16 << '\n';
			uint32_t u32 = (unsigned long)(a); // conversion to uint32_t
			std::cout << "uint32_t conversion : " << to_binary(u32) << " : " << u32 << '\n';
			uint64_t u64 = (unsigned long long)(a); // conversion to uint64_t
			std::cout << "uint64_t conversion : " << to_binary(u64) << " : " << u64 << '\n';
			int8_t i8 = (signed char)(a); // conversion to int8_t
			std::cout << "int8_t   conversion : " << to_binary(i8) << " : " << i8 << '\n';
			int16_t i16 = short(a); // conversion to int16_t
			std::cout << "int16_t  conversion : " << to_binary(i16) << " : " << i16 << '\n';
			int32_t i32 = long(a); // conversion to int32_t
			std::cout << "int32_t  conversion : " << to_binary(i32) << " : " << i32 << '\n';
			int64_t i64 = (long long)(a); // conversion to int64_t
			std::cout << "int64_t  conversion : " << to_binary(i64) << " : " << i64 << '\n';
			float f = float(a); // conversion to float
			std::cout << "float    conversion : " << to_binary(f) << " : " << f << '\n';
			double d = double(a); // conversion to double
			std::cout << "double   conversion : " << to_binary(d) << " : " << d << '\n';
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