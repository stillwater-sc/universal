//  chem_equilibrium.cpp : example of calculating the chemical balance of a solution
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <universal/posit/posit>

/*
 * Simple chemical equilibrium
 *
 * 2*x1 + x2 + x3 + 2*x4        = 110
 *   x1      + x3        + 2*x5 = 55
 *        x2 - x3               = 0
 *
 *        x2 * x3                   = 10^-14.94
 *                    x4 * sqrt(x5) = 10^-50.48
 *
 * From the linear equations, one can deduce that 
 *        x2 = x3
 *        x4 = 2*x5
 *
 * From these you can solve the non-linear equations.
 * Both x4 and x5 will have very small values, ~ 1e-34
 */

int main()
try {
	using namespace std;
	using namespace sw::universal;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Posit = posit<nbits, es>;

	Posit x2, x4, x5;
	Posit c1, c2;

	// x2 * x3 = 1e-14.94, x2 = x3 => 2* x2 = 1e-14.94
	c1 = pow(10, -14.94);
	x2 = c1 / 2;

	// x4 * sqrt(x5) = 1e-50.48, x4 = 2*x5 => x4 * sqrt(2*x4) = c2 =>
	c2 = pow(10, -50.48);
	x5 = c2;
	x4 = 2 * x5;

	cout << "x2 = " << x2 << endl;
	cout << "x4 = " << x4 << endl;
	cout << "x5 = " << x5 << endl;
	
    
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
