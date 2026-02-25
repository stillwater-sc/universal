// fractional.cpp: test suite runner for mod/frac/reminder functions specialized for logarithmic floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/verification/lns_test_suite_mathlib.hpp>

namespace sw { namespace universal {

template<typename TestType>
int VerifyLnsFractionExponent(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a{}, b{}, c{};
	int exp;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		b = frexp(a, &exp);
		c = ldexp(b, exp);
//		std::cout << "input : " << to_binary(a) << " : " << a << '\n';
//		std::cout << "frexp : " << to_binary(b) << " : " << b << '\n';
//		std::cout << "ldexp : " << to_binary(c) << " : " << c << '\n';
		if (a != c) {
			if (a.isnan() && c.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "frexp/ldexp", a, b, c);
		}
		else {
			// if (reportTestCases) ReportOneInputFunctionError("PASS", "frexp/ldexp", a, b, c);
		}
		if (nrOfFailedTests > 24) return 25;
	}
	return nrOfFailedTests;
}

// enumerate all fmod value combinations for a lns configuration
template<typename TestType>
int VerifyLnsFmod(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a{}, b{}, c{}, n{}, fref{};

	for (size_t i = 0; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		for (size_t j = 0; j < NR_TEST_CASES; ++j) {
			b.setbits(j);
			c = fmod(a, b);
			// generate reference
			if (b.isnan()) {
				fref.setnan();
			}
			else if (b.iszero()) { // this must return a quiet NaN
				fref.setnan();
			}
			else if (a.iszero()) {
				fref = a;          // take the sign of a
			}
			else if (a.isinf()) {
				fref.setnan();
			}

			else if (b.isinf()) {
				fref = a;
			}
			else {
				n = int(a / b);  // we can use an int because this method can only run with lns that are small enough
				fref = a - n * b;
			}

			if (c != fref) {
				if (c.isnan() && fref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
				if (c.iszero() && fref.iszero()) continue; // optimizer destroys the sign
				nrOfFailedTests++;
				if (reportTestCases)	ReportTwoInputFunctionError("FAIL", "fmod", a, b, c, fref);
				std::cout << "a / b = " << n << '\n';
				std::cout << "n * y = " << (n * b) << '\n';
				std::cout << "cmod  = " << c << '\n';
				std::cout << "fmod  = " << std::fmod(float(a), float(b)) << '\n';
				std::cout << "fref  = " << fref << '\n';
			}
			else {
				// if (reportTestCases) ReportTwoInputFunctionError("PASS", "fmod", a, b, c, fref);
			}
			if (nrOfFailedTests > 24) return 25;
		}
	}
	return nrOfFailedTests;
}

}}

/*
 std::fmod(x, y)
 The floating-point remainder of the division operation x/y calculated by this function is 
 exactly the value x - n*y, where n is x/y with its fractional part truncated.

The returned value has the same sign as x and is less than y in magnitude.
 Parameters
x, y	-	floating point values
Return value
If successful, returns the floating-point remainder of the division x/y as defined above.

If a domain error occurs, an implementation-defined value is returned (NaN where supported).

If a range error occurs due to underflow, the correct result (after rounding) is returned.
*/
template<typename Real>
Real trace_fmod(Real x, Real y) {
	using namespace sw::universal;
	using std::trunc;  // incase Real is a native
	constexpr size_t NR_DIGITS = 20;

	auto old_precision = std::cout.precision();
	std::cout << std::setprecision(NR_DIGITS);
	std::cout << "fmod( " << x << ", " << y << ")\n";
	if (x < y) return x;
	Real c = x / y;
	std::cout << x << " / " << y << " = " << to_binary(c) << " : " << c << '\n';
	Real n = trunc(c);
	//n.truncate();

	std::cout << "x         = " << to_binary(x) << " : " << x << '\n';
	std::cout << "n         = " << to_binary(n) << " : " << n << '\n';
	Real n_times_y = n * y;
	std::cout << "n*y       = " << to_binary(n_times_y) << " : " << n_times_y << '\n';
	Real diff = x - n_times_y;
	std::cout << "x - n*y   = " << to_binary(diff) << " : " << diff << '\n';
	float floatmod = std::fmod(float(x), float(y));
	std::cout << "std::fmod = " << to_binary(floatmod) << " : " << floatmod << '\n';
	std::cout << std::setprecision(old_precision);

	return x - n_times_y;
}

/*
std::remainder(x, y)
The IEEE floating-point remainder of the division operation x/y calculated by this function is 
exactly the value x - n*y, where the value n is the integral value nearest the exact value x/y. 
When |n-x/y| = 1/2, the value n is chosen to be even.

In contrast to std::fmod(), the returned value is not guaranteed to have the same sign as x.

If the returned value is 0, it will have the same sign as x.
Parameters
x, y	-	values of floating-point or integral types
Return value
If successful, returns the IEEE floating-point remainder of the division x/y as defined above.

If a domain error occurs, an implementation-defined value is returned (NaN where supported)

If a range error occurs due to underflow, the correct result is returned.

If y is zero, but the domain error does not occur, zero is returned.
*/
template<typename Real>
Real trace_remainder(Real x, Real y) {
	using namespace sw::universal;
	using std::trunc;  // incase Real is a native
	std::cout << "remainder( " << x << ", " << y << ")\n";
	if (x < y) return x;
	Real c = x / y;
	std::cout << x << " / " << y << " = " << c << '\n';
	Real n = trunc(c);
	//n.truncate();

	Real n_times_y = n * y;
	std::cout << "x       = " << x << '\n';
	std::cout << "n       = " << n << '\n';
	std::cout << "n*y     = " << n_times_y << '\n';
	std::cout << "x - n*y = " << x - n_times_y << '\n';

	return x - n_times_y;
}

/*
frac(x) returns the fractional value of x
*/

template<typename Real>
Real test_frac(Real x) {
	using namespace sw::universal;
	std::cout << "frac(" << x << ") = " << frac(x) << '\n';
	std::cout << "reference = " << (double(x) - double(static_cast<long long>(x))) << '\n';
	return frac(x);
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "lns<> mathlib fractional validation";
	std::string test_tag    = "fractional";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		std::cout << std::setprecision(8);
		float f = 1e9f / 3.0f;
		std::cout << to_binary(f) << " : " << f << '\n';
		f = 3 * f;
		std::cout << to_binary(f) << " : " << f << '\n';
	}

	{
		float f = 3;
		trace_fmod(1e1f, f);
		trace_fmod(1e3f, f);
		trace_fmod(1e6f, f);
		trace_fmod(1e9f, f);
	}
	{
		double f = 3;
		trace_fmod(1e1, f);
		trace_fmod(1e3, f);
		trace_fmod(1e6, f);
		trace_fmod(1e9, f);
	}

	{
		float f = 3.14159265358979f;
		trace_fmod(1e1f, f);
		trace_fmod(1e3f, f);
		trace_fmod(1e6f, f);
		trace_fmod(1e9f, f);
	}

	{
		using Real = sw::universal::lns<32, 8, uint32_t>;
		Real pi = Real(3.14159265358979);
		std::cout << to_binary(pi) << " : " << pi << '\n';
		for (int i = 0; i < 10; ++i) {
			Real powerOfTen = Real(std::pow(10.0f, float(i)));
			std::cout << to_binary(powerOfTen) << " : " << powerOfTen << '\n';
			Real cmod = lnsmod(powerOfTen, pi);
			float fmod = std::fmod(float(powerOfTen), float(pi));
			std::cout << "lnsmod  : " << to_binary(cmod) << " : " << cmod << "\n";
			std::cout << "floatmod: " << to_binary(fmod) << " : " << fmod << '\n';
		}

		//		std::cout << "float: " << std::fmod(1e9f, 3.14159265358979f) << "\n";
	}


// #define MY_DBL_MIN          2.2250738585072014e-308 // minpos value

	{
		constexpr size_t nbits = 32;
		constexpr size_t es = 8;
		using bt = uint32_t;
		using Real = lns<nbits, es, bt>;

		float fa(1.5), fb(2.25);
		Real a(fa), b(fb);

		std::cout << "IEEE-754 float reference\n";
		std::cout << "fmod      : " << fmod(fa, fb) << " : " << fa << " : " << fb << '\n';
		std::cout << "fmod      : " << fmod(-fa, fb) << " : " << -fa << " : " << fb << '\n';
		std::cout << "fmod      : " << fmod(fb, -fa) << " : " << fb << " : " << fa << '\n';
		std::cout << "fmod      : " << fmod(fb, fa) << " : " << fb << " : " << -fa << '\n';
		std::cout << "remainder : " << remainder(fa, fb) << " : " << fa << " : " << fb << '\n';
		std::cout << "remainder : " << remainder(fb, fa) << " : " << fb << " : " << fa << '\n';
	//	std::cout << "frac      : " << std::frac(fa) << " : " << fa << '\n';

		std::cout << "lns results\n";
		std::cout << "fmod      : " << fmod(a, b) << " : " << a << " : " << b << '\n';
		std::cout << "fmod      : " << fmod(-a, b) << " : " << -a << " : " << b << '\n';
		std::cout << "fmod      : " << fmod(b, a) << " : " << b << " : " << a << '\n';
		std::cout << "fmod      : " << fmod(b, -a) << " : " << b << " : " << -a << '\n';
		std::cout << "remainder : " << remainder(a, b) << " : " << a << " : " << b << '\n';
		std::cout << "remainder : " << remainder(b, a) << " : " << b << " : " << a << '\n';
		std::cout << "frac      : " << frac(a) << " : " << a << '\n';
		std::cout << "frac      : " << frac(Real(-2.0625)) << " : " << Real(-2.0625) << '\n';
		for (int i = 0; i < 10; ++i) {
			Real x = 0.5;
			Real exp = pow(10.0, i);
			x += exp;
			std::cout << "frac      : " << to_binary(frac(x)) << " : " << frac(x) << " : " << to_binary(x) << " : " << x << '\n';
		}
	}

	{
		using Real = lns<16, 2, uint8_t>;
		Real a;
		a = -1.5;
//		a.showLimbs();
		std::cout << to_binary(a) << " : " << a << " : " << trunc(a) << " : " << to_binary(a) << " : " << a << '\n';
	}

	{
		using Real = lns<16, 2, uint8_t>;
		Real a, b, c;
		a = 1.5; b = 2.25;
		c = trace_fmod(a, b);
		std::cout << "fmod = " << c << '\n';
		c = trace_fmod(b, a);
		std::cout << "fmod = " << c << '\n';
		c = trace_fmod(-a, b);
		std::cout << "fmod = " << c << '\n';
		c = trace_fmod(b, -a);
		std::cout << "fmod = " << c << '\n';
	}

	{
		using Real = lns<32, 8, uint8_t>;
		Real a, b, c;
		a = 1.5; b = 2.25;
		c = trace_fmod(a, b);
		std::cout << "fmod = " << c << '\n';
		c = trace_fmod(b, a);
		std::cout << "fmod = " << c << '\n';
	}

	{
		using Real = lns<32, 8, uint32_t>;
		float fa(1e9f), fb(3.14159265358979f), fc;
		Real a(fa), b(fb), c;
		std::cout << "lns    : " << fmod(Real(a), Real(b)) << "\n";
		std::cout << "float  : " << std::fmod(fa, fb) << "\n";
		fc = trace_fmod(fa, fb);
		std::cout << "fmod = " << fc << '\n';
		c = trace_fmod(a, b);
		std::cout << "fmod = " << c << '\n';
	}

	{
		using Real = lns<32, 8, uint8_t>;
		Real a;
		a = 1.5;
		test_frac(a);
		test_frac(a = 2.25);
		test_frac(a = 0.25);
		test_frac(a = 0.0625);
		test_frac(a = -0.0625);
	}

	{
		using Real = lns<32, 23, std::uint32_t>;
		Real a, b, c;
		float fa(32.0f), fb(0.0625f + 0.125f);
		a = fa;
		b = fb;
		c = a + b;
		std::cout << to_binary(a) << " : " << "scale : " << a.scale() << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << "scale : " << b.scale() << " : " << b << '\n';
		std::cout << to_binary(c) << " : " << "scale : " << c.scale() << " : " << c << '\n';

		int exp;
		auto fr = frexp(c, &exp);
		std::cout << to_binary(fr) << " : " << "scale : " << exp << " : " << fr << '\n';
		c = ldexp(fr, exp);
		std::cout << to_binary(c) << " : " << "scale : " << c.scale() << " : " << c << '\n';

		std::cout << "fmod " << fmod(fa, fb) << '\n';
		std::cout << "cmod " << fmod(a, b) << '\n';

	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

	{
		using Real = lns<8, 4, uint8_t>;
		Real a, b, c;
		a.setbits(1u);
		b.setinf(false);

		//float fa = 0.0214844f;
		//float fb = 0.00585938f;
		float fa = 0.5f;
		float fb = 0.00195312f;
		a = fa;
		b = fb;

		std::cout << "a = " << a << " and b = " << b << '\n';
		std::cout << "a = " << color_print(a) << " and b = " << color_print(b) << '\n';
		c = lnsmod(a, b);
		std::cout << "lnsmod(" << a << ", " << b << ") = " << c << '\n';

		Real d = a / b;
		Real n = trunc(d);

		std::cout << "a / b " << d << " : " << n << " : " << (n * b) << " : " << (a - (n * b)) << '\n';
		std::cout << "fmod " << fmod(fa, fb) << '\n';
		std::cout << "cmod " << fmod(a, b) << '\n';

	}

	{
		float a, b, c;
		a = 0.001953125f;
		b = INFINITY;
		c = fmod(a, b);
		std::cout << a << " " << b << " = " << c << '\n';
	}

	{
		float a, b, c;
		a = INFINITY;
		b = 0.001953125f;
		c = fmod(a, b);
		std::cout << a << " " << b << " = " << c << '\n';
	}



#if REGRESSION_LEVEL_1
	{
		using LNS8_4 = lns<8, 4, std::uint8_t>;

		nrOfFailedTestCases += ReportTestResult(VerifyLnsFractionExponent < LNS8_4 >(reportTestCases), type_tag(LNS8_4()), "frexp/ldexp");
		nrOfFailedTestCases += ReportTestResult(VerifyLnsFmod < LNS8_4 >(reportTestCases), type_tag(LNS8_4()), "fmod");
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
