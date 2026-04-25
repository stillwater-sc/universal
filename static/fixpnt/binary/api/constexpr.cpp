// constexpr.cpp: compile time tests for fixed-point constexpr
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_suite.hpp>
#include <math/constants/double_constants.hpp>

template<typename Fixpnt>
int DecoratedConstructors() {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	{
		// decorated constructors
		{
			constexpr Fixpnt a(1l);  // signed long
			std::cout << a << '\n';
		}
		{
			constexpr Fixpnt a(1ul);  // unsigned long
			std::cout << a << '\n';
		}
		// constexpr for float depends on C++20 support and bit_cast<>
		{
			BIT_CAST_CONSTEXPR Fixpnt a(1.0f);  // float
			std::cout << a << '\n';
		}
		{
			BIT_CAST_CONSTEXPR Fixpnt a(1.0);   // double
			std::cout << a << '\n';
		}
#if LONG_DOUBLE_SUPPORT
		{
			#if defined(DEBUG_LONG_DOUBLE_CONSTEXPR)
			Fixpnt a(1.0l);  // long double
			std::cout << a << '\n';
			#endif
		}
#endif // LONG_DOUBLE_SUPPORT

	}

	return nrOfFailedTestCases;
}

template<typename Fixpnt>
int AssignmentOperators() {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	{
		// assignment operators
		{
			constexpr Fixpnt a = 1l;  // signed long
			std::cout << a << '\n';
		}
		{
			constexpr Fixpnt a = 1ul;  // unsigned long
			std::cout << a << '\n';
		}
		// constexpr for float depends on C++20 support and bit_cast<>
		{
			BIT_CAST_CONSTEXPR Fixpnt a = 1.0f;  // float
			std::cout << a << '\n';
		}
		{
			BIT_CAST_CONSTEXPR Fixpnt a = 1.0;   // double
			std::cout << a << '\n';
		}
#if LONG_DOUBLE_SUPPORT
		{
			Fixpnt a = 1.0l;  // long double
			std::cout << a << '\n';
		}
#endif // LONG_DOUBLE_SUPPORT
	}

	return nrOfFailedTestCases;
}

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

template<typename Fixpnt>
void ConstexprFixpnt() {
	CONSTEXPRESSION Fixpnt a(sw::universal::d_pi);
	std::cout << type_tag(a) << " : " << a << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fixed-point constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Fixpnt = sw::universal::fixpnt<8, 4, Modulo, uint16_t>;

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	{
		fixpnt<8, 4> a(pi);
		std::cout << a << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(DecoratedConstructors<Fixpnt>(), test_tag, "constructors");
	nrOfFailedTestCases += ReportTestResult(AssignmentOperators<Fixpnt>(), test_tag, "assignment");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

	constexpr size_t FIRST_COLUMN = 43;
	std::cout << "constexpr pi approximations\n";
	std::cout << std::setw(FIRST_COLUMN) << "type" << " : " << d_pi << '\n';
	ConstexprFixpnt<fixpnt<8, 4>>();
	ConstexprFixpnt<fixpnt<9, 6>>();
	ConstexprFixpnt<fixpnt<16, 4>>();
	ConstexprFixpnt<fixpnt<16, 8>>();
	ConstexprFixpnt<fixpnt<16, 12>>();
	ConstexprFixpnt<fixpnt<32, 28>>();
	auto oldPrecision = std::cout.precision();
	std::cout << std::setprecision(30);
	std::cout << std::setw(FIRST_COLUMN) << "double" << " : " << d_pi << '\n';
	std::cout << std::setprecision(oldPrecision);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(DecoratedConstructors<Fixpnt>(), test_tag, "constructors");
	nrOfFailedTestCases += ReportTestResult(AssignmentOperators<Fixpnt>(), test_tag, "assignment");

	// ----------------------------------------------------------------------
	// Comprehensive constexpr arithmetic / comparison / shift / bitwise
	// (issue #721) -- joins integer (#720), posit (#718), cfloat (#719),
	// bfloat16 (#725) as a fully constexpr-compliant Universal type.
	// ----------------------------------------------------------------------
	{
		std::cout << "+----- constexpr arithmetic + comparison (issue #721)\n";

		// Acceptance form: Q16.16 multiply 2.5 * 1.5 produces 3.75 at compile time.
		using Q16_16 = fixpnt<32, 16, Modulo, std::uint32_t>;
		BIT_CAST_CONSTEXPR Q16_16 q_a(2.5);
		BIT_CAST_CONSTEXPR Q16_16 q_b(1.5);
		constexpr auto q_prod = q_a * q_b;  // 2.5 * 1.5 = 3.75
		// raw bit pattern of 3.75 in Q16.16 = 3 << 16 | 0xC000 = 0x0003C000
		// uint32 block: low 32 bits hold the whole representation.
		constexpr auto q_prod_bits = q_prod.bits();
		static_assert(q_prod_bits.block(0) == 0x0003C000u, "Q16.16 2.5 * 1.5 == 3.75 raw bits 0x0003C000");

		// Pure integer arithmetic on Modulo Q-format
		using Q32 = fixpnt<32, 0, Modulo, std::uint32_t>;
		constexpr Q32 a(42), b(7);
		constexpr auto cx_sum  = a + b;
		constexpr auto cx_diff = a - b;
		constexpr auto cx_prod = a * b;
		constexpr auto cx_quot = a / b;
		constexpr auto cx_rem  = a % b;
		constexpr auto cx_neg  = -a;
		constexpr auto cx_shl  = a << 2;
		constexpr auto cx_shr  = a >> 2;

		// Compound assignment via lambda
		constexpr Q32 cx_addeq = []() { Q32 t(42); t += Q32(7); return t; }();
		constexpr Q32 cx_muleq = []() { Q32 t(42); t *= Q32(7); return t; }();

		// Constexpr comparisons
		static_assert(!(a == b), "constexpr 42 != 7");
		static_assert(b < a,     "constexpr 7 < 42");
		static_assert(a >= b,    "constexpr 42 >= 7");

		// Cross-check via runtime; use to_long() for value comparison
		Q32 ra(42), rb(7);
		Q32 r49(49), r294(294), r6(6), r0(0), r_neg42(-42), r168(168), r10(10);
		auto check = [&](const char* name, bool ok) {
			if (!ok) { ++nrOfFailedTestCases; std::cout << "FAIL " << name << '\n'; }
		};
		check("constexpr 42+7  == 49",  cx_sum  == r49);
		check("constexpr 42*7  == 294", cx_prod == r294);
		check("constexpr 42/7  == 6",   cx_quot == r6);
		check("constexpr 42%7  == 0",   cx_rem  == r0);
		check("constexpr -42",          cx_neg  == r_neg42);
		check("constexpr 42<<2 == 168", cx_shl  == r168);
		check("constexpr 42>>2 == 10",  cx_shr  == r10);
		check("constexpr += matches +", cx_addeq == r49);
		check("constexpr *= matches *", cx_muleq == r294);
		(void)cx_diff;

		// Multi-limb constexpr: fixpnt<128, 64> via uint32 limbs.
		using Q128_64 = fixpnt<128, 64, Modulo, std::uint32_t>;
		BIT_CAST_CONSTEXPR Q128_64 ml_a(1000.0);
		BIT_CAST_CONSTEXPR Q128_64 ml_b(1000.0);
		constexpr auto ml_prod = ml_a * ml_b;
		BIT_CAST_CONSTEXPR Q128_64 r_million(1000000.0);
		check("constexpr Q128.64 1000 * 1000 == 1_000_000 (multi-limb mul)", ml_prod == r_million);

		// Saturating variant: maxpos clamping under * overflow at compile time.
		using Sat8_4 = fixpnt<8, 4, Saturate, std::uint8_t>;
		constexpr Sat8_4 sat_a(7);   // close to maxpos
		constexpr Sat8_4 sat_b(2);
		constexpr auto sat_prod = sat_a * sat_b;  // 7*2 = 14, would overflow -> clamp
		constexpr Sat8_4 sat_maxpos(SpecificValue::maxpos);
		check("constexpr Saturate 7*2 clamps to maxpos", sat_prod == sat_maxpos);

		// Saturating addition that should NOT overflow (in-range result).
		constexpr Sat8_4 sat_add = []() { Sat8_4 t(3); t += Sat8_4(1); return t; }();  // 3+1=4 in range
		constexpr Sat8_4 r4(4);
		check("constexpr Saturate 3+1 == 4 (no clamping)", sat_add == r4);

		// Edge case: divide-by-zero defined behavior under
		// FIXPNT_THROW_ARITHMETIC_EXCEPTION=0 (set above for this TU).
		// Modulo: result is 0 (setzero).
		constexpr Q32 div_zero_mod = []() { Q32 t(42); t /= Q32(0); return t; }();
		check("constexpr Modulo div-by-zero returns 0", div_zero_mod == r0);

		// Saturating: clamps to maxpos for positive dividend.
		constexpr Sat8_4 div_zero_sat = []() { Sat8_4 t(2); t /= Sat8_4(0); return t; }();
		check("constexpr Saturate div-by-zero (pos) clamps to maxpos", div_zero_sat == sat_maxpos);

		std::cout << "constexpr arithmetic regression: PASS\n";
	}
#endif

#if REGRESSION_LEVEL_2
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
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
