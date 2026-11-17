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
	// than 53 digits, and products and quotients inside double's range) and the exact
	// reference otherwise. Asking for double where it cannot hold them is a compile error,
	// for every type that has an exact reference: posit (posit_rounding_oracle.hpp) and cfloat
	// (cfloat_rounding_oracle.hpp, #2123). A type without one (dd, qd, the cascades, lns) still
	// checks against double; giving it an exact_rounding is how it moves off.
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
	// than double, and the exponent range of every product and quotient of two operands inside
	// double's normal range, which is about twice the operands' range. posit<32,6> has 24 digits
	// but its operands reach 2^1920; posit<32,5> has operands inside double's range, but
	// minpos * minpos is 2^-1920, which double flushes to zero where the posit gives minpos.
	template<typename TestType>
	inline constexpr bool double_reference_holds =
		std::numeric_limits<TestType>::is_specialized &&
		std::numeric_limits<TestType>::digits < std::numeric_limits<double>::digits &&
		2LL * std::numeric_limits<TestType>::max_exponent <= std::numeric_limits<double>::max_exponent &&
		2LL * std::numeric_limits<TestType>::min_exponent >= std::numeric_limits<double>::min_exponent;

	template<typename TestType>
	using default_reference_t = std::conditional_t<has_exact_rounding<TestType> && !double_reference_holds<TestType>,
	                                               exact_reference, double>;

	// a reference type the verifiers accept for TestType
	template<typename TestType, typename ReferenceType>
	inline constexpr bool valid_reference_v =
		std::is_same_v<ReferenceType, exact_reference> ? has_exact_rounding<TestType> :
		std::is_same_v<ReferenceType, double> ? (double_reference_holds<TestType> || !has_exact_rounding<TestType>) :
		false;

	// a result agrees with the exact reference when they are equal, or both NaN: NaN != NaN, and
	// the reference does not model which NaN encoding an operation returns
	template<typename TestType>
	bool reference_equal(const TestType& result, const TestType& ref) {
		if constexpr (requires { result.isnan(); }) {
			if (result.isnan() && ref.isnan()) return true;
		}
		return result == ref;
	}

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
