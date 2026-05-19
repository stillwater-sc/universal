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
#include <universal/number/cfloat/cfloat.hpp>

#include <iostream>
#include <iomanip>
#include <strstream>

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

namespace sw::universal {
	template<unsigned _ndigits, unsigned _es, DecimalEncoding _Encoding, typename bt>
	const std::string dfp_pair(const dfloat<_ndigits, _es, _Encoding, bt>& a, unsigned precision = 7) {
		std::stringstream s;
		s << std::setprecision(precision) << a << " : ";
		s << to_binary(a, true);
		return s.str();
	}

    template<typename Real>
    void numeric_properties() {
	    Real maxval = std::numeric_limits<Real>::max();
	    Real minval = std::numeric_limits<Real>::min();
	    std::cout << type_tag(Real{}) << '\n';
	    std::cout << " radix     : " << std::numeric_limits<Real>::radix << '\n';
	    std::cout << " digits    : " << std::numeric_limits<Real>::digits << '\n';
	    std::cout << " digits10  : " << std::numeric_limits<Real>::digits10 << '\n';
	    std::cout << " is_exact  : " << std::numeric_limits<Real>::is_exact << '\n';
	    std::cout << " max       : " << dfp_pair(maxval, 7u) << '\n';
	    std::cout << " min       : " << dfp_pair(minval, 7u) << '\n';
    }
    
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dfloat<> Application Programming Interface tests";
	std::string test_tag    = "dfloat<> API";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// important behavioral traits
	std::cout << "+---------    BID decimal floating-point behavioral traits\n";
	{
		ReportTrivialityOfType<decimal32>();
		ReportTrivialityOfType<decimal64>();
		ReportTrivialityOfType<decimal128>();
	}

	// BID encoding decimal floating-point arithmetic operators
	std::cout << "+---------    BID encoding decimal floating-point arithmetic operators\n";
	{
		using Real = dfloat<7, 6, DecimalEncoding::BID, std::uint32_t>;  // BID : Binary Integer Decimal : significand stored as binary integer
		std::cout << "type : " << type_tag(Real{}) << '\n';

		Real a(1.0f), b(0.5f);
		ArithmeticOperators(a, b);
	}
	// DPD encoding decimal floating-point arithmetic operators
	std::cout << "+---------    DPD encoding decimal floating-point arithmetic operators\n";
	{
		using Real = dfloat<7, 6, DecimalEncoding::DPD, std::uint32_t>;  // DPD : Densely Packed Decimal encoding : significand stored as 10-bit declets
		std::cout << "type : " << type_tag(Real{}) << '\n';

		Real a(1.0f), b(0.5f);
		ArithmeticOperators(a, b);
	}
	// BCD encoding decimal floating-point arithmetic operators
	std::cout << "+---------    BCD encoding decimal floating-point arithmetic operators\n";
	{
		using Real = dfloat<7, 6, DecimalEncoding::BCD, std::uint32_t>;  // BCD : Binary Coded Decimal (4 bits per digit)
		std::cout << "type : " << type_tag(Real{}) << '\n';

		Real a(1.0f), b(0.5f);
		ArithmeticOperators(a, b);
	}

	// basic value construction and conversion
	std::cout << "+---------    Basic value construction and conversion\n";
	{
		using Real = decimal32;

		Real zero(0);
		Real one(1);
		Real ten(10);
		Real quarter(0.25);
		Real half(0.5);
		Real pi(3.14159);
		Real pinf(SpecificValue::infpos);

		std::cout << "+inf      : " << std::setw(12) << pinf    << " : " << to_binary(pinf) << '\n';
		std::cout << "ten       : " << std::setw(12) << ten     << " : " << to_binary(ten) << '\n';
		std::cout << "zero      : " << std::setw(12) << zero    << " : " << to_binary(zero) << '\n';
		std::cout << "one       : " << std::setw(12) << one     << " : " << to_binary(one) << '\n';
		std::cout << "minus one : " << std::setw(12) << -one    << " : " << to_binary(-one) << '\n';
		std::cout << "half      : " << std::setw(12) << half    << " : " << to_binary(half) << '\n';
		std::cout << "quarter   : " << std::setw(12) << quarter << " : " << to_binary(quarter) << '\n';
		std::cout << "pi        : " << std::setw(12) << pi      << " : " << to_binary(pi) << '\n';

		// verify round-trip through double
		double a = 1.23456789e10;
		Real r(a);
		double b = double(a);
		if (a != b) {
			std::cerr << "FAIL: round-trip 1.23456789e10 failed: " << a << " != " << b << '\n';
			std::cerr << to_binary(a) << '\n' << to_binary(b) << '\n';
			++nrOfFailedTestCases;
		} else {
			std::cout << "PASS: round-trip 1.23456789e10 succeeded\n";
			ReportValue(a, "a", 2, 10);
			ReportValue(b, "b", 2, 10);
		}
	}

	// Decimal Floating-Point properties: NaN, infinity
	std::cout << "+---------    Decimal FP properties: NaN, infinity\n";
	{
		using Real = decimal32;

		Real a(1.0f), b(0.0f), c{};
		c = a / b;  // should produce infinity
		ReportValue(c, "1.0 / 0.0");
		std::cout << "  isinf(1.0 / 0.0)  : " << (c.isinf() ? "true" : "false") << '\n'; // should be true
		c = b / b;  // should produce NaN
		ReportValue(c, "0.0 / 0.0");
		std::cout << "  isnan(0.0 / 0.0)  : " << (c.isnan() ? "true" : "false") << '\n'; // should be true;

		Real qnan(SpecificValue::qnan);
		ReportValue(qnan, "quiet NaN");
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

		std::cout << "+inf     : " << std::setw(12) << pinf << " : " << to_binary(pinf) << " isinf=" << pinf.isinf() << '\n';
		std::cout << "-inf     : " << std::setw(12) << ninf << " : " << to_binary(ninf) << " isinf=" << ninf.isinf() << '\n';
		std::cout << "qnan     : " << std::setw(12) << qnan << " : " << to_binary(qnan) << " isnan=" << qnan.isnan() << '\n';
		std::cout << "snan     : " << std::setw(12) << snan << " : " << to_binary(snan) << " isnan=" << snan.isnan() << '\n';
		std::cout << "maxpos   : " << std::setw(12) << maxp << " : " << to_binary(maxp) << '\n';
		std::cout << "minpos   : " << std::setw(12) << minp << " : " << to_binary(minp) << '\n';

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
		for (int i = 0; i < 10; ++i)
			sum += tenth;
		std::cout << "10 * 0.1 = " << sum << '\n';
	}

	// integer type conversion
	std::cout << "+---------    Integer type conversion\n";
	{
		using Real = dfloat<7, 6>;

		Real a(42);
		Real b(-17);
		Real c(1000000);

		std::cout << "42      : " << std::setw(12) << a << " : " << to_binary(a) << " : " << components(a) << '\n';
		std::cout << "-17     : " << std::setw(12) << b << " : " << to_binary(b) << " : " << components(b) << '\n';
		std::cout << "1000000 : " << std::setw(12) << c << " : " << to_binary(c) << " : " << components(c) << '\n';
	}

	// dynamic range
	std::cout << "+---------    Dynamic range\n";
	{
		dfloat<7, 6> d32;
		std::cout << dynamic_range(d32) << '\n';

		dfloat<16, 8> d64;
		std::cout << dynamic_range(d64) << '\n';

		dfloat<34, 12> d128;
		std::cout << dynamic_range(d128) << '\n';
	}

	// numeric_limits
	std::cout << "+---------    numeric_limits\n";
	{ 
		numeric_properties<decimal32>();
		numeric_properties<decimal64>();
		numeric_properties<decimal128>();
	}

	// parsing input strings
	std::cout << "+---------    parsing input strings\n";
	{
		decimal32 d32("999.9999");
		ReportValue(d32, "d32 initial");
		d32.assign("-123.456e-78");
		ReportValue(d32, "d32 assigned");
	}
	{
		decimal64 d64("999.9999999999999");
		ReportValue(d64, "d64 initial");
		d64.assign("-123.456e-78");
		ReportValue(d64, "d64 assigned");
	}
	{
		decimal128 d128("999.999999999999999999999");
		ReportValue(d128, "d128 initial");
		d128.assign("-123.456e-78");
		ReportValue(d128, "d128 assigned");
	}

		// printing
	std::cout << "+---------    I/O and printing\n";
	{
		std::cout << "\ncfloat<32, 8> reference\n";
		using Real = single;
		Real a(SpecificValue::maxneg), b(SpecificValue::minpos);
		std::cout << "values print : " << a << ", " << b << '\n';
		std::cout << "binary print : " << to_binary(a, true) << "\n             : " << to_binary(b, true) << '\n';
		std::cout << "color print  : " << color_print(a) << "\n             : " << color_print(b, true) << '\n';
		std::cout << "components   : " << components(a) << "\n             : " << components(b) << '\n';
	}
	{
		std::cout << "\ndecimal32 reference\n";
		using Real = decimal32;
		Real a(SpecificValue::maxneg), b(SpecificValue::minpos);
		std::cout << "values print : " << a << ", " << b << '\n';
		std::cout << "binary print : " << to_binary(a, true) << "\n             : " << to_binary(b, true) << '\n';
		std::cout << "color print  : " << color_print(a) << "\n             : " << color_print(b, true) << '\n';
		std::cout << "components   : " << components(a) << "\n             : " << components(b) << '\n';
	}
	{
		std::cout << "\ndecimal64 reference\n";
		using Real = decimal64;
		Real a(SpecificValue::maxneg), b(SpecificValue::minpos);
		std::cout << "values print : " << a << ", " << b << '\n';
		std::cout << "binary print : " << to_binary(a, true) << "\n             : " << to_binary(b, true) << '\n';
		std::cout << "color print  : " << color_print(a) << "\n             : " << color_print(b, true) << '\n';
		std::cout << "components   : " << components(a) << "\n             : " << components(b) << '\n';
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
