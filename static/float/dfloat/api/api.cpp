// api.cpp: application programming interface tests for decimal floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the dfloat template environment
#define DFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>

/*
Table 3.6 of the IEEE 754-2008 spec defines a set of standard decimal floats from the total bit width k using four formulas:

  ┌─────────────────────────────────────────────┬─────────────────────┬─────────────────────┬───────────────────────┐
  │                   Formula                   │        k=32         │        k=64         │         k=128         │
  ├─────────────────────────────────────────────┼─────────────────────┼─────────────────────┼───────────────────────┤
  │ p = 9k/32 - 2 (precision in digits)         │ 9(32)/32 - 2 = 7    │ 9(64)/32 - 2 = 16   │ 9(128)/32 - 2 = 34    │
  ├─────────────────────────────────────────────┼─────────────────────┼─────────────────────┼───────────────────────┤
  │ w = k/16 + 4 (exponent continuation bits)   │ 32/16 + 4 = 6       │ 64/16 + 4 = 8       │ 128/16 + 4 = 12       │
  ├─────────────────────────────────────────────┼─────────────────────┼─────────────────────┼───────────────────────┤
  │ t = 15k/16 - 10 (trailing significand bits) │ 15(32)/16 - 10 = 20 │ 15(64)/16 - 10 = 50 │ 15(128)/16 - 10 = 110 │
  ├─────────────────────────────────────────────┼─────────────────────┼─────────────────────┼───────────────────────┤
  │ emax = 3 × 2^(k/16+3)                       │ 3 × 2^5 = 96        │ 3 × 2^7 = 384       │ 3 × 2^11 = 6144       │
  └─────────────────────────────────────────────┴─────────────────────┴─────────────────────┴───────────────────────┘

  The bit budget for each format:

  1 (sign) + 5 (combination) + w (exponent) + t (trailing significand) = k

  decimal32:   1 + 5 +  6 +  20 =  32
  decimal64:   1 + 5 +  8 +  50 =  64
  decimal128:  1 + 5 + 12 + 110 = 128

  The trailing significand holds p-1 digits (the leading digit is encoded in the 5-bit combination field):

  - BID: t bits store the trailing digits as a binary integer (2^20 > 10^6, 2^50 > 10^15, 2^110 > 10^33)
  - DPD: t bits store (p-1)/3 declets of 10 bits each (2 declets = 6 digits, 5 declets = 15 digits, 11 declets = 33 digits)

  The formulas were designed so that:
  - The trailing significand is always divisible by 10 bits (for clean DPD declet packing)
  - BID has enough bits to hold 10^(p-1) - 1 as a binary integer
  - The exponent range grows proportionally with precision

dfloat<7, 6> literally means "7 significant decimal digits, 6 exponent continuation bits" — the two independent
parameters that, together with the fixed 1+5 bit sign+combination field, determine everything else.
 */

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dfloat<> Application Programming Interface tests";
	std::string test_tag    = "dfloat<> API";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// important behavioral traits
	{
		//using TestType = decimal32; // == dfloat<7, 6>;
		ReportTrivialityOfType<decimal32>();
		ReportTrivialityOfType<decimal64>();
#ifdef __SIZEOF_INT128__
		ReportTrivialityOfType<decimal128>();
#endif
	}

	// default behavior: BID encoding decimal floating-point
	std::cout << "+---------    BID encoding decimal floating-point\n";
	{
		using Real = dfloat<7, 6>;  // decimal32 equivalent
		std::cout << "type : " << type_tag(Real{}) << '\n';

		Real a(1.0f), b(0.5f);
		ArithmeticOperators(a, b);
	}

	// BID encoding decimal floating-point arithmetic operators
	std::cout << "+---------    BID encoding decimal floating-point arithmetic operators\n";
	{
		using Real = dfloat<7, 6>;  // decimal32 equivalent
		std::cout << "type : " << type_tag(Real{}) << '\n';

		Real a(1.0f), b(0.5f);
		ArithmeticOperators(a, b);
	}

	// basic value construction and conversion
	std::cout << "+---------    Basic value construction and conversion\n";
	{
		using Real = dfloat<7, 6>;

		Real zero(0);
		Real one(1);
		Real ten(10);
		Real quarter(0.25);
		Real half(0.5);
		Real pi(3.14159);

		std::cout << "zero    : " << zero    << " : " << to_binary(zero)    << '\n';
		std::cout << "one     : " << one     << " : " << to_binary(one)     << '\n';
		std::cout << "ten     : " << ten     << " : " << to_binary(ten)     << '\n';
		std::cout << "quarter : " << quarter << " : " << to_binary(quarter) << '\n';
		std::cout << "half    : " << half    << " : " << to_binary(half)    << '\n';
		std::cout << "pi      : " << pi      << " : " << to_binary(pi)      << '\n';

		// verify round-trip through double
		double d = 42.0;
		Real r(d);
		double d2 = double(r);
		if (d != d2) {
			std::cerr << "FAIL: round-trip 42.0 failed: " << d << " != " << d2 << '\n';
			++nrOfFailedTestCases;
		}
	}

	// decimal exactness test
	std::cout << "+---------    Decimal exactness\n";
	{
		using Real = dfloat<7, 6>;

		// 0.1 should be representable exactly in decimal floating-point
		Real tenth(0.1);
		std::cout << "0.1 in dfloat: " << tenth << " : " << to_binary(tenth) << '\n';
		std::cout << "0.1 components: " << components(tenth) << '\n';

		// accumulate ten times 0.1 - should be exactly 1.0
		Real sum(0);
		for (int i = 0; i < 10; ++i) sum += tenth;
		std::cout << "10 * 0.1 = " << sum << '\n';
	}

	// special values
	std::cout << "+---------    Special values\n";
	{
		using Real = dfloat<7, 6>;

		Real pinf(SpecificValue::infpos);
		Real ninf(SpecificValue::infneg);
		Real qnan(SpecificValue::qnan);
		Real snan(SpecificValue::snan);
		Real maxp(SpecificValue::maxpos);
		Real minp(SpecificValue::minpos);

		std::cout << "+inf   : " << pinf << " : " << to_binary(pinf) << " isinf=" << pinf.isinf() << '\n';
		std::cout << "-inf   : " << ninf << " : " << to_binary(ninf) << " isinf=" << ninf.isinf() << '\n';
		std::cout << "qnan   : " << qnan << " : " << to_binary(qnan) << " isnan=" << qnan.isnan() << '\n';
		std::cout << "snan   : " << snan << " : " << to_binary(snan) << " isnan=" << snan.isnan() << '\n';
		std::cout << "maxpos : " << maxp << " : " << to_binary(maxp) << '\n';
		std::cout << "minpos : " << minp << " : " << to_binary(minp) << '\n';

		// NaN comparisons
		if (qnan == qnan) {
			std::cerr << "FAIL: NaN == NaN should be false\n";
			++nrOfFailedTestCases;
		}
	}

	// arithmetic operations
	std::cout << "+---------    Arithmetic operations\n";
	{
		using Real = dfloat<7, 6>;

		Real a(100), b(3);
		Real sum = a + b;
		Real diff = a - b;
		Real prod = a * b;
		Real quot = a / b;

		std::cout << a << " + " << b << " = " << sum  << '\n';
		std::cout << a << " - " << b << " = " << diff << '\n';
		std::cout << a << " * " << b << " = " << prod << '\n';
		std::cout << a << " / " << b << " = " << quot << '\n';
	}

	// integer type conversion
	std::cout << "+---------    Integer type conversion\n";
	{
		using Real = dfloat<7, 6>;

		Real a(42);
		Real b(-17);
		Real c(1000000);

		std::cout << "42      : " << a << " : " << to_binary(a) << " : " << components(a) << '\n';
		std::cout << "-17     : " << b << " : " << to_binary(b) << " : " << components(b) << '\n';
		std::cout << "1000000 : " << c << " : " << to_binary(c) << " : " << components(c) << '\n';
	}

	// dynamic range
	std::cout << "+---------    Dynamic range\n";
	{
		dfloat<7, 6> d32;
		std::cout << dynamic_range(d32) << '\n';

		dfloat<16, 8> d64;
		std::cout << dynamic_range(d64) << '\n';
	}

	// numeric_limits
	std::cout << "+---------    numeric_limits\n";
	{
		using Real = decimal32;
		std::cout << "decimal32 radix     : " << std::numeric_limits<Real>::radix << '\n';
		std::cout << "decimal32 digits    : " << std::numeric_limits<Real>::digits << '\n';
		std::cout << "decimal32 digits10  : " << std::numeric_limits<Real>::digits10 << '\n';
		std::cout << "decimal32 is_exact  : " << std::numeric_limits<Real>::is_exact << '\n';
		std::cout << "decimal32 max       : " << std::numeric_limits<Real>::max() << '\n';
		std::cout << "decimal32 min       : " << std::numeric_limits<Real>::min() << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
