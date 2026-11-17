#pragma once
// exact_reference.hpp : the reference type of the random-operand verifiers (#2121)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <concepts>
#include <limits>
#include <type_traits>

namespace sw { namespace universal {

	// The random-operand verifiers (test_suite_randoms.hpp, posit_test_suite_randoms.hpp) check
	// an operator against a reference: a value computed in a reference type and rounded once to
	// the type under test. The reference type is a template parameter of the verifier:
	//
	//   double           the operands convert to double, the operation runs in double, and the
	//                    result is rounded once to TestType (correctly_rounded_reference.hpp,
	//                    #1861). Correct while TestType has fewer than 53 digits: then every
	//                    operand is a double and double's result says on which side of a
	//                    TestType midpoint the exact result lies. Past that, double's operands
	//                    are already rounded and the reference has fewer bits than the operator
	//                    it checks: posit<64,2> division failed against it at regression level 4.
	//
	//   exact_reference  the operands decode exactly into dyadic rationals, the operation is
	//                    exact (a quotient or a square root is compared, never computed), and
	//                    the result is rounded once by TestType's own rounding rule, read off its
	//                    encoding. It holds every bit any operator can produce, at any width.
	//                    A type provides it by specializing exact_rounding<TestType>.
	//
	// default_reference_t<TestType> picks double where double holds TestType's results (fewer
	// than 53 digits, and a range inside double's) and the exact reference otherwise. Asking for double where it cannot hold them is a compile error,
	// for every type that has an exact reference. A type without one (dd, qd, the cascades, lns,
	// cfloat past 53 digits) still checks against double; giving it an exact_rounding is how it
	// moves off.
	struct exact_reference {};

	// The customization point. A specialization provides
	//   static bool binary(char op, const TestType& a, const TestType& b, TestType& ref);
	//     ref = a op b rounded once, for op one of '+', '-', '*', '/'; false for any other op
	//   static bool sqrt(const TestType& a, TestType& ref);
	//     ref = sqrt(a) rounded once
	template<typename TestType>
	struct exact_rounding {};

	template<typename TestType>
	concept has_exact_rounding = requires(const TestType& a, TestType& ref) {
		{ exact_rounding<TestType>::binary('+', a, a, ref) } -> std::same_as<bool>;
		{ exact_rounding<TestType>::sqrt(a, ref) } -> std::same_as<bool>;
	};

	// true when double holds every result of TestType's operators, rounded once: fewer digits
	// than double, and an exponent range inside double's normal range. posit<32,6> has 24 digits
	// but reaches 2^1920; its operands past double's range become inf or zero there.
	template<typename TestType>
	inline constexpr bool double_reference_holds =
		std::numeric_limits<TestType>::is_specialized &&
		std::numeric_limits<TestType>::digits < std::numeric_limits<double>::digits &&
		std::numeric_limits<TestType>::max_exponent <= std::numeric_limits<double>::max_exponent &&
		std::numeric_limits<TestType>::min_exponent >= std::numeric_limits<double>::min_exponent;

	template<typename TestType>
	using default_reference_t = std::conditional_t<has_exact_rounding<TestType> && !double_reference_holds<TestType>,
	                                               exact_reference, double>;

	// a reference type the verifiers accept for TestType
	template<typename TestType, typename ReferenceType>
	inline constexpr bool valid_reference_v =
		std::is_same_v<ReferenceType, exact_reference> ? has_exact_rounding<TestType> :
		std::is_same_v<ReferenceType, double> ? (double_reference_holds<TestType> || !has_exact_rounding<TestType>) :
		false;

	template<typename TestType, typename ReferenceType>
	constexpr void check_reference_type() {
		static_assert(std::is_same_v<ReferenceType, double> || std::is_same_v<ReferenceType, exact_reference>,
		              "the reference type is double or exact_reference");
		static_assert(!std::is_same_v<ReferenceType, exact_reference> || has_exact_rounding<TestType>,
		              "exact_reference needs an exact_rounding<TestType> specialization");
		static_assert(!std::is_same_v<ReferenceType, double> || valid_reference_v<TestType, double>,
		              "double cannot hold the results of this type's operators: use exact_reference");
	}

}} // namespace sw::universal
