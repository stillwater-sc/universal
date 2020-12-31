//  stirlings_approximation.cpp : Stirling's approximation for factorials
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
// the oracle
#include <universal/decimal/decimal>
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

template<typename Scalar>
Scalar StirlingsApproximation(size_t n) {
	Scalar pi = 3.14159265358979323846;
	Scalar term1 = sqrt(Scalar(2) * pi * Scalar(n));
	Scalar e = 2.71828182845904523536;
	Scalar term2 = pow(Scalar(n) / e, Scalar(n));

	Scalar factorial = term1 * term2;
	return factorial;
}

int main()
try {
	using namespace std;
	using namespace sw::universal;

	//using Real = mpf;
	using Real = posit<32,2>;
	using Integer = decimal;

	constexpr size_t FIRST_COLUMN = 10;
	constexpr size_t COLUMN_WIDTH = 40;
	cout << setw(FIRST_COLUMN) << "factorial"
		<< setw(COLUMN_WIDTH) << "Stirling's Approximation"
		<< setw(COLUMN_WIDTH) << "Real Approximation"
		<< setw(COLUMN_WIDTH) << "Actual Factorial\n";
	for (size_t i = 1; i < 30; i += 1) {
		cout << setw(FIRST_COLUMN) << i << "! = "
			<< setw(COLUMN_WIDTH) << StirlingsApproximation<Real>(i) << "\t"
			<< setw(COLUMN_WIDTH) << sw::function::factorial<Real>(i) << "\t"
			<< setw(COLUMN_WIDTH) << sw::function::factorial<Integer>(i) << endl;
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
