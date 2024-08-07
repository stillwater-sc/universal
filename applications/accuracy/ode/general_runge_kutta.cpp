// general_runge_kutta.cpp: program to solve odes with general RK method using coefficients from Butcher's table
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
// Author: Jacob Todd  jtodd1@une.edu
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
// #define _USE_MATH_DEFINES
// #include <cmath>
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

// our test function where dy/dx is f(x,y)
template<typename Scalar>
Scalar myFunc(const Scalar& x, const Scalar& y) {
	return (0.98*y);
}

// Generalized Runge-Kutta Method
// Use Butcher's table as argument and define ODE
template<typename Scalar>
Scalar GRK(Scalar b_table[5][5], Scalar (*f)(const Scalar&, const Scalar&), const Scalar h, Scalar x0, Scalar y0) {
    int s = sizeof(b_table[0])/sizeof(b_table[0][0]) - 1; // number of steps
    Scalar ks[4];
    std::fill(ks, ks + s, Scalar(0));

    for (int i = 0; i < s; ++i) {
        Scalar sum = 0;
        for(int j = 1; j <= s; ++j) {
            sum = sum + b_table[i][j] * ks[j - 1];
        }
        sum = h * sum;
        ks[i] = f(x0 + h * b_table[0][i], y0 + sum);
    }

    Scalar out = 0;
    for (int i = 1; i <= s; ++i) {
        out = out + b_table[s][i] * ks[i - 1];
    }
    out = out + y0;
	return out;
}

int main() 
try {
	using namespace sw::universal;
	{
		using Scalar = float;
		Scalar butcher[5][5] = {
			{0, 0, 0, 0, 0},
			{0.5, 0.5, 0, 0, 0},
			{0.5, 0, 0.5, 0, 0},
			{1, 0, 0, 1, 0},
			{0, Scalar(1)/Scalar(6), Scalar(1)/Scalar(3), Scalar(1)/Scalar(3), Scalar(1)/Scalar(6)}
		};
		Scalar h = 1;
		Scalar y0 = 1;
		Scalar x0 = 0;

		Scalar solution = GRK(butcher, myFunc, h, x0, y0);
		std::cout << "y(" << x0 + h << ") ~= " << solution << '\n';

		float true_sol = exp(0.98);
		std::cout << "true = " << true_sol << '\n';
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
