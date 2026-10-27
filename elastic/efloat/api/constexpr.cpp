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
//   constants   sqrt(2) to 1024 bits by the compiler, bit for bit the run-time root, and pi, e
//               and phi to 1024 and 2048 bits: a Chudnovsky series, the series of 1/k! and
//               (1 + sqrt(5)) / 2, each rounded once, limb for limb the run-time constants.
//
// Constant evaluation rounds to nearest and raises no status flags: it cannot read or write the
// thread's rounding mode and flags. The math library and the float and double conversions are
// verified in elastic/efloat/math/constexpr_mathlib.cpp (#1819 phase 3). Out of scope still
// (phases 4 and 5): parse(), and constexpr values past the inline limbs, whose heap block a
// constant expression cannot keep.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- the cases above, the chain at 32 to 1056 bits, pi, e and phi at 1024 bits.
//   LEVEL 2 -- the chain, pi, e and phi at 2048 bits as well.
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

	// pi, e and phi by the compiler. Each is evaluated with 64 guard bits, which absorb the
	// rounding errors of a series, and then rounded once to the target. The run-time constants
	// parse 1000-digit literals, about 3300 bits, so 2048 bits is the widest comparison.
	template<unsigned bits> constexpr unsigned work_bits = bits + 64;
	template<unsigned bits> using work_t = efloat<work_bits<bits> / 32 + 1>;
	template<unsigned bits> constexpr int last_scale = -int(work_bits<bits>) - 8;   // terms below are noise

	// Chudnovsky: 1/pi = 12 / 640320^(3/2) sum_k a_k (13591409 + 545140134 k), with
	// a_k / a_(k-1) = -(6k-5)(2k-1)(6k-1) / (k^3 640320^3 / 24). Each term adds about 14 digits,
	// so 2048 bits takes 45 terms, where Machin's formula takes about 590 and exceeds clang's
	// default constexpr step limit.
	template<unsigned bits>
	constexpr work_t<bits> pi_chudnovsky() {
		work_t<bits> a(1);
		a.set_precision(work_bits<bits>);
		work_t<bits> sum = a;
		sum *= 13591409;
		for (long long k = 1; a.scale() > last_scale<bits>; ++k) {
			a *= -(6 * k - 5) * (2 * k - 1) * (6 * k - 1);
			a /= k * k * k;
			a /= 10939058860032000ll;   // 640320^3 / 24
			work_t<bits> t = a;
			t *= 13591409 + 545140134 * k;
			sum += t;
		}
		work_t<bits> c(10005);
		c.set_precision(work_bits<bits>);
		work_t<bits> root = sqrt(c);   // 640320^(3/2) / 12 = 426880 sqrt(10005)
		root *= 426880;
		return root / sum;
	}

	// e = sum 1/k!
	template<unsigned bits>
	constexpr work_t<bits> e_series() {
		work_t<bits> term(1);
		term.set_precision(work_bits<bits>);
		work_t<bits> sum = term;
		for (int k = 1; term.scale() > last_scale<bits>; ++k) {
			term /= k;
			sum += term;
		}
		return sum;
	}

	// phi = (1 + sqrt(5)) / 2
	template<unsigned bits>
	constexpr work_t<bits> phi_root() {
		work_t<bits> five(5);
		five.set_precision(work_bits<bits>);
		work_t<bits> phi = sqrt(five);
		phi += 1;
		phi /= 2;
		return phi;
	}

	// a constant rounded to bits: its sign, scale and limbs, least significant first
	template<unsigned bits>
	struct rounded_constant {
		std::array<std::uint64_t, bits / 64> limb{};
		int  scale    = 0;
		bool negative = false;
	};

	template<unsigned bits>
	consteval rounded_constant<bits> round_constant(work_t<bits> (*constant)()) {
		work_t<bits> v = constant();
		v.set_precision(bits);
		rounded_constant<bits> r;
		for (std::size_t i = 0; i < v.limb_count() && i < r.limb.size(); ++i) r.limb[i] = v.limb_data()[i];
		r.scale    = v.scale();
		r.negative = v.sign() < 0;
		return r;
	}

	template<unsigned bits, unsigned nlimbs>
	int CompareConstant(const char* name, const rounded_constant<bits>& compiled, const efloat<nlimbs>& run, bool reportTestCases) {
		constexpr std::size_t K = bits / 64;
		int failures = (run.limb_count() != K || run.scale() != compiled.scale || (run.sign() < 0) != compiled.negative) ? 1 : 0;
		for (std::size_t i = 0; failures == 0 && i < K; ++i) {
			if (run.limb_data()[i] != compiled.limb[i]) ++failures;
		}
		if (failures != 0 && reportTestCases) std::cerr << "FAIL: " << name << " at " << bits << " bits: constant evaluation differs from the run time\n";
		return failures;
	}

	template<unsigned bits>
	int VerifyMathConstants(bool reportTestCases) {
		constexpr rounded_constant<bits> pi  = round_constant<bits>(pi_chudnovsky<bits>);
		constexpr rounded_constant<bits> e   = round_constant<bits>(e_series<bits>);
		constexpr rounded_constant<bits> phi = round_constant<bits>(phi_root<bits>);
		constexpr std::size_t top = bits / 64 - 1;
		static_assert(pi.limb[top]  == 0xC90F'DAA2'2168'C234ull && pi.scale == 1 && !pi.negative, "pi = 11.0010010000111111011...");
		static_assert(e.limb[top]   == 0xADF8'5458'A2BB'4A9Aull && e.scale == 1 && !e.negative, "e = 10.1011011111100001010...");
		static_assert(phi.limb[top] == 0xCF1B'BCDC'BFA5'3E0Aull && phi.scale == 0 && !phi.negative, "phi = 1.1001111000110111011...");
		constexpr unsigned n = bits / 32;
		int failures = 0;
		failures += CompareConstant("pi", pi, sw::universal::efloat_pi<n>(), reportTestCases);
		failures += CompareConstant("e", e, sw::universal::efloat_e<n>(), reportTestCases);
		failures += CompareConstant("phi", phi, sw::universal::efloat_phi<n>(), reportTestCases);
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
	nrOfFailedTestCases += ReportTestResult(VerifyMathConstants<1024>(reportTestCases), "efloat", "pi, e and phi at 1024 bits");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyConstantEvaluation<64, 2048>(reportTestCases), "efloat<64>", "constant evaluation at 2048 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyMathConstants<2048>(reportTestCases), "efloat", "pi, e and phi at 2048 bits");
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
