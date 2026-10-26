// constexpr.cpp: compile-time tests for adaptive precision binary floating-point (efloat)
//
//   values      the default value, the state and sign selectors, clear() and setzero(), copies
//               and moves, as before (#747), and now nonzero constexpr variables: a value whose
//               limbs fit the inline buffer is a literal, usable in static_assert (#1819).
//   arithmetic  +, -, *, /, sqrt, the compound operators with efloat and integer operands, the
//               comparisons, and the integer and dyadic constructors, in static_assert.
//   agreement   a chain of operations evaluated by the compiler and at run time, at precisions
//               of one 32-bit limb to 2048 bits, on and past the inline limbs: the limbs, the
//               exponent and the sign of every result are the same (#1819). The kernels take
//               their portable loops in constant evaluation, the assembly at run time.
//   constants   sqrt(2) to 1024 bits by the compiler, bit for bit the run-time root.
//
// Constant evaluation rounds to nearest and raises no status flags: it cannot read or write the
// thread's rounding mode and flags. Out of scope still (#1819 phases 3 to 5): the math library
// past sqrt, conversion from and to float and double, parse(), and constexpr values past the
// inline limbs, whose heap block a constant expression cannot keep.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- the cases above, the chain at 32 to 1056 bits.
//   LEVEL 2 -- the chain at 2048 bits as well.
//   LEVEL 3 -- as level 2.
//   LEVEL 4 -- as level 3.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using sw::universal::efloat;

	// the state, sign, exponent and limbs of a sequence of values, for comparing a constant
	// evaluation with the run time
	template<std::size_t K>
	struct digest {
		std::array<std::uint64_t, K> word{};
		std::size_t n = 0;
		constexpr void put(std::uint64_t v) { if (n < K) word[n] = v; ++n; }
		template<unsigned nlimbs>
		constexpr void put(const efloat<nlimbs>& v) {
			put(static_cast<std::uint64_t>(v.scale()));
			put((v.iszero() ? 1u : 0u) | (v.isinf() ? 2u : 0u) | (v.isnan() ? 4u : 0u) | (v.sign() < 0 ? 8u : 0u));
			put(v.get_precision());
			put(v.limb_count());
			for (std::size_t i = 0; i < v.limb_count(); ++i) put(v.limb_data()[i]);
		}
		constexpr bool operator==(const digest&) const = default;
	};

	// sums, differences, products, quotients and roots of values that fill their precision, with
	// cancellations, a product of powers that leaves the inline limbs, integer operands, and the
	// special values
	template<unsigned nlimbs>
	constexpr digest<1024> chain(unsigned bits) {
		using E = efloat<nlimbs>;
		digest<1024> d;
		E three(3), seven(7);
		three.set_precision(bits);
		seven.set_precision(bits);
		const E x = E(1) / three;   // 1/3 at the precision
		const E y = E(22) / seven;  // 22/7
		d.put(x); d.put(y);
		d.put(x + y); d.put(x - y); d.put(y - x); d.put(x * y); d.put(x / y); d.put(y / x);
		d.put(x * x - x * x);       // an exact cancellation
		E p = y;
		for (int i = 0; i < 12; ++i) p = p * y + x;
		d.put(p);
		d.put(p / (x + y));
		d.put((x + E(1)) - E(1));   // the bits of x below 1's precision are kept
		d.put(p - (p - x));
		d.put(x * E(-12345678));
		d.put(sqrt(x)); d.put(sqrt(y)); d.put(sqrt(p)); d.put(sqrt(x * x));
		E w = y;
		w /= 7;           d.put(w);   // a one-limb divisor
		w += 5;           d.put(w);
		w *= -3;          d.put(w);
		w -= 1000000007;  d.put(w);
		w /= x;           d.put(w);
		d.put((x < y) ? 1u : 0u); d.put((x == x) ? 1u : 0u); d.put((p >= y) ? 1u : 0u); d.put((-x > x) ? 1u : 0u);
		E z(0);
		d.put(x / z);      // inf
		d.put(z / z);      // NaN
		d.put(-(z * x));   // -0 * x = -0
		return d;
	}

	template<unsigned nlimbs, unsigned bits>
	int VerifyConstantEvaluation(bool reportTestCases) {
		constexpr digest<1024> compiled = chain<nlimbs>(bits);
		static_assert(compiled.n <= 1024, "the digest holds every word");
		const digest<1024> run = chain<nlimbs>(bits);
		if (compiled == run) return 0;
		if (reportTestCases) std::cerr << "FAIL: efloat<" << nlimbs << "> at " << bits << " bits: constant evaluation differs from the run time\n";
		return 1;
	}

	// sqrt(2) at 1024 bits, its limbs least significant first
	consteval std::array<std::uint64_t, 16> sqrt2_limbs_compiled() {
		efloat<32> two(2);
		two.set_precision(1024);
		const efloat<32> r = sqrt(two);
		std::array<std::uint64_t, 16> limbs{};
		for (std::size_t i = 0; i < r.limb_count() && i < limbs.size(); ++i) limbs[i] = r.limb_data()[i];
		return limbs;
	}

	int VerifyConstants(bool reportTestCases) {
		constexpr std::array<std::uint64_t, 16> compiled = sqrt2_limbs_compiled();
		static_assert(compiled[15] == 0xB504'F333'F9DE'6484ull, "sqrt(2) = 1.0110101000001001111...");
		efloat<32> two(2);
		two.set_precision(1024);
		const efloat<32> r = sqrt(two);
		int failures = 0;
		if (r.limb_count() != 16) ++failures;
		for (std::size_t i = 0; failures == 0 && i < 16; ++i) {
			if (r.limb_data()[i] != compiled[i]) ++failures;
		}
		if (failures != 0 && reportTestCases) std::cerr << "FAIL: sqrt(2) at 1024 bits: constant evaluation differs from the run time\n";
		return failures;
	}

	// the selectors and the arithmetic in static_assert: each fails to compile when wrong
	int VerifyStaticAsserts() {
		using ef4 = efloat<4>;

		// the default value: zero, as before (#747)
		constexpr ef4 z{};
		static_assert( z.iszero() && !z.isnan() && !z.isinf(), "default is zero");
		static_assert(!z.ispos() && !z.isneg() && !z.isone(), "zero is neither positive nor negative");
		static_assert( z.sign() == 1 && z.scale() == 0, "default is +0");

		// clear() leaves a Normal value, setzero() zero again
		constexpr ef4 cleared = []() { ef4 t{}; t.clear(); return t; }();
		static_assert(!cleared.iszero() && cleared.ispos(), "clear(): Normal and positive");
		constexpr ef4 zeroed = []() { ef4 t{}; t.clear(); t.setzero(); return t; }();
		static_assert(zeroed.iszero(), "setzero(): zero");

		// nonzero constexpr values (#1819): the limbs live in the value
		constexpr ef4 three(3);
		constexpr ef4 four(4);
		constexpr ef4 seven = three + four;
		static_assert(seven == ef4(7) && seven.scale() == 2 && seven.ispos(), "3 + 4 = 7");
		static_assert(three - four == ef4(-1), "3 - 4 = -1");
		static_assert(three * four == ef4(12), "3 * 4 = 12");
		static_assert(ef4(12) / four == three, "12 / 4 = 3");
		static_assert(ef4(-12) / four == -three, "-12 / 4 = -3");
		static_assert(ef4(1).isone() && !ef4(2).isone(), "isone()");
		static_assert(four.scale() == 2 && ef4(1023).scale() == 9, "scale()");
		static_assert(ef4(-5).isneg() && ef4(-5).sign() == -1, "sign()");

		// a quotient that does not terminate, rounded to nearest at 128 bits
		constexpr ef4 third = ef4(1) / three;
		static_assert(third > ef4(0) && third < ef4(1) && third.scale() == -2, "1/3 in [1/4, 1/2)");
		static_assert(ef4(1) - third * three < ef4(1) / ef4(1'000'000'000), "1 - 3 (1/3) is tiny");
		static_assert(third + third + third <= ef4(1) + ef4(1) / ef4(1'000'000'000), "1/3 + 1/3 + 1/3 ~= 1");

		// comparisons
		static_assert(three < four && four > three && three <= three && four >= three, "<, >, <=, >=");
		static_assert(three != four && !(three == four), "==, !=");
		static_assert(-four < -three && -three < z && z < three, "ordering across zero");

		// compound operators with efloat and integer operands
		constexpr ef4 compound = []() {
			ef4 v(10);
			v += ef4(5);   // 15
			v -= 3;        // 12
			v *= 7;        // 84
			v /= ef4(4);   // 21
			v /= 3;        // 7
			return v;
		}();
		static_assert(compound == seven, "((10 + 5 - 3) 7 / 4) / 3 = 7");

		// square roots: exact and rounded
		static_assert(sqrt(ef4(4)) == ef4(2) && sqrt(ef4(1'000'000)) == ef4(1'000), "exact roots");
		constexpr ef4 root2 = sqrt(ef4(2));
		static_assert(root2 > ef4(1) && root2 < ef4(3) / ef4(2), "sqrt(2) in (1, 1.5)");
		static_assert(sqrt(ef4(-1)).isnan(), "sqrt(-1) is NaN");

		// the special values
		static_assert((ef4(1) / z).isinf() && (ef4(-1) / z).isneg() == false && (ef4(-1) / z).sign() == -1, "1/0 = inf, -1/0 = -inf");
		static_assert((z / z).isnan(), "0/0 is NaN");
		static_assert((-(z * three)).iszero() && (-(z * three)).sign() == -1, "-(0 3) = -0");

		// the dyadic constructor: 32-bit limbs, most significant first, the top bit weighing
		// 2^exponent; 0xC0000000 at 2^10 is 1.5 2^10
		constexpr ef4 dyadic = []() { return ef4(false, 10, std::vector<std::uint32_t>{ 0xC000'0000u }); }();
		static_assert(dyadic == ef4(1536), "1.5 * 2^10 = 1536");

		// a wider type: the same values
		using ef8 = efloat<8>;
		static_assert(ef8(3) + ef8(4) == ef8(7) && sw::universal::is_efloat<ef8> && !sw::universal::is_efloat<double>, "efloat<8>");
		return 0;
	}

}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat constexpr verification";
	std::string test_tag            = "constexpr";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyStaticAsserts(), "efloat", "static_assert");
	nrOfFailedTestCases += ReportTestResult(VerifyConstantEvaluation<4, 128>(reportTestCases), "efloat<4>", "constant evaluation at 128 bits");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyStaticAsserts(), "efloat", "static_assert");
	nrOfFailedTestCases += ReportTestResult(VerifyConstantEvaluation<1, 32>(reportTestCases), "efloat<1>", "constant evaluation at 32 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyConstantEvaluation<4, 96>(reportTestCases), "efloat<4>", "constant evaluation at 96 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyConstantEvaluation<4, 128>(reportTestCases), "efloat<4>", "constant evaluation at 128 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyConstantEvaluation<8, 160>(reportTestCases), "efloat<8>", "constant evaluation at 160 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyConstantEvaluation<8, 256>(reportTestCases), "efloat<8>", "constant evaluation at 256 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyConstantEvaluation<16, 480>(reportTestCases), "efloat<16>", "constant evaluation at 480 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyConstantEvaluation<16, 512>(reportTestCases), "efloat<16>", "constant evaluation at 512 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyConstantEvaluation<64, 1056>(reportTestCases), "efloat<64>", "constant evaluation at 1056 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyConstants(reportTestCases), "efloat<32>", "sqrt(2) at 1024 bits");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyConstantEvaluation<64, 2048>(reportTestCases), "efloat<64>", "constant evaluation at 2048 bits");
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
