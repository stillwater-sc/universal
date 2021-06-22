// pascals_triangle.cpp: example program to show binomial coefficients
//
// Binomial coefficients are useful to generate the inverse of a Hilbert matrix
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal number project, which is released under an MIT Open Source license.
#include <iostream>
#include <universal/number/integer/integer.hpp>
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/functions/binomial.hpp>

std::string spacing(unsigned n) {
	std::stringstream ss;
	for (unsigned i = 0; i < n; ++i) {
		ss << ' ';
	}
	return ss.str();
}

// Generate Pascal's triangle
// the bottom layer of the triangle has N+1 values, and given a columns width we can calculate the
// center of the pyramid.
template<typename Scalar>
void PascalsTriangle(Scalar N) {
	std::cout << "Pascal's Triangle for binomial coefficients of the " << N << "th order\n";
	std::cout << "Computed with type: " << typeid(N).name() << '\n';
	int widths[] = { 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7 };
	int order = int(N);
	int columnWidth = unsigned(order) < (sizeof(widths) / sizeof(int)) ? widths[order] : 10;
	int width = (order + 1) * (columnWidth + 2);
	int middle = width / 2;
	int leftMargin = middle;
	Scalar n, k;
	std::cout << spacing(leftMargin) << "             1\n";
	leftMargin -= columnWidth/2;
	//// GOTCHA!!!!! we have redefined the increment/decrement operators for posits to be working on ULP. Here we want pure integer behavior, so we need to be explicity if we want to support posits as Scalar
	for (n = 1; n <= N; n += 1) {
		std::cout << "n = " << std::setw(3) << n << spacing(leftMargin);
		for (k = 0; k <= n; k += 1) {
			std::cout << std::setw(columnWidth) << std::right << sw::function::binomial(n, k) << ' ';
		}
		leftMargin -= columnWidth/2;
		std::cout << '\n';
	}
}

// Enumerate the binomial coefficients of order n
template<typename Scalar>
void Binomials(Scalar n) {
	Scalar k;
	//// NOTE!!!!! the posit library has redefined the increment/decrement operators for posits to work at the ULP (Unit in Last Position). 
	//// Here we want pure integer behavior, so we need to be explicit in our adding of the integer value of 1
	//// if we want to use posits as an integer scalar.
	for (k = 0; k <= n; k = k + 1) {
		std::cout << "Binomial(" << std::setw(3) << n << "," << std::setw(3) << k << ") = " << std::setw(10) << sw::function::binomial(n, k) << std::endl;
	}
}

// conditional compilation
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main() 
try {
	using namespace std;
	using namespace sw::function;

#if MANUAL_TESTING
	using int128_t = sw::universal::integer<128>;
	using posit = sw::universal::posit<32, 2>;

	PascalsTriangle(long(20));
	PascalsTriangle(int128_t(20));
	PascalsTriangle(posit(20));
	
	Binomials(posit(21));

#else // MANUAL_TESTING
	using int128_t = sw::universal::integer<128>;
	using posit = sw::universal::posit<32, 2>;

	PascalsTriangle(int128_t(15));
	PascalsTriangle(posit(15));

	Binomials(10);

#if STRESS_TESTING
	PascalsTriangle(long(20));
	PascalsTriangle(int128_t(20));
	PascalsTriangle(posit(20));

	Binomials(posit(21));

#endif // STRESS_TESTING
#endif // MANUAL_TESTING


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
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
