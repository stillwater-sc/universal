// first_order_ode.cpp: program to compare different numerical solvers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#define _USE_MATH_DEFINES
#include <cmath>
// Configure the posit library with arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>

/*

A first-order differential equation is defined by an equation: 
         dy/dx = f(x,y) 
of two variables x and y with its function f(x,y) defined on a 
region in the xy-plane. It has only the first derivative dy/dx 
so that the equation is of the first order and no higher-order 
derivatives exist. The differential equation in first-order can 
also be written as;

               y’ = f(x,y)   or
        (d/dx) y  = f(x,y)

The differential equation is generally used to express a relation 
between the function and its derivatives. In Physics and chemistry, 
it is used as a technique for determining the functions over its 
domain if we know the functions and some of the derivatives.

First Order Linear Differential Equation

If the function f is a linear expression in y, then the first-order 
differential equation y’ = f(x, y) is a linear equation. That is, 
the equation is linear and the function f takes the form

          f(x,y) = p(x)y + q(x)

Since the linear equation is y = mx+b

where p and q are continuous functions on some interval I. 
Differential equations that are not linear are called 
nonlinear equations.

Consider the first-order differential equation 
               y’ = f(x,y),  
is a linear equation and it can be written in the form
               y’ + a(x)y = f(x)
where a(x) and f(x) are continuous functions of x

The alternate method to represent the first-order linear equation 
in a reduced form is

            (dy/dx) + P(x)y = Q (x)

Where P(x) and Q(x) are the functions of x which are the 
continuous functions. If P(x) or Q(x) is equal to zero, 
the differential equation is reduced to the variable 
separable form and can be solved analytically

There are basically five types of differential equations in 
the first order. They are:
  1. Linear Differential Equations
  2. Homogeneous Equations
  3. Exact Equations
  4. Separable Equations
  5. Integrating Factor

The goal of this example is to juxtapose the different numerical methods
and measure their error
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
void rk4(Scalar (*f)(const Scalar&, const Scalar&), int n, Scalar& h, Scalar& x, Scalar& y) {
	const Scalar x0 = x;
	for (int i = 0; i < n + 1; i++) {
		x = x0 + i*h;
		auto f1 = h*f(x, y);
		auto f2 = h*f(x + h/2, y + f1/2);
		auto f3 = h*f(x + h/2, y + f2/2);
		auto f4 = h*f(x + h, y + f3);
		y = y + (f1 + 2*f2 + 2*f3 + f4)/6;
		std::cout << "y(" << x << ") ~= " << y << std::endl;
	}
	return;
}

int main() 
try {
	using namespace sw::universal;

	{	
		using Scalar = float;
		Scalar x0 = 0; // initial x
		Scalar y0 = 1; // initial y
		Scalar h = Scalar(M_PI_4); // step size between intervals
		int n = 4;  // number of intervals
		std::cout << "\nThe ode is: dy/dx = (5*x*x - y)/exp(x + y)\n" << std::endl;
		std::cout << "Using float" << std::endl;
		std::cout << "Appoximating y(x) from " << x0 << " to " << x0 + n * h << std::endl;
		std::cout << "step size = " << h << std::endl;
		rk4(&myFunc, n, h, x0, y0);
	}

	{	using Scalar = posit<16, 2>;
		Scalar x0 = 0; // initial x
		Scalar y0 = 1; // initial y
		Scalar h = Scalar(M_PI_4); // step size between intervals
		int n = 4;  // number of intervals
		std::cout << "\nThe ode is: dy/dx = (5*x*x - y)/exp(x + y)\n" << std::endl;
		std::cout << "Using posit<16, 1>" << std::endl;
		std::cout << "Appoximating y(x) from " << x0 << " to " << x0 + n*h << std::endl;
		std::cout << "step size = " << h << std::endl;
		rk4 (&myFunc, n, h, x0, y0);
	}
	{
		using Scalar = posit<32, 2>;
		std::cout << "\nUsing posit<32, 1>" << std::endl;
		Scalar x0 = 0; // initial x
		Scalar y0 = 1; // initial y
		Scalar h = Scalar(M_PI_4); // step size between intervals
		int n = 4;  // number of intervals
		std::cout << "Appoximating y(x) from " << x0 << " to " << x0 + n*h << std::endl;
		std::cout << "step size = " << h << std::endl;
		rk4 (&myFunc, n, h, x0, y0);
	}
	{
		using Scalar = posit<64, 2>;
		std::cout << "\nUsing posit<64, 1>" << std::endl;
		Scalar x0 = 0; // initial x
		Scalar y0 = 1; // initial y
		Scalar h = Scalar(M_PI_4); // step size between intervals
		int n = 4;  // number of intervals
		std::cout << "Appoximating y(x) from " << x0 << " to " << x0 + n*h << std::endl;
		std::cout << "step size = " << h << std::endl;
		rk4 (&myFunc, n, h, x0, y0);
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

