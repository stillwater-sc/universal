//  stirlings_approximation.cpp : Stirling's approximation for factorials
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
// multi-precision float
//#include <universal/mpf/mpf.hpp>
#include <universal/posit/posit>
#include <universal/functions/factorial.hpp>

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

template<typename Real>
Real StirlingsApproximation(const Real& n) {
	Real pi = 3.14159265358979323846;
	Real term1 = sqrt(Real(2) * pi * n);
	Real e = 2.71828182845904523536;
	Real term2 = pow(n / e, n);

	Real factorial = term1 * term2;
	return factorial;
}

int main()
try {
	using namespace std;
	using namespace sw::unum;

	//using Real = mpf;
	using Real = posit<32,2>;

	for (Real i = 1; i < 20; i += 1) {
		cout << setw(2) << i << "! = " 
			<< setw(20) << StirlingsApproximation(i) << "  " 
			<< setw(20) << sw::function::factorial(i) << endl;
	}

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
