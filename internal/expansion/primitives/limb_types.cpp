// limb_types.cpp: expansion arithmetic on float, double and long double limbs
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// expansion_ops is a template on its limb type (#1563, part 1 of #1355). This suite checks
// that each limb type the host offers gets exact arithmetic:
//
//   - the limb trait accepts the p-bit IEEE-754 binary types and rejects IBM
//     double-double -- by its properties, not by is_iec559, which libstdc++ reports as
//     true for double-double
//   - two_sum, fast_two_sum and two_prod are error-free, and grow, sum, scale and product
//     preserve the exact value, all checked against an exact dyadic oracle rather than
//     through a wider floating-point type (for long double there is none)
//   - results stay non-overlapping at the limb type's own precision
//   - the range management of #1553 works at each type's own top of range
//
// long double is x87 extended on x86-64, binary128 on aarch64 and riscv64, and double on
// MSVC and Apple ARM64; its cases run whenever it is a valid limb.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>
#include <universal/internal/expansion/expansion_ops.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;
	using namespace sw::universal::expansion_ops;

	// the exact value of a limb of any type: dyadic::from_double would round a long double
	template<typename T>
	dyadic to_dyadic(T v) { return dyadic::from_fp(v); }
	template<typename T>
	dyadic sum_of(const std::vector<T>& e) {
		dyadic acc;
		for (T c : e) acc = acc + to_dyadic(c);
		return acc;
	}
	// floor(log2 |d|), for bounding an error; d must be non-zero
	int log2_abs(const dyadic& d) {
		auto n = d.numerator;
		if (n.sign()) n.setsign(false);
		int bits = 0;
		while (!n.iszero()) { n >>= 1; ++bits; }
		return bits - 1 + d.scale;
	}

	template<typename T>
	struct generator {
		std::mt19937_64 rng{ 1563 };
		// a random value 2^[lo, hi] with a full random significand and sign
		T value(int lo, int hi) {
			std::uniform_int_distribution<int> ex(lo, hi), sg(0, 1);
			std::uniform_int_distribution<std::uint32_t> bits;
			T m = T(1);
			for (int k = 0; k < std::numeric_limits<T>::digits; k += 32) m += std::ldexp(T(bits(rng)), -32 - k);
			m = std::ldexp(m, -1);                             // [0.5, 1), and possibly rounded up to 1
			const T v = std::ldexp(m, ex(rng));
			return sg(rng) ? -v : v;
		}
		// A valid, non-overlapping expansion of up to 4 components: a leading value within
		// 2^[-center, center] and tail components down to depth binades below it.
		//
		// The span is chosen per type so that the product of two such expansions keeps its
		// lowest bit above the type's smallest subnormal: below it, the roundoff of two_prod
		// is not representable and no error-free transformation can be exact. For double and
		// long double that leaves room for tails 3p deep; float's range is narrow enough that
		// it binds (center 4, depth 47), which is the limit ereal's float limbs (#1568) will
		// have to manage.
		static constexpr int p = std::numeric_limits<T>::digits;
		static constexpr int center = std::numeric_limits<T>::max_exponent / 32 < 30 ? std::numeric_limits<T>::max_exponent / 32 : 30;
		static constexpr int room = (p - std::numeric_limits<T>::min_exponent) / 2 - center - (p - 1);   // lowest-bit budget
		static constexpr int depth = room < 3 * p ? room : 3 * p;
		std::vector<T> expansion() {
			std::vector<T> e{ value(-center, center) };
			std::uniform_int_distribution<int> n(0, 3);
			for (int k = n(rng); k > 0; --k) e = grow_expansion(e, value(-center - depth, center - p));
			return renormalize_expansion(e);
		}
		// a scalar whose products with such an expansion stay in range as well
		T scalar() { return value(-center, center); }
	};

	int expect(bool ok, const std::string& what, bool reportTestCases) {
		if (ok) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}

	// ---- the limb trait -----------------------------------------------------------------

	int VerifyLimbTrait(bool reportTestCases) {
		int fails = 0;
		static_assert(is_expansion_limb_v<float>);
		static_assert(is_expansion_limb_v<double>);
		static_assert(!is_expansion_limb_v<int>);
		// IBM double-double, as measured on ppc64le: libstdc++ reports is_iec559 == true for
		// it, and only the exponent-range property (IEEE's emin = 1 - emax) separates it.
		constexpr bool ibm_double_double = expansion_limb_properties(true, true, 2, -968, 1024);
		static_assert(!ibm_double_double, "IBM double-double must not be accepted as a limb");
		// the binary types it must accept, from the same measurements
		static_assert(expansion_limb_properties(true, true, 2, -16381, 16384));   // x87, binary128
		// on this host, long double is a limb exactly when it is a p-bit IEEE type
		constexpr bool ld_ieee = std::numeric_limits<long double>::min_exponent == 3 - std::numeric_limits<long double>::max_exponent;
		fails += expect(is_expansion_limb_v<long double> == ld_ieee, "long double limb verdict matches its exponent range", reportTestCases);
		return fails;
	}

	// ---- error-free transformations -----------------------------------------------------

	template<typename T>
	int VerifyErrorFreeTransformations(const char* name, int cases, bool reportTestCases) {
		int fails = 0;
		generator<T> g;
		const int p = std::numeric_limits<T>::digits;
		for (int i = 0; i < cases; ++i) {
			// operands up to 2p binades apart, so the roundoff is often non-zero
			const T a = g.value(-40, 40), b = g.value(-40 - 2 * p, 40);
			T x, y;
			two_sum(a, b, x, y);
			fails += expect(to_dyadic(x) + to_dyadic(y) == to_dyadic(a) + to_dyadic(b), std::string(name) + ": two_sum is error-free", reportTestCases && fails < 5);
			const T big = std::abs(a) >= std::abs(b) ? a : b, small = std::abs(a) >= std::abs(b) ? b : a;
			fast_two_sum(big, small, x, y);
			fails += expect(to_dyadic(x) + to_dyadic(y) == to_dyadic(a) + to_dyadic(b), std::string(name) + ": fast_two_sum is error-free", reportTestCases && fails < 5);
			const T c = g.value(-40, 40);
			two_prod(a, c, x, y);
			fails += expect(to_dyadic(x) + to_dyadic(y) == to_dyadic(a) * to_dyadic(c), std::string(name) + ": two_prod is error-free", reportTestCases && fails < 5);
		}
		return fails;
	}

	// ---- expansion operations keep the exact value --------------------------------------

	template<typename T>
	int VerifyExpansionArithmetic(const char* name, int cases, bool reportTestCases) {
		int fails = 0;
		generator<T> g;
		const int p = std::numeric_limits<T>::digits;
		for (int i = 0; i < cases; ++i) {
			const auto e = g.expansion(), f = g.expansion();
			const T b = g.scalar();
			const dyadic E = sum_of(e), F = sum_of(f);
			const std::string n(name);
			const bool report = reportTestCases && fails < 5;

			const auto grown = grow_expansion(e, b);
			fails += expect(sum_of(grown) == E + to_dyadic(b), n + ": grow_expansion is exact", report);
			const auto sum = expansion_sum_normalized(e, f);
			fails += expect(sum_of(sum) == E + F, n + ": sum is exact", report);
			fails += expect(is_nonoverlapping(sum), n + ": sum is non-overlapping at p = " + std::to_string(p), report);
			const auto scaled = scale_expansion(e, b);
			fails += expect(sum_of(scaled) == E * to_dyadic(b), n + ": scale_expansion is exact", report);
			const auto product = expansion_product(e, f);
			fails += expect(sum_of(product) == E * F, n + ": product is exact", report);
			fails += expect(is_nonoverlapping(product), n + ": product is non-overlapping", report);
			fails += expect(compare_adaptive(e, f) == (E == F ? 0 : (sum_of(expansion_sum_normalized(e, scale_expansion(f, T(-1)))).numerator.sign() ? -1 : 1)),
			                n + ": compare_adaptive agrees with the exact difference", report);

			// the quotient is not exact, but q * f must agree with e far beyond one limb
			if (!F.iszero() && !E.iszero()) {
				const auto q = expansion_quotient(e, f);
				const dyadic residual = sum_of(q) * F - E;
				const int rel = residual.iszero() ? -100000 : log2_abs(residual) - log2_abs(E);
				fails += expect(rel <= -2 * p, n + ": quotient agrees to beyond 2p bits (" + std::to_string(-rel) + ")", report);
			}
		}
		return fails;
	}

	// ---- range management at each type's own top of range (#1553) ------------------------

	template<typename T>
	int VerifyRangeManagement(const char* name, bool reportTestCases) {
		int fails = 0;
		const std::string n(name);
		const T max = std::numeric_limits<T>::max();
		const int emax = std::numeric_limits<T>::max_exponent;
		// max + max overflows: one signed infinity, not an expansion of NaNs
		const auto overflow = expansion_sum_normalized(std::vector<T>{ max }, std::vector<T>{ max });
		fails += expect(overflow.size() == 1 && std::isinf(overflow[0]) && overflow[0] > T(0), n + ": max + max is a single +inf", reportTestCases);
		// max / 2 does not overflow on the way: the dividend is scaled first
		const auto half = expansion_quotient(std::vector<T>{ max }, std::vector<T>{ T(2) });
		fails += expect(!half.empty() && std::isfinite(half[0]) && sum_of(half) == to_dyadic(max) * to_dyadic(T(0.5)), n + ": max / 2 is exact", reportTestCases);
		// a product just inside the range stays finite and exact
		const T a = std::ldexp(T(1.5), emax - 30), b = std::ldexp(T(1.25), 20);
		const auto product = expansion_product(std::vector<T>{ a }, std::vector<T>{ b });
		fails += expect(!product.empty() && std::isfinite(product[0]) && sum_of(product) == to_dyadic(a) * to_dyadic(b), n + ": a product near the top is finite and exact", reportTestCases);
		// two_prod with a factor near the top of the range. For limbs wider than double it is
		// Dekker's product, whose split multiplies by 2^s + 1 and would overflow on such a
		// factor unless it is moved first; {max} * {2^-100} runs unscaled, so this is reached.
		for (const T big : { max, std::ldexp(T(1.75), emax - 10) }) {
			for (const T small : { std::ldexp(T(1), -100), std::ldexp(T(1.5) + std::numeric_limits<T>::epsilon(), -60) }) {
				T x, y;
				two_prod(big, small, x, y);
				fails += expect(std::isfinite(x) && to_dyadic(x) + to_dyadic(y) == to_dyadic(big) * to_dyadic(small), n + ": two_prod with a factor near the top is error-free", reportTestCases);
				two_prod(small, big, x, y);
				fails += expect(std::isfinite(x) && to_dyadic(x) + to_dyadic(y) == to_dyadic(big) * to_dyadic(small), n + ": two_prod with the large factor second", reportTestCases);
				const auto prod = expansion_product(std::vector<T>{ big }, std::vector<T>{ small });
				fails += expect(sum_of(prod) == to_dyadic(big) * to_dyadic(small), n + ": {near max} * {small} is exact", reportTestCases);
			}
		}
		// Scaling an operand down must not lose its smallest components. {max, denorm_min} is
		// a valid expansion spanning the whole range; shifted down, its tail falls below the
		// smallest subnormal. When the leading limbs then cancel, the tail is all that is left:
		// {max, denorm_min} + {-max} was 0, and {max, denorm_min} * {1} lost the denorm_min.
		const T tiny = std::numeric_limits<T>::denorm_min();
		const std::vector<T> wide{ max, tiny };
		const auto cancelled = expansion_sum_normalized(wide, std::vector<T>{ -max });
		fails += expect(sum_of(cancelled) == to_dyadic(tiny), n + ": {max, denorm_min} + {-max} keeps denorm_min", reportTestCases);
		const auto kept = expansion_product(wide, std::vector<T>{ T(1) });
		fails += expect(sum_of(kept) == to_dyadic(max) + to_dyadic(tiny), n + ": {max, denorm_min} * {1} keeps denorm_min", reportTestCases);
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

	std::string test_suite  = "expansion arithmetic on float, double and long double limbs";
	std::string test_tag    = "expansion limbs";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	std::cout << "long double: digits " << std::numeric_limits<long double>::digits
	          << ", min_exponent " << std::numeric_limits<long double>::min_exponent
	          << (is_expansion_limb_v<long double> ? " -- a valid limb" : " -- not a limb on this host") << '\n';

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyErrorFreeTransformations<float>("float", 10, true), test_tag, "float EFTs");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLimbTrait(reportTestCases), test_tag, "limb trait");
	nrOfFailedTestCases += ReportTestResult(VerifyErrorFreeTransformations<float>("float", 2000, reportTestCases), test_tag, "float EFTs");
	nrOfFailedTestCases += ReportTestResult(VerifyErrorFreeTransformations<double>("double", 2000, reportTestCases), test_tag, "double EFTs");
	nrOfFailedTestCases += ReportTestResult(VerifyExpansionArithmetic<float>("float", 300, reportTestCases), test_tag, "float expansions");
	nrOfFailedTestCases += ReportTestResult(VerifyExpansionArithmetic<double>("double", 300, reportTestCases), test_tag, "double expansions");
	nrOfFailedTestCases += ReportTestResult(VerifyRangeManagement<float>("float", reportTestCases), test_tag, "float range");
	nrOfFailedTestCases += ReportTestResult(VerifyRangeManagement<double>("double", reportTestCases), test_tag, "double range");
	if constexpr (is_expansion_limb_v<long double>) {
		nrOfFailedTestCases += ReportTestResult(VerifyErrorFreeTransformations<long double>("long double", 2000, reportTestCases), test_tag, "long double EFTs");
		nrOfFailedTestCases += ReportTestResult(VerifyExpansionArithmetic<long double>("long double", 300, reportTestCases), test_tag, "long double expansions");
		nrOfFailedTestCases += ReportTestResult(VerifyRangeManagement<long double>("long double", reportTestCases), test_tag, "long double range");
	}
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExpansionArithmetic<float>("float", 5000, reportTestCases), test_tag, "float expansions, more");
	nrOfFailedTestCases += ReportTestResult(VerifyExpansionArithmetic<double>("double", 5000, reportTestCases), test_tag, "double expansions, more");
	if constexpr (is_expansion_limb_v<long double>) {
		nrOfFailedTestCases += ReportTestResult(VerifyExpansionArithmetic<long double>("long double", 5000, reportTestCases), test_tag, "long double expansions, more");
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
