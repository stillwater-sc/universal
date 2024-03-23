// bisection.cpp: example program to a root of a polynomial through bisection, a linear method
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit library with arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>

/*

Mathematical 	C++ Symbol	Decimal Representation
Expression
pi              M_PI        3.14159265358979323846
pi/2			M_PI_2		1.57079632679489661923
pi/4			M_PI_4		0.785398163397448309616
1/pi			M_1_PI		0.318309886183790671538
2/pi			M_2_PI		0.636619772367581343076
2/sqrt(pi)		M_2_SQRTPI	1.12837916709551257390
sqrt(2)			M_SQRT2		1.41421356237309504880
1/sqrt(2)		M_SQRT1_2	0.707106781186547524401
e               M_E         2.71828182845904523536
log_2(e)		M_LOG2E		1.44269504088896340736
log_10(e)		M_LOG10E	0.434294481903251827651
log_e(2)		M_LN2		0.693147180559945309417
log_e(10)		M_LN10		2.30258509299404568402

*/

// constexpr double pi = 3.14159265358979323846;  // best practice for C++

// our test function
template<typename Scalar>
Scalar fnctn(const Scalar& a) {
	return a*a*a - 2*a*a + 3;
}

// in general, the quire can be used to improve polynomial function evaluation
// but factorization is likely a better return on investment

template<typename Scalar>
Scalar bisection(Scalar& a, Scalar& b, Scalar (*f)(const Scalar&), const Scalar& precision) {
	if (f(a) * f(b) >= 0) return INFINITY;

	std::cout << "precision = " << precision << '\n';

	// use constexpr when C++20 arrives
	const Scalar half = Scalar(0.5f);
	Scalar c = a;
	while ((b - a) >= precision) {
		c = half * (a + b);
		
//		std::cout << "f(" << a << ") = " << f(a) << std::endl;
//		std::cout << "f(" << c << ") = " << f(c) << std::endl;
//		std::cout << "f(" << b << ") = " << f(b) << std::endl;

		if (abs(f(c)) < precision) {
//			std::cout << "Root at " << c << std::endl;
			break;
		}
		else if (f(c) * f(a) < 0) {
//			std::cout << "narrow right side to " << c << std::endl;
			b = c;
		}
		else {
//			std::cout << "narrow left  side to " << c << std::endl;
			a = c;
		}
	}
	return c;
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	{
		using Scalar = posit<16, 1>;
		Scalar a = -10;
		Scalar b = 20;
		std::cout << "The function used is x ^ 3 - 2x ^ 2 + 3\n";
		std::cout << "a = " << a << '\n';
		std::cout << "b = " << b << '\n';
		Scalar root = bisection(a, b, &fnctn, std::numeric_limits<Scalar>::epsilon());
		std::cout << "root = " << root << std::endl;
	}

	{
		using Scalar = posit<32, 2>;
		Scalar a = -10;
		Scalar b = 20;
		std::cout << "The function used is x ^ 3 - 2x ^ 2 + 3\n";
		std::cout << "a = " << a << '\n';
		std::cout << "b = " << b << '\n';
		Scalar root = bisection(a, b, &fnctn, std::numeric_limits<Scalar>::epsilon());
		std::cout << "root = " << root << std::endl;
	}

	{
		using Scalar = posit<64, 3>;
		Scalar a = -10;
		Scalar b = 20;
		std::cout << "The function used is x ^ 3 - 2x ^ 2 + 3\n";
		std::cout << "a = " << a << '\n';
		std::cout << "b = " << b << '\n';
		Scalar root = bisection(a, b, &fnctn, std::numeric_limits<Scalar>::epsilon());
		std::cout << "root = " << root << std::endl;
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
