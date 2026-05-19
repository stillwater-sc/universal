// api.cpp: application programming interface tests for IBM System/360 hexadecimal floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the hfloat template environment
#define HFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/hfloat/hfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/number/cfloat/cfloat.hpp>

#include <iostream>
#include <iomanip>
#include <strstream>

namespace sw::universal {
	template<unsigned _ndigits, unsigned _es, typename bt>
	const std::string hfp_pair(const hfloat<_ndigits, _es, bt>& a, unsigned precision = 7) {
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
		std::cout << " radix            : " << std::numeric_limits<Real>::radix << '\n';
		std::cout << " digits (binary)  : " << std::numeric_limits<Real>::digits << '\n';
		std::cout << " has_infinity     : " << (std::numeric_limits<Real>::has_infinity ? "yes" : "no") << '\n';
		std::cout << " has_quiet_NaN    : " << (std::numeric_limits<Real>::has_quiet_NaN ? "yes" : "no") << '\n';
		std::cout << " round_style      : " << std::numeric_limits<Real>::round_style << " (toward_zero=0)\n";
		std::cout << " max              : " << hfp_pair(maxval, 7u) << '\n';
		std::cout << " min              : " << hfp_pair(minval, 7u) << '\n';
	}
}  // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "hfloat<> Application Programming Interface tests";
	std::string test_tag = "hfloat<> API";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// important behavioral traits
	{
		ReportTrivialityOfType<hfp32>();
		ReportTrivialityOfType<hfp64>();
		ReportTrivialityOfType<hfp128>();
	}

	// IBM HFP short precision
	std::cout << "+---------    IBM System/360 Hexadecimal Floating-Point tests\n";
	{
		using Real = hfp32;
		std::cout << "type : " << type_tag(Real{}) << '\n';

		Real a(1.0f), b(0.5f);
		ArithmeticOperators(a, b);
	}

	// basic value construction and conversion
	std::cout << "+---------    Basic value construction and conversion\n";
	{
		using Real = hfp32;

		Real zero(0);
		Real one(1);
		Real ten(10);
		Real quarter(0.25);
		Real half(0.5);

		std::cout << "zero    : " << zero    << " : " << to_binary(zero)    << " : " << components(zero)    << '\n';
		std::cout << "one     : " << one     << " : " << to_binary(one)     << " : " << components(one)     << '\n';
		std::cout << "ten     : " << ten     << " : " << to_binary(ten)     << " : " << components(ten)     << '\n';
		std::cout << "quarter : " << quarter << " : " << to_binary(quarter) << " : " << components(quarter) << '\n';
		std::cout << "half    : " << half    << " : " << to_binary(half)    << " : " << components(half)    << '\n';

		// hex representation
		std::cout << "one  hex: " << to_hex(one) << '\n';
		std::cout << "ten  hex: " << to_hex(ten) << '\n';
		std::cout << "half hex: " << to_hex(half) << '\n';

		// verify round-trip through double
		double d = 42.0;
		Real r(d);
		double d2 = double(r);
		if (d != d2) {
			std::cerr << "FAIL: round-trip 42.0 failed: " << d << " != " << d2 << '\n';
			++nrOfFailedTestCases;
		}
	}

	// IBM HFP properties: no NaN, no infinity
	std::cout << "+---------    IBM HFP properties: no NaN, no infinity\n";
	{
		using Real = hfp32;

		Real a(1.0f);
		std::cout << "isnan(1.0)  : " << a.isnan() << " (should be 0)\n";
		std::cout << "isinf(1.0)  : " << a.isinf() << " (should be 0)\n";

		// NaN request maps to zero
		Real qnan(SpecificValue::qnan);
		std::cout << "qnan maps to: " << qnan << " (should be 0)\n";

		// infinity request maps to maxpos
		Real pinf(SpecificValue::infpos);
		Real mp(SpecificValue::maxpos);
		std::cout << "infpos maps to maxpos: " << (pinf == mp ? "PASS" : "FAIL") << '\n';
		if (!(pinf == mp)) ++nrOfFailedTestCases;
	}

	// arithmetic operations
	std::cout << "+---------    Arithmetic operations\n";
	{
		using Real = hfp32;

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

	// wobbling precision demonstration
	std::cout << "+---------    Wobbling precision (IBM HFP characteristic)\n";
	{
		using Real = hfp32;

		// 1.0 and 8.0 have different effective precision due to hex alignment
		Real one(1.0f), eight(8.0f);
		std::cout << "1.0  binary: " << to_binary(one, true)   << " : " << components(one) << '\n';
		std::cout << "8.0  binary: " << to_binary(eight, true) << " : " << components(eight) << '\n';
		std::cout << "Note: 1.0 has 3 leading zero bits in its MSB hex digit (wobbling precision)\n";
	}

	// special values
	std::cout << "+---------    Special values\n";
	{
		using Real = hfp32;

		Real maxp(SpecificValue::maxpos);
		Real minp(SpecificValue::minpos);
		Real maxn(SpecificValue::maxneg);
		Real minn(SpecificValue::minneg);

		std::cout << "maxpos : " << maxp << " : " << to_binary(maxp) << " : " << components(maxp) << '\n';
		std::cout << "minpos : " << minp << " : " << to_binary(minp) << " : " << components(minp) << '\n';
		std::cout << "maxneg : " << maxn << " : " << to_binary(maxn) << " : " << components(maxn) << '\n';
		std::cout << "minneg : " << minn << " : " << to_binary(minn) << " : " << components(minn) << '\n';
	}

	// dynamic range
	std::cout << "+---------    Dynamic range\n";
	{
		hfp32 s;
		std::cout << dynamic_range(s) << '\n';
	}

	// numeric_limits
	std::cout << "+---------    numeric_limits\n";
	{ 
		numeric_properties<hfp32>();
		numeric_properties<hfp64>();
		numeric_properties<hfp128>();
	}

	// truncation rounding verification
	std::cout << "+---------    Truncation rounding (never rounds up)\n";
	{
		using Real = hfp32;

		// 1/3 should truncate, not round
		Real one(1), three(3);
		Real result = one / three;
		double dresult = double(result);
		double exact = 1.0 / 3.0;
		std::cout << "1/3 in hfloat: " << result << " (double: " << dresult << ")\n";
		std::cout << "1/3 exact:     " << exact << '\n';
		// truncation means result <= exact value for positive numbers
		if (dresult > exact) {
			std::cerr << "FAIL: truncation rounding should never produce a result larger than exact\n";
			++nrOfFailedTestCases;
		}
	}

	// parsing
	std::cout << "+---------    parsing\n";
	{
		hfp32 a;
		hfp64 b;
		hfp128 c;

		parse("1.23456789e-10", a);
		ReportValue(a, "parsed 1.23456789e-10");
		parse("1.23456789e-10", b);
		ReportValue(b, "parsed 1.23456789e-10");
		parse("1.23456789e-10", c);
		ReportValue(c, "parsed 1.23456789e-10");

		// high-precision constants to seed high-precision HFPs
		constexpr char pi_str[] = "3.141592653589793238462643383279502884197169";
		parse(pi_str, a);
		ReportValue(a, "parsed pi", 21, 7);
		parse(pi_str, b);
		ReportValue(b, "parsed pi", 21, 14);
		parse(pi_str, c);
		ReportValue(c, "parsed pi", 21, 28);
	}

	// printing
	std::cout << "+---------    I/O and printing\n";
	{
		std::cout << "\ncfloat<32, 8> reference\n";
		using Real = single;
		Real a(SpecificValue::maxneg), b(SpecificValue::minpos);
		std::cout << "values print : " << a << ", " << b << '\n';
		std::cout << "hex print    : " << to_hex(a) << ", " << to_hex(b) << '\n';
		std::cout << "binary print : " << to_binary(a, true) << "\n             : " << to_binary(b, true) << '\n';
		std::cout << "color print  : " << color_print(a) << "\n             : " << color_print(b, true) << '\n';
		std::cout << "components   : " << components(a) << "\n             : " << components(b) << '\n';
	}
	{
		std::cout << "\nhfp32 reference\n";
		using Real = hfp32;
		Real a(SpecificValue::maxneg), b(SpecificValue::minpos);
		std::cout << "values print : " << a << ", " << b << '\n';
		std::cout << "hex print    : " << to_hex(a) << ", " << to_hex(b) << '\n';
		std::cout << "binary print : " << to_binary(a, true) << "\n             : " << to_binary(b, true) << '\n';
		std::cout << "color print  : " << color_print(a) << "\n             : " << color_print(b, true) << '\n';
		std::cout << "components   : " << components(a) << "\n             : " << components(b) << '\n';
	}
	{
		std::cout << "\nhfp64 reference\n";
		using Real = hfp64;
		Real a(SpecificValue::maxneg), b(SpecificValue::minpos);
		std::cout << "values print : " << a << ", " << b << '\n';
		std::cout << "hex print    : " << to_hex(a) << "\n             : " << to_hex(b) << '\n';
		std::cout << "binary print : " << to_binary(a, true) << "\n             : " << to_binary(b, true) << '\n';
		std::cout << "color print  : " << color_print(a, true) << "\n             : " << color_print(b, true) << '\n';
		std::cout << "components   : " << components(a) << "\n             : " << components(b) << '\n';
	}
	{
		std::cout << "\nhfp128 reference\n";
		using Real = hfp128;
		Real a(SpecificValue::maxneg), b(SpecificValue::minpos);
		std::cout << "values print : " << a << ", " << b << '\n';
		std::cout << "hex print    : " << to_hex(a) << "\n             : " << to_hex(b) << '\n';
		std::cout << "binary print : " << to_binary(a, true) << "\n             : " << to_binary(b, true) << '\n';
		std::cout << "color print  : " << color_print(a, true) << "\n             : " << color_print(b, true) << '\n';
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
