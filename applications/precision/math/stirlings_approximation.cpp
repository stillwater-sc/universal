//  stirlings_approximation.cpp : Stirling's approximation for factorials
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>

#include <universal/native/ieee754.hpp>
#include <universal/number/edecimal/edecimal.hpp> // the oracle number system to use
#include <universal/number/posit/posit.hpp>
#include <math/functions/factorial.hpp>

#include <universal/utility/error.hpp>

/*
 * Stirling's approximation is an approximation for factorials, leading to accurate
 * results even for small values of n. It is named after James Stirling, though it
 * was first stated by Abraham de Moivre.
 *
 * The version of the formula typically used in applications is
 *
 *     ln n! = n ln n - n + O(ln n)
 *
 * Changing the base of the logarithm (for instance in the worst-case lower bound for comparison sorting)
 *
 *     log_2 n! = n log_2 n - n log_2 e + O(log_2 n)
 *
 * Specifying theh constant and the O(ln n) error term gives 1 over 2 times ln(2 pi n)
 * yielding the more precise formula
 *
 *     n! ~ sqrt(2 pi n)( n / e)^n
 *
 * where the ~ symbol indicates that the two quantities are asymptotic, that is, their ratio tends to 1
 * as n tends to infinity.
 *
 * One may also give simple bounds valid for all positive integers n, rather than only for large n:
 *
 *     sqrt(2 pi) * n^(n+1/2) * e^(-1) <= n! <= e * n^(n + 1/2) * e ^(-n)
 *
 */

template<typename Scalar>
Scalar StirlingsApproximation(size_t n) {
	Scalar pi = 3.14159265358979323846;
	Scalar term1 = sqrt(Scalar(2) * pi * Scalar(n));
	Scalar e = 2.71828182845904523536;
	Scalar term2 = pow(Scalar(n) / e, Scalar(n));

	Scalar factorial = term1 * term2;
	return factorial;
}

/*
 factorial                Stirling's Approximation                      Real Approximation                        Actual Factorial                         Relative Error
		 1! =                                 0.922137                                         1                                               1                                     -0.07786300
		 2! =                                    1.919                                         2                                               2                                     -0.04049780
		 3! =                                  5.83621                                         6                                               6                                     -0.02729840
		 4! =                                  23.5062                                        24                                              24                                     -0.02057600
		 5! =                                  118.019                                       120                                             120                                     -0.01650690
		 6! =                                  710.078                                       720                                             720                                     -0.01378030
		 7! =                                   4980.4                                      5040                                            5040                                     -0.01182620
		 8! =                                  39902.4                                     40320                                           40320                                     -0.01035730
		 9! =                                   359537                                    362880                                          362880                                     -0.00921276
		10! =                               3.5987e+06                                3.6288e+06                                         3628800                                     -0.00829596
		11! =                              3.96156e+07                               3.99168e+07                                        39916800                                     -0.00754507
		12! =                              4.75687e+08                               4.79002e+08                                       479001600                                     -0.00691879
		13! =                              6.18724e+09                               6.22702e+09                                      6227020800                                     -0.00638850
		14! =                               8.6661e+10                               8.71783e+10                                     87178291200                                     -0.00593370
		15! =                              1.30043e+12                               1.30767e+12                                   1307674368000                                     -0.00553933
		16! =                              2.08141e+13                               2.09228e+13                                  20922789888000                                     -0.00519412
		17! =                              3.53948e+14                               3.55687e+14                                 355687428096000                                     -0.00488940
		18! =                               6.3728e+15                               6.40237e+15                                6402373705728000                                     -0.00461846
		19! =                              1.21113e+17                               1.21645e+17                              121645100408832000                                     -0.00437596
		20! =                              2.42279e+18                                2.4329e+18                             2432902008176640000                                     -0.00415765
		21! =                              5.08886e+19                               5.10909e+19                            51090942171709440000                                     -0.00396009
		22! =                              1.11975e+21                                 1.124e+21                          1124000727777607680000                                     -0.00378045
		23! =                              2.57585e+22                                2.5852e+22                         25852016738884976640000                                     -0.00361641
		24! =                              6.18298e+23                               6.20448e+23                        620448401733239439360000                                     -0.00346600
		25! =                              1.54596e+25                               1.55112e+25                      15511210043330985984000000                                     -0.00332761
		26! =                              4.02001e+26                               4.03291e+26                     403291461126605635584000000                                     -0.00319984
		27! =                              1.08553e+28                               1.08889e+28                   10888869450418352160768000000                                     -0.00308152
		28! =                              3.03982e+29                               3.04888e+29                  304888344611713860501504000000                                     -0.00297164
		29! =                              8.81639e+30                               8.84176e+30                 8841761993739701954543616000000                                     -0.00286932
		30! =                              2.64517e+32                               2.65253e+32               265252859812191058636308480000000                                     -0.00277382
 */

int main()
try {
	using namespace sw::universal;
	using namespace sw::math::function;

	using Real = posit<256,2>;
	using Integer = edecimal;

	constexpr size_t FIRST_COLUMN = 10;
	constexpr size_t COLUMN_WIDTH = 40;
	std::cout << std::setw(FIRST_COLUMN) << "factorial"
		<< std::setw(COLUMN_WIDTH) << "Stirling's Approximation"
		<< std::setw(COLUMN_WIDTH) << "Real Approximation"
		<< std::setw(COLUMN_WIDTH) << "Actual Factorial"
		<< std::setw(COLUMN_WIDTH) << "Relative Error\n";
	for (size_t i = 1; i < 31; i += 1) {
		Real approximation = StirlingsApproximation<Real>(i);
		Real actual        = factorial<Real>(i);
		Integer oracle     = factorial<Integer>(i);
		std::cout << std::setw(FIRST_COLUMN) << i << "! = "
			<< std::setw(COLUMN_WIDTH) << approximation << '\t'
			<< std::setw(COLUMN_WIDTH) << actual << '\t'
			<< std::setw(COLUMN_WIDTH) << oracle << '\t'
			<< std::setw(COLUMN_WIDTH) << RelativeError(approximation, actual) << '\n';
	}
	std::cout << std::endl;
	{
		std::string ref = "815915283247897734345611269596115894272000000000";
		double ld = factorial<double>(40);
		double ldr = factoriali<double>(40);
		edecimal d = factorial<edecimal>(40);
		double ad = double(d);
		auto precision = std::cout.precision();
		auto digits = std::numeric_limits<double>::max_digits10;
		std::cout << std::setprecision(digits);
		std::cout << "factorial(40) calculated with double and decimal oracle rounded to double\n";
		std::cout << ref << '\n';
		std::cout << d << '\n';
		std::cout << std::setw(digits + 5ll) << ld << '\n';
		std::cout << std::setw(digits + 5ll) << ldr << '\n';
		std::cout << std::setw(digits + 5ll) << ad << "   TODO: explain the difference between the two methods of calculation\n";
		std::cout << "scale of 40! is " << scale(ld) << '\n';

		std::cout << "factorial(50) calculated with double and decimal oracle rounded to double\n";
		ref = "30414093201713378043612608166064768844377641568960512000000000000";
		ld = factorial<double>(50);
		ldr = factoriali<double>(50);
		d = factorial<edecimal>(50);
		ad = double(d);
		std::cout << ref << '\n';
		std::cout << d << '\n';
		std::cout << std::setw(digits + 5ll) << ld << '\n';
		std::cout << std::setw(digits + 5ll) << ldr << '\n';
		std::cout << std::setw(digits + 5ll) << ad << "   TODO: explain the difference between the two methods of calculation\n";
		std::cout << "scale of 50! is " << scale(ld) << '\n';

		std::cout << "factorial(60) calculated with double and decimal oracle rounded to double\n";
		ref = "8320987112741390144276341183223364380754172606361245952449277696409600000000000000";
		ld = factorial<double>(60);
		ldr = factoriali<double>(60);
		d = factorial<edecimal>(60);
		ad = double(d);
		std::cout << ref << '\n';
		std::cout << d << '\n';
		std::cout << std::setw(digits + 5ll) << ld << '\n';
		std::cout << std::setw(digits + 5ll) << ldr << '\n';
		std::cout << std::setw(digits + 5ll) << ad << "   TODO: explain why the two methods show the same error\n";
		std::cout << "scale of 60! is " << scale(ld) << '\n';
		std::cout << std::setprecision(precision);

		
	}

	return EXIT_SUCCESS;
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
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

/*

running the program yields:

...
factorial(40) calculated with double and decimal oracle rounded to double
815915283247897734345611269596115894272000000000
815915283247897734345611269596115894272000000000
8.1591528324789768e+47
8.1591528324789768e+47
8.1591528324789785e+47   TODO: explain the difference between the two methods of calculation
scale of 40! is 159
factorial(50) calculated with double and decimal oracle rounded to double
30414093201713378043612608166064768844377641568960512000000000000
30414093201713378043612608166064768844377641568960512000000000000
3.0414093201713376e+64
3.0414093201713376e+64
3.0414093201713381e+64   TODO: explain the difference between the two methods of calculation
scale of 50! is 214
factorial(60) calculated with double and decimal oracle rounded to double
8320987112741390144276341183223364380754172606361245952449277696409600000000000000
8320987112741390144276341183223364380754172606361245952449277696409600000000000000
8.3209871127413916e+81
8.3209871127413916e+81
8.3209871127413916e+81   TODO: explain why the two methods show the same error
scale of 60! is 272



  The problem is NOT catastrophic cancellation!

  The intuition that LSB-first accumulation avoids cancellation is correct. All terms are positive, so there's no subtraction-based precision loss. 
  The accumulator grows proportionally with the terms, exactly as you described.

  The actual problem: order *= 10 develops a systematic bias

  The order variable is multiplied by 10.0 at every step. Here's the key fact: 10^k is exactly representable in double only for k ≤ 22 (because 10^k = 5^k ×
   2^k and 5^23 exceeds 53 bits). Starting at k=23, each order *= 10 rounds, and the rounding always goes the same direction — down:

  order < true 10^k:  15 times  (systematic negative bias)
  order > true 10^k:   0 times
  order = true 10^k:  33 times

  This is the critical difference from random rounding errors that would tend to cancel out. The order multiplier develops a monotonically accumulating
  negative bias because 10.0 in binary is 1.01 × 2³ — when you multiply a slightly-too-small value by this, the rounding tends to stay on the low side.

  The consequence: every term from digit 23 onward is biased low

  Every digits[k] * order for k ≥ 23 is systematically smaller than the true digits[k] × 10^k. The trace shows it concretely for 40!:

  step 38: digit=2 × 10^38  got 1.999...96e+38  should be 2e+38
  step 40: digit=8 × 10^40  got 7.999...83e+40  should be 8.000...02e+40
  step 43: digit=1 × 10^43  got 9.999...89e+42  should be 1e+43
  step 44: digit=9 × 10^44  got 8.999...84e+44  should be 9e+44

  Paradoxically, this cumulative negative bias in the terms produces a result that ends up +1 ULP high (0x...98ff vs correct 0x...98fe) because of how the
  biased-low terms interact with the addition rounding. The error direction flips — the biased low terms cause the accumulator's rounding to overshoot.

  Why Horner doesn't have this problem

  Horner's method (d = d * 10 + digit) doesn't maintain a separate power-of-ten variable. Each step transforms the accumulated best answer — it multiplies
  the current approximation by 10, then adds the next digit. The errors from each multiplication are relative to the value itself and tend to be more
  balanced (no systematic bias from maintaining a separate geometric sequence).

  Why long double is the real fix

  Even Horner in double can be off by 1 ULP for very large numbers (50! for example). The definitive fix is using long double (80-bit, 64-bit significand)
  as the intermediate, where 10^k is exact for k ≤ 27, and the extra 11 bits of precision prevent the Horner rounding errors from accumulating enough to
  cross a ULP boundary when cast back to double. In testing, this produces correctly-rounded doubles for all factorials up to 170!.

 */
