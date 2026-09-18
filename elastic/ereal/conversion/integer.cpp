// integer.cpp: ereal construction and assignment from integers
//
// convert_signed() and convert_unsigned() were `// TBD` stubs for every non-zero value,
// and all ten integer constructors and assignment operators route through them, so every
// ereal built or assigned from a non-zero integer was silently zero: ereal(5), ereal(-7)
// and `e = 42` all compared equal to 0 (#1460).
//
// The checks are exact. A 64-bit integer does not fit one double, so the expected value
// is compared by summing the limbs in integer arithmetic, which owes nothing to ereal.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <climits>
#include <cstdint>
#include <limits>
#include <string>

#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	int expect_true(bool actual, const char* what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}


	using Real = ereal<>;

	// Whether an ereal holds exactly the given 64-bit integer.
	//
	// A 64-bit integer does not fit one double, and long double is only 64 bits on some
	// hosts, so the check cannot lean on a wider float. Instead: every limb of an
	// integer-valued expansion is itself an integer, so the limbs are summed in uint64
	// arithmetic, modulo 2^64, and compared with the wanted bit pattern. Modular equality
	// alone would accept a value off by a multiple of 2^64, so the plain double sum is
	// also required to land within rounding of the wanted value, which rules that out.
	bool holds_exactly(const Real& v, std::uint64_t wantedBits, double wantedApprox) {
		constexpr double twoTo64 = 18446744073709551616.0;
		std::uint64_t sumBits = 0;
		double approx = 0.0;
		for (double limb : v.limbs()) {
			if (limb != std::floor(limb) || std::abs(limb) > twoTo64) return false;
			approx += limb;
			const double mag = std::abs(limb);
			const std::uint64_t magBits = (mag == twoTo64) ? 0u : static_cast<std::uint64_t>(mag);
			sumBits += (limb < 0.0) ? (std::uint64_t(0) - magBits) : magBits;
		}
		return sumBits == wantedBits && std::abs(approx - wantedApprox) <= 4096.0;
	}

	bool holds_exactly(const Real& v, std::int64_t wanted) {
		return holds_exactly(v, static_cast<std::uint64_t>(wanted), static_cast<double>(wanted));
	}

	bool holds_exactly_unsigned(const Real& v, std::uint64_t wanted) {
		return holds_exactly(v, wanted, static_cast<double>(wanted));
	}

	// ---- the values the issue reported -------------------------------------------

	int VerifySmallIntegers(bool reportTestCases) {
		int fails = 0;

		fails += expect_true(double(Real(5)) == 5.0, "ereal(5) is 5", reportTestCases);
		fails += expect_true(double(Real(-7)) == -7.0, "ereal(-7) is -7", reportTestCases);
		Real e;
		e = 42;
		fails += expect_true(double(e) == 42.0, "assignment from 42", reportTestCases);
		fails += expect_true(!Real(1).iszero(), "ereal(1) is not zero", reportTestCases);
		fails += expect_true(Real(0).iszero(), "ereal(0) is zero", reportTestCases);
		fails += expect_true(Real(3) == Real(3.0), "an integer and a double agree", reportTestCases);

		// every integer a double holds exactly converts to exactly that double
		for (int v = -1000; v <= 1000; ++v) {
			if (double(Real(v)) != double(v)) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL ereal(" << v << ")\n";
			}
		}

		return fails;
	}

	// ---- all ten integer types, both through the constructor and through operator= ----

	template<typename Int>
	int check_type(Int v, const char* tag, bool reportTestCases) {
		int fails = 0;
		Real constructed(v);
		Real assigned;
		assigned = v;
		const double wanted = static_cast<double>(v);
		fails += expect_true(double(constructed) == wanted, tag, reportTestCases);
		fails += expect_true(double(assigned) == wanted, tag, reportTestCases);
		return fails;
	}

	int VerifyEveryIntegerType(bool reportTestCases) {
		int fails = 0;

		fails += check_type<signed char>(-100, "signed char", reportTestCases);
		fails += check_type<short>(-30000, "short", reportTestCases);
		fails += check_type<int>(-2000000000, "int", reportTestCases);
		fails += check_type<long>(-2000000000L, "long", reportTestCases);
		fails += check_type<long long>(-9000000000000LL, "long long", reportTestCases);
		fails += check_type<unsigned short>(60000, "unsigned short", reportTestCases);
		fails += check_type<unsigned int>(4000000000u, "unsigned int", reportTestCases);
		fails += check_type<unsigned long>(4000000000ul, "unsigned long", reportTestCases);
		fails += check_type<unsigned long long>(9000000000000ull, "unsigned long long", reportTestCases);

		// plain char is signed on some platforms and unsigned on others. It must convert
		// to its value either way -- a negative char read as unsigned would be near 2^64.
		{
			const char c = static_cast<char>(-5);
			const double wanted = static_cast<double>(c);    // -5 or 251, per the platform
			fails += expect_true(double(Real(c)) == wanted, "plain char converts to its value",
				reportTestCases);
			fails += check_type<char>('A', "char", reportTestCases);
		}

		// the extremes of every signed type, where the magnitude does not fit the type
		fails += check_type<signed char>(SCHAR_MIN, "SCHAR_MIN", reportTestCases);
		fails += check_type<short>(SHRT_MIN, "SHRT_MIN", reportTestCases);
		fails += check_type<int>(INT_MIN, "INT_MIN", reportTestCases);
		fails += check_type<int>(INT_MAX, "INT_MAX", reportTestCases);

		return fails;
	}

	// ---- above 2^53 a double cannot hold every integer: the expansion must --------------

	int VerifyWideIntegersAreExact(bool reportTestCases) {
		int fails = 0;

		// values a single double would round
		const std::int64_t signedCases[] = {
			(std::int64_t(1) << 53) + 1,             // the first integer a double loses
			123456789012345678LL,
			-123456789012345678LL,
			LLONG_MAX,                               // 2^63 - 1
			LLONG_MIN,                               // -2^63, whose negation overflows
			LLONG_MIN + 1,
		};
		for (std::int64_t v : signedCases) {
			if (!holds_exactly(Real(static_cast<long long>(v)), v)) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL ereal(" << v << ") is not exact\n";
			}
		}

		const std::uint64_t unsignedCases[] = {
			(std::uint64_t(1) << 53) + 1,
			(std::uint64_t(1) << 63) + 1,
			ULLONG_MAX,                              // 2^64 - 1: rounds to 2^64 in a double
			ULLONG_MAX - 12345,
		};
		for (std::uint64_t v : unsignedCases) {
			if (!holds_exactly_unsigned(Real(static_cast<unsigned long long>(v)), v)) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL ereal(" << v << "u) is not exact\n";
			}
		}

		// the naive single-double conversion really would have been wrong for these
		fails += expect_true(static_cast<std::int64_t>(static_cast<double>((std::int64_t(1) << 53) + 1))
		                     != (std::int64_t(1) << 53) + 1,
			"a double really cannot hold 2^53 + 1", reportTestCases);

		return fails;
	}

	// ---- integer-constructed values take part in arithmetic ---------------------------

	int VerifyArithmeticOnIntegers(bool reportTestCases) {
		int fails = 0;

		Real a(7), b(3);
		fails += expect_true(double(a + b) == 10.0, "7 + 3", reportTestCases);
		fails += expect_true(double(a - b) == 4.0, "7 - 3", reportTestCases);
		fails += expect_true(double(a * b) == 21.0, "7 * 3", reportTestCases);
		fails += expect_true(double(a * b / b) == 7.0, "7 * 3 / 3", reportTestCases);

		// the difference of two wide integers that differ by one is exactly one
		Real big(LLONG_MAX), bigger(LLONG_MAX - 1);
		fails += expect_true(double(big - bigger) == 1.0,
			"LLONG_MAX - (LLONG_MAX - 1) is exactly 1", reportTestCases);

		return fails;
	}

}  // anonymous namespace

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

	std::string test_suite  = "ereal integer conversion";
	std::string test_tag    = "ereal integer";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySmallIntegers(reportTestCases), test_tag, "small integers");
	nrOfFailedTestCases += ReportTestResult(VerifyEveryIntegerType(reportTestCases), test_tag, "every integer type");
	nrOfFailedTestCases += ReportTestResult(VerifyWideIntegersAreExact(reportTestCases), test_tag, "wide integers exact");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticOnIntegers(reportTestCases), test_tag, "arithmetic on integers");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
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
