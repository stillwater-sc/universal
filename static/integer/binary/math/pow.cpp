// pow.cpp: test suite runner for the integer power of fixed-size integers: ipow and pow(a, int)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
// throw, so that the division by zero of 0^-n is observable
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

/*
   ipow(a, b) is the exact integer power, by squaring, in integer's own arithmetic (#1875):
   a result past the range wraps as integer's operator*= wraps, a^0 == 1 for every a, and a
   negative b gives the truncated 1 / a^|b|. pow(a, int) and pow(a, integer) are ipow; they
   used to go through double, which loses the low bits of any power past 2^53.
*/

namespace sw { namespace universal {

	// a^n wrapped to nbits two's complement, the value integer<nbits> holds. The product is
	// computed modulo 2^64 in unsigned arithmetic, which is also modulo 2^nbits for nbits <= 64.
	template<unsigned nbits>
	long long WrappedPower(long long a, unsigned n) {
		static_assert(nbits <= 64, "the reference is native");
		unsigned long long r = 1, b = static_cast<unsigned long long>(a);
		for (unsigned i = 0; i < n; ++i) r *= b;
		if constexpr (nbits < 64) {
			const unsigned long long mask = (1ull << nbits) - 1ull;
			r &= mask;
			if (r & (1ull << (nbits - 1))) r |= ~mask;   // sign extend
		}
		return static_cast<long long>(r);
	}

	// every base in [amin, amax] and every exponent in [0, nmax], against the native reference,
	// through the three entry points: ipow(integer, integer), ipow(integer, long long), pow(integer, int)
	template<unsigned nbits, typename bt>
	int VerifyIntegerPower(long long amin, long long amax, unsigned nmax, bool reportTestCases) {
		using Integer = integer<nbits, bt>;
		int nrOfFailedTests = 0;
		for (long long a = amin; a <= amax; ++a) {
			for (unsigned n = 0; n <= nmax; ++n) {
				const Integer ref(WrappedPower<nbits>(a, n));
				const Integer x(a);
				const Integer r1 = ipow(x, Integer(n));
				const Integer r2 = ipow(x, static_cast<long long>(n));
				const Integer r3 = pow(x, static_cast<int>(n));
				if (r1 != ref || r2 != ref || r3 != ref) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: " << a << " ^ " << n << " = " << r1 << ", " << r2 << ", " << r3 << " reference " << ref << '\n';
				}
			}
		}
		return nrOfFailedTests;
	}

	// a negative exponent: 1 / a^|n| truncated, and the division by zero for 0
	template<unsigned nbits, typename bt>
	int VerifyNegativeExponent(bool reportTestCases) {
		using Integer = integer<nbits, bt>;
		int nrOfFailedTests = 0;
		for (long long a = -5; a <= 5; ++a) {
			for (int n = -9; n <= -1; ++n) {
				const Integer x(a);
				if (a == 0) {
					int caught = 0;
					try { (void)ipow(x, Integer(n)); } catch (const integer_divide_by_zero&) { ++caught; }
					try { (void)ipow(x, static_cast<long long>(n)); } catch (const integer_divide_by_zero&) { ++caught; }
					try { (void)pow(x, n); } catch (const integer_divide_by_zero&) { ++caught; }
					if (caught != 3) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: 0 ^ " << n << " did not report a division by zero\n";
					}
					continue;
				}
				long long expected = 0;
				if (a == 1) expected = 1;
				if (a == -1) expected = (n % 2 != 0) ? -1 : 1;
				const Integer ref(expected);
				const Integer r1 = ipow(x, Integer(n));
				const Integer r2 = ipow(x, static_cast<long long>(n));
				const Integer r3 = pow(x, n);
				if (r1 != ref || r2 != ref || r3 != ref) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: " << a << " ^ " << n << " = " << r1 << ", " << r2 << ", " << r3 << " reference " << ref << '\n';
				}
			}
		}
		return nrOfFailedTests;
	}

	// past 64 bits: powers of two against a shift, and odd powers against repeated multiplication
	template<unsigned nbits, typename bt>
	int VerifyWidePower(bool reportTestCases) {
		using Integer = integer<nbits, bt>;
		int nrOfFailedTests = 0;
		for (unsigned n = 0; n < nbits - 1; ++n) {
			Integer ref(1);
			ref <<= static_cast<int>(n);
			if (ipow(Integer(2), Integer(n)) != ref || pow(Integer(2), static_cast<int>(n)) != ref) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 2 ^ " << n << " != " << ref << '\n';
			}
		}
		for (long long a : { 3ll, -3ll, 7ll, 10ll, -11ll, 12345ll }) {
			Integer ref(1);
			for (unsigned n = 0; n <= 3 * nbits; ++n) {   // well past the range: the wrap too
				if (ipow(Integer(a), static_cast<long long>(n)) != ref) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: " << a << " ^ " << n << " != " << ref << '\n';
				}
				ref *= Integer(a);
			}
		}
		return nrOfFailedTests;
	}

}} // namespace sw::universal

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

	std::string test_suite  = "integer power function validation";
	std::string test_tag    = "ipow";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using Integer = integer<128, std::uint32_t>;
		std::cout << "3^80     = " << ipow(Integer(3), Integer(80)) << '\n';
		std::cout << "pow(3,80)= " << pow(Integer(3), 80) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPower<8, std::uint8_t>(-128, 127, 20, reportTestCases), "integer<8>", "ipow");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPower<16, std::uint8_t>(-300, 300, 20, reportTestCases), "integer<16>", "ipow");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPower<64, std::uint32_t>(-20, 20, 70, reportTestCases), "integer<64>", "ipow");
	nrOfFailedTestCases += ReportTestResult(VerifyNegativeExponent<16, std::uint16_t>(reportTestCases), "integer<16>", "ipow negative exponent");
	nrOfFailedTestCases += ReportTestResult(VerifyWidePower<256, std::uint32_t>(reportTestCases), "integer<256>", "ipow");

	{
		// pow(a, int) used to go through double: 3^40 = 12157665459056928801 > 2^53 lost its low bits
		using Integer = integer<128, std::uint32_t>;
		Integer ref;
		ref.assign("12157665459056928801");
		if (pow(Integer(3), 40) != ref || pow(Integer(3), Integer(40)) != ref) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: pow(3, 40) = " << pow(Integer(3), 40) << " reference " << ref << '\n';
		}
	}
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPower<12, std::uint16_t>(-2048, 2047, 15, reportTestCases), "integer<12>", "ipow");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPower<32, std::uint32_t>(-1000, 1000, 40, reportTestCases), "integer<32>", "ipow");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyWidePower<1024, std::uint32_t>(reportTestCases), "integer<1024>", "ipow");
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::integer_arithmetic_exception& err) {
	std::cerr << "Uncaught integer arithmetic exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::integer_internal_exception& err) {
	std::cerr << "Uncaught integer internal exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
