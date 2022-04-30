// fractional.cpp: test suite runner for classification functions specialized for classic floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system configuration
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/cfloat_math_test_suite.hpp>

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
Real test_fmod(Real x, Real y) {
	std::cout << "fmod( " << x << ", " << y << ")\n";
	if (x < y) return x;
	Real c = x / y;
	std::cout << x << " / " << y << " = " << c << '\n';
	Real n(c);
	n.truncate();
	Real n_times_y = n * y;
	std::cout << "x       = " << x << '\n';
	std::cout << "n       = " << n << '\n';
	std::cout << "n*y     = " << n_times_y << '\n';
	std::cout << "x - n*y = " << x - n_times_y << '\n';

	return x - n_times_y;
}

/*
std::remainder(x, y)
The IEEE floating-point remainder of the division operation x/y calculated by this function is 
exactly the value x - n*y, where the value n is the integral value nearest the exact value x/y. 
When |n-x/y| = ½, the value n is chosen to be even.

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


/*
frac(x) returns the fractional value of x
*/

template<typename Real>
Real test_frac(Real x) {
	using namespace sw::universal;
	std::cout << "frac(" << x << ") = " << frac(x) << '\n';
	std::cout << "reference = " << (double(x) - (long long)(x)) << '\n';
	return frac(x);
}

#define MANUAL_TESTING 1

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat<> mathlib fractional validation";
	std::string test_tag    = "fractional";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

#define MY_DBL_MIN          2.2250738585072014e-308 // minpos value

	constexpr size_t nbits = 32;
	constexpr size_t es = 8;
	using bt = uint32_t;
	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = false;
	using Real = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;

	float fa(1.5), fb(2.25);
	Real a(fa), b(fb);

	std::cout << "IEEE-754 float reference\n";
	std::cout << "fmod      : " << fmod(fa, fb) << " : " << fa << " : " << fb << '\n';
	std::cout << "fmod      : " << fmod(fb, fa) << " : " << fb << " : " << fa << '\n';
	std::cout << "remainder : " << remainder(fa, fb) << " : " << fa << " : " << fb << '\n';
	std::cout << "remainder : " << remainder(fb, fa) << " : " << fb << " : " << fa << '\n';
//	std::cout << "frac      : " << std::frac(fa) << " : " << fa << '\n';

	std::cout << "cfloat results\n";
	std::cout << "fmod      : " << fmod(a, b) << " : " << a << " : " << b << '\n';
	std::cout << "fmod      : " << fmod(b, a) << " : " << b << " : " << a << '\n';
	std::cout << "remainder : " << remainder(a, b) << " : " << a << " : " << b << '\n';
	std::cout << "remainder : " << remainder(b, a) << " : " << b << " : " << a << '\n';
	std::cout << "frac      : " << frac(a) << " : " << a << '\n';

	{
		using Real = cfloat<16, 2, uint8_t, false, false, false>;
		Real a, b, c;
		a = -1.5;
		a.blocks();
		std::cout << to_binary(a) << " : " << a << " : " << a.truncate() << " : " << to_binary(a) << " : " << a << '\n';
	}

	{
		using Real = cfloat<16, 2, uint8_t, false, false, false>;
		Real a, b, c;
		a = 1.5; b = 2.25;
		c = test_fmod(a, b);
		std::cout << "fmod = " << c << '\n';
		c = test_fmod(b, a);
		std::cout << "fmod = " << c << '\n';
	}

	{
		using Real = cfloat<32, 8, uint8_t, false, false, false>;
		Real a, b, c;
		a = 1.5; b = 2.25;
		c = test_fmod(a, b);
		std::cout << "fmod = " << c << '\n';
		c = test_fmod(b, a);
		std::cout << "fmod = " << c << '\n';
	}

	{
		using Real = cfloat<32, 8, uint8_t, false, false, false>;
		Real a;
		a = 1.5;
		test_frac(a);
		test_frac(a = 2.25);
		test_frac(a = 0.25);
		test_frac(a = 0.0625);
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

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
