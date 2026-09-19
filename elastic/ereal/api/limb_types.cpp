// limb_types.cpp: ereal<maxlimbs, FpType> on float, double and long double limbs
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// ereal's limb is a template parameter (#1564, part 2 of #1355), defaulted to double so
// ereal<8> keeps its meaning. This suite checks the core on every limb type the host has:
//
//   - the derived limits: max_safe_limbs is -(min_exponent - 1) / digits of the limb
//   - conversions are exact: integers of every width, and floating-point values wider
//     than one limb (a double into float limbs, an x87 long double into double limbs)
//   - sums, differences and products are exact, and quotients carry the precision the
//     limbs allow -- all checked against an exact dyadic oracle, never through a double
//   - special values, overflow, and to_string
//   - and the point of the exercise: with long double limbs, precision past the 303
//     digits double limbs are capped at
//
// long double is x87 extended on x86-64, binary128 on aarch64 and riscv64; its cases run
// whenever it is a valid limb (not on ppc64le's default IBM double-double).
#include <universal/utility/directives.hpp>
#include <climits>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	template<unsigned N, typename F>
	dyadic value_of(const ereal<N, F>& x) {
		dyadic acc;
		for (F c : x.limbs()) acc = acc + dyadic::from_fp(c);
		return acc;
	}
	dyadic from_integer(long long v) {
		// exact: split into two 32-bit halves, each exact in a double
		const bool negative = v < 0;
		const unsigned long long u = negative ? 0ull - static_cast<unsigned long long>(v) : static_cast<unsigned long long>(v);
		dyadic d = dyadic::from_fp(static_cast<double>(u >> 32)) * dyadic::from_fp(4294967296.0) + dyadic::from_fp(static_cast<double>(u & 0xFFFFFFFFull));
		return negative ? -d : d;
	}
	dyadic from_unsigned(unsigned long long u) {
		return dyadic::from_fp(static_cast<double>(u >> 32)) * dyadic::from_fp(4294967296.0) + dyadic::from_fp(static_cast<double>(u & 0xFFFFFFFFull));
	}
	// floor(log2 |d|); d must be non-zero
	int log2_abs(const dyadic& d) {
		auto n = d.numerator;
		if (n.sign()) n.setsign(false);
		int bits = 0;
		while (!n.iszero()) { n >>= 1; ++bits; }
		return bits - 1 + d.scale;
	}
	// relative agreement of x with y, in bits
	int agreed_bits(const dyadic& x, const dyadic& y) {
		const dyadic diff = x - y;
		if (diff.iszero()) return 100000;
		return log2_abs(y) - log2_abs(diff);
	}

	int expect(bool ok, const std::string& what, bool reportTestCases) {
		if (ok) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}

	// ---- the limits follow the limb type -----------------------------------------------

	// Only instantiated where long double is a limb: naming ereal<N, long double> anywhere
	// else -- even inside an if constexpr in a non-template function -- instantiates the
	// class, and its static_assert rightly refuses IBM double-double on ppc64le.
	template<typename LD>
	int VerifyLongDoubleLimits(bool reportTestCases) {
		constexpr unsigned expected = static_cast<unsigned>((-(std::numeric_limits<LD>::min_exponent - 1)) / std::numeric_limits<LD>::digits);
		return expect(ereal<8, LD>::max_safe_limbs == expected, "long double max_safe_limbs", reportTestCases);
	}

	int VerifyLimits(bool reportTestCases) {
		int fails = 0;
		static_assert(ereal<8>::max_safe_limbs == 19, "double limbs: 1022 / 53");
		static_assert(ereal<5, float>::max_safe_limbs == 5, "float limbs: 126 / 24");
		static_assert(std::is_same_v<ereal<8>, ereal<8, double>>, "ereal<8> still means double limbs");
		static_assert(ereal<8>::EXP_BIAS == 1023 && ereal<8>::MAX_EXP == 1024 && ereal<8>::MIN_EXP_NORMAL == -1022,
		              "double's constants, as they were written before");
		if constexpr (is_expansion_limb_v<long double>) fails += VerifyLongDoubleLimits<long double>(reportTestCases);
		return fails;
	}

	// ---- conversions are exact ---------------------------------------------------------

	template<unsigned N, typename F>
	int VerifyConversions(const std::string& name, bool reportTestCases) {
		using R = ereal<N, F>;
		int fails = 0;
		for (long long v : { 0LL, 1LL, -7LL, 16777217LL, 9007199254740993LL, 123456789012345678LL, LLONG_MAX, LLONG_MIN }) {
			fails += expect(value_of(R(v)) == from_integer(v), name + ": integer " + std::to_string(v) + " converts exactly", reportTestCases);
		}
		for (unsigned long long u : { 4294967297ULL, 18446744073709551615ULL }) {
			fails += expect(value_of(R(u)) == from_unsigned(u), name + ": unsigned " + std::to_string(u) + " converts exactly", reportTestCases);
		}
		// doubles that need more than one float limb, and one that does not: within the limb
		// type's range, the conversion keeps every bit of the double
		for (double d : { 0.1, 1.0 / 3.0, -1.0e-10, 3.0 }) {
			fails += expect(value_of(R(d)) == dyadic::from_fp(d), name + ": double " + std::to_string(d) + " converts exactly", reportTestCases);
		}
		if constexpr (is_expansion_limb_v<long double> && std::numeric_limits<long double>::digits > 53) {
			// a long double with more bits than a double: ereal<N> used to round it to one
			// double; it is an expansion of the limbs now
			const long double ld = 1.0L / 3.0L;
			if constexpr (std::numeric_limits<F>::max_exponent >= 1024) {
				fails += expect(value_of(R(ld)) == dyadic::from_fp(ld), name + ": long double 1/3 converts exactly", reportTestCases);
			}
		}
		// back to double: the nearest double to the value
		const R third = R(1.0) / R(3.0);
		fails += expect(double(third) == 1.0 / 3.0, name + ": (1/3) converts back to the nearest double", reportTestCases);
		// signed zero survives
		fails += expect(R(-0.0).signbit() && R(-0.0).iszero(), name + ": -0 keeps its sign", reportTestCases);
		return fails;
	}

	// ---- arithmetic is exact where it can be, and precise where it cannot ---------------

	template<unsigned N, typename F>
	int VerifyArithmetic(const std::string& name, int cases, bool reportTestCases) {
		using R = ereal<N, F>;
		int fails = 0;
		std::mt19937_64 rng(1564);
		std::uniform_real_distribution<double> mant(1.0, 2.0);
		std::uniform_int_distribution<int> ex(-8, 8), sg(0, 1);
		constexpr int p = std::numeric_limits<F>::digits;
		// a random value at the limb type's own precision (a double rounded to F, or exact in F)
		auto rnd = [&]() { double v = std::ldexp(mant(rng), ex(rng)); return R(static_cast<F>(sg(rng) ? -v : v)); };
		// Operands of two or three limbs: a value plus independent tails 3p/4 and p binades
		// down. The exact product of two such operands must stay above the limb type's
		// smallest subnormal, or no error-free transformation can hold it: for float limbs
		// that bounds the span (wider operands fail in expansion_product itself, #1568).
		const R t1(std::ldexp(1.0, -(3 * p) / 4)), t2(std::ldexp(1.0, -p));
		for (int i = 0; i < cases; ++i) {
			R a = rnd() + rnd() * t1;
			R b = rnd() - rnd() * t2;
			const dyadic A = value_of(a), B = value_of(b);
			fails += expect(value_of(a + b) == A + B, name + ": sum is exact", reportTestCases && fails < 5);
			fails += expect(value_of(a - b) == A - B, name + ": difference is exact", reportTestCases && fails < 5);
			fails += expect(value_of(a * b) == A * B, name + ": product is exact", reportTestCases && fails < 5);
			// the quotient: q * b must give a back to about the precision of N limbs
			const R q = a / b;
			const int bits = agreed_bits(value_of(q) * B, A);
			const int wanted = static_cast<int>(N) * p - 2 * p;
			fails += expect(bits >= wanted, name + ": quotient carries " + std::to_string(bits) + " bits, wanted " + std::to_string(wanted), reportTestCases && fails < 5);
			// comparisons agree with the exact values
			fails += expect((a < b) == (value_of(b - a).numerator.sign() == false && !(A == B)), name + ": a < b agrees with the exact difference", reportTestCases && fails < 5);
		}
		return fails;
	}

	// ---- special values and the limb type's range ---------------------------------------

	template<unsigned N, typename F>
	int VerifySpecialValues(const std::string& name, bool reportTestCases) {
		using R = ereal<N, F>;
		int fails = 0;
		const R maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos), maxneg(SpecificValue::maxneg);
		fails += expect(maxpos[0] == std::numeric_limits<F>::max(), name + ": maxpos leads with the limb type's max", reportTestCases);
		fails += expect(minpos[0] == std::numeric_limits<F>::min(), name + ": minpos is the limb type's smallest normal", reportTestCases);
		fails += expect(value_of(maxneg) == -value_of(maxpos), name + ": maxneg is -maxpos", reportTestCases);
		bool nonoverlapping = true;
		for (std::size_t i = 1; i < maxpos.limbs().size(); ++i) {
			nonoverlapping = nonoverlapping && std::ilogb(maxpos[i]) < std::ilogb(maxpos[i - 1]) - std::numeric_limits<F>::digits;
		}
		fails += expect(nonoverlapping, name + ": maxpos is a non-overlapping expansion", reportTestCases);
		// overflow and the IEEE special values
		const R top(std::numeric_limits<F>::max());
		const R overflow = top + top;
		fails += expect(overflow.isinf() && !overflow.signbit() && overflow.limbs().size() == 1, name + ": max + max is +inf", reportTestCases);
		fails += expect((R(1.0) / R(0.0)).isinf() && (R(0.0) / R(0.0)).isnan(), name + ": 1/0 is inf, 0/0 is nan", reportTestCases);
		fails += expect(R(SpecificValue::infneg).isinf() && R(SpecificValue::qnan).isnan(), name + ": special-value constructors", reportTestCases);
		return fails;
	}

	// ---- to_string -----------------------------------------------------------------------

	template<unsigned N, typename F>
	int VerifyToString(const std::string& name, int digits, bool reportTestCases) {
		using R = ereal<N, F>;
		int fails = 0;
		// 1/3 to the requested number of digits: "3.333...3e-01"
		const std::string s = (R(1.0) / R(3.0)).to_string(digits);
		const std::string expected = "3." + std::string(static_cast<std::size_t>(digits), '3') + "e-01";
		fails += expect(s == expected, name + ": 1/3 prints as " + expected.substr(0, 12) + "... to " + std::to_string(digits) + " digits (got " + s.substr(0, 20) + "...)", reportTestCases);
		return fails;
	}

	// ---- reach: long double limbs go past double's 303 digits ----------------------------

	template<unsigned N>
	int VerifyReach(bool reportTestCases) {
		using R = ereal<N, long double>;
		int fails = 0;
		const R third = R(1.0) / R(3.0);
		const int bits = agreed_bits(value_of(third) * dyadic::from_fp(3.0), dyadic::from_fp(1.0));
		const int digits = static_cast<int>(bits * 0.30103);
		// double limbs stop at 19 limbs, about 1007 bits (303 digits)
		fails += expect(digits > 303, "ereal<" + std::to_string(N) + ", long double>: 1/3 to " + std::to_string(digits) + " digits, past double's 303", reportTestCases);
		const R x = R(2.0);
		const R r = x / R(7.0);
		fails += expect(agreed_bits(value_of(r) * dyadic::from_fp(7.0), dyadic::from_fp(2.0)) > 1007, "ereal<" + std::to_string(N) + ", long double>: 2/7 beyond 1007 bits", reportTestCases);
		return fails;
	}

}  // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal<maxlimbs, FpType> on float, double and long double limbs";
	std::string test_tag    = "ereal limbs";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	std::cout << "long double: digits " << std::numeric_limits<long double>::digits
	          << (is_expansion_limb_v<long double> ? " -- a valid limb" : " -- not a limb on this host") << '\n';

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyToString<4, float>("float", 20, true), test_tag, "float to_string");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLimits(reportTestCases), test_tag, "limits");
	nrOfFailedTestCases += ReportTestResult(VerifyConversions<5, float>("ereal<5, float>", reportTestCases), test_tag, "float conversions");
	nrOfFailedTestCases += ReportTestResult(VerifyConversions<8, double>("ereal<8>", reportTestCases), test_tag, "double conversions");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<5, float>("ereal<5, float>", 200, reportTestCases), test_tag, "float arithmetic");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<8, double>("ereal<8>", 200, reportTestCases), test_tag, "double arithmetic");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<5, float>("ereal<5, float>", reportTestCases), test_tag, "float special values");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<8, double>("ereal<8>", reportTestCases), test_tag, "double special values");
	nrOfFailedTestCases += ReportTestResult(VerifyToString<5, float>("ereal<5, float>", 25, reportTestCases), test_tag, "float to_string");
	nrOfFailedTestCases += ReportTestResult(VerifyToString<8, double>("ereal<8>", 100, reportTestCases), test_tag, "double to_string");
	if constexpr (is_expansion_limb_v<long double>) {
		nrOfFailedTestCases += ReportTestResult(VerifyConversions<8, long double>("ereal<8, long double>", reportTestCases), test_tag, "long double conversions");
		nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<8, long double>("ereal<8, long double>", 200, reportTestCases), test_tag, "long double arithmetic");
		nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<8, long double>("ereal<8, long double>", reportTestCases), test_tag, "long double special values");
		nrOfFailedTestCases += ReportTestResult(VerifyToString<8, long double>("ereal<8, long double>", 100, reportTestCases), test_tag, "long double to_string");
		nrOfFailedTestCases += ReportTestResult(VerifyReach<24>(reportTestCases), test_tag, "long double reach");
	}
#endif

#if REGRESSION_LEVEL_2
	if constexpr (is_expansion_limb_v<long double>) {
		nrOfFailedTestCases += ReportTestResult(VerifyReach<64>(reportTestCases), test_tag, "long double reach, 64 limbs");
	}
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
