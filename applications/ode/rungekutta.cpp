// rungekutta.cpp: program to solve odes with classic Runge-Kutta method
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// Author: Jacob Todd  jtodd1@une.edu
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#define _USE_MATH_DEFINES
#include <cmath>
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

// our test function where dy/dx is f(x,y)
template<typename Scalar>
Scalar myFunc(const Scalar& x, const Scalar& y) {
	return ((5*x*x - y)/exp ((x + y)));
}

// rk4 algorithm
// requires a two variable function (defined above), initial conditions for x and y,
// number of intervals across which to approximate y(x), and stepsize between intervals

template<typename Scalar>
void rk4(Scalar (*f)(const Scalar&, const Scalar&), size_t n, const Scalar& h, const Scalar& x0, const Scalar& y0) {
	using namespace std;
	Scalar y = y0;
	for (size_t i = 0; i <= n; i++) {
		Scalar x = x0 + i*h;
		auto f1 = h*f(x, y);
		auto f2 = h*f(x + h/2, y + f1/2);
		auto f3 = h*f(x + h/2, y + f2/2);
		auto f4 = h*f(x + h, y + f3);
		y = y + (f1 + 2*f2 + 2*f3 + f4)/6;
		cout << "y(" << x << ") ~= " << y << std::endl;
	}
	return;
}

int main() 
try {
	using namespace std;
	using namespace sw::universal;

	size_t N = 10; // number of intervals
	double h = M_PI_4; // step size between intervals

	{	
		using Scalar = float;
		Scalar x0 = 0; // initial x
		Scalar y0 = 1; // initial y
		Scalar h = Scalar(M_PI_4); // step size between intervals
		std::cout << "\nThe ode is: dy/dx = (5*x*x - y)/exp(x + y)\n" << std::endl;
		std::cout << "Using float" << std::endl;
		std::cout << "Appoximating y(x) from " << x0 << " to " << x0 + N * h << std::endl;
		std::cout << "step size = " << h << std::endl;
		rk4(&myFunc, N, h, x0, y0);
	}

	{	using Scalar = posit<16, 2>;
		Scalar x0 = 0; // initial x
		Scalar y0 = 1; // initial y
		Scalar h = Scalar(M_PI_4); // step size between intervals
		std::cout << "\nThe ode is: dy/dx = (5*x*x - y)/exp(x + y)\n" << std::endl;
		std::cout << "Using float" << std::endl;
		std::cout << "Appoximating y(x) from " << x0 << " to " << x0 + N * h << std::endl;
		std::cout << "step size = " << h << std::endl;
		rk4(&myFunc, N, h, x0, y0);
	}

	{	using Scalar = posit<16, 2>;
		Scalar x0 = 0; // initial x
		Scalar y0 = 1; // initial y
		std::cout << "\nThe ode is: dy/dx = (5*x*x - y)/exp(x + y)\n" << std::endl;
		std::cout << "Using posit<16, 1>" << std::endl;
		std::cout << "Appoximating y(x) from " << x0 << " to " << x0 + N*h << std::endl;
		std::cout << "step size = " << h << std::endl;
		rk4 (&myFunc, N, Scalar(h), x0, y0);
	}
	{
		using Scalar = posit<32, 2>;
		std::cout << "\nUsing posit<32, 1>" << std::endl;
		Scalar x0 = 0; // initial x
		Scalar y0 = 1; // initial y
		std::cout << "Appoximating y(x) from " << x0 << " to " << x0 + N*h << std::endl;
		std::cout << "step size = " << h << std::endl;
		rk4 (&myFunc, N, Scalar(h), x0, y0);
	}
	{
		using Scalar = posit<64, 2>;
		std::cout << "\nUsing posit<64, 1>" << std::endl;
		Scalar x0 = 0; // initial x
		Scalar y0 = 1; // initial y
		std::cout << "Appoximating y(x) from " << x0 << " to " << x0 + N*h << std::endl;
		std::cout << "step size = " << h << std::endl;
		rk4 (&myFunc, N, Scalar(h), x0, y0);
	}

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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

