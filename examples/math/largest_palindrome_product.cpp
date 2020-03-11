//  largest_palindrome_product.cpp : algorithm to find the largest palindrome product
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <universal/integer/integer.hpp>

/*
 * A palinddrome number reads the same both ways. The largest palindrome made from the product
 * of two 2-digit numbers is 9009 = 91 x 99.
 *
 * Find the largest palindrome made from the product of two n-digit numbers.
 */

template<size_t nbits>
bool Largest2DigitPalindromeProduct() {
	using namespace std;
	using namespace sw::unum;

	using Integer = integer<nbits>;
	using Real = long double;

	int nrOfSteps = 0;
	// construct palindromes starting from the largest descending to the smallest
	for (Integer i = 9; i >= 0; --i) {
		for (Integer j = 9; j >= 0; --j) {
			// construct the palindrome
			Integer palindrome = (i * 1000) + (j * 100) + (j * 10) + i;
//			cout << palindrome << endl;
			// generate guidance on the scale of the product terms
			Real squareRoot = sqrt((long double)palindrome);
//			cout << "sqrt guidance is " << squareRoot << endl;
			Integer a, b, c;
			a = squareRoot;
			while (a < 100) {
				b = palindrome / a;
				if ((palindrome % a) == 0 && b < 100) {
					cout << "In step " << nrOfSteps << " found largest 2-digit palindrome product: " << a << " * " << b << " = " << palindrome << " check product: " << a * b << endl;
					return true;
				}
				else {
					++a;
					++nrOfSteps;
				}
			}
		}
	}
	return false; // didn't find a solution
}

template<size_t nbits>
bool Largest3DigitPalindromeProduct() {
	using namespace std;
	using namespace sw::unum;

	using Integer = integer<nbits>;
	using Real = long double;

	int nrOfSteps = 0;
	// construct palindromes starting from the largest descending to the smallest
	for (Integer i = 9; i >= 0; --i) {
		for (Integer j = 9; j >= 0; --j) {
			for (Integer k = 9; k >= 0; --k) {
				// construct the palindrome
				Integer palindrome = (i * 100000) + (j * 10000) + (k * 1000) + (k * 100) + (j * 10) + i;
// 				cout << palindrome << endl;
				// generate guidance on the scale of the product terms
				Real squareRoot = sqrt((long double)palindrome);
//				cout << "sqrt guidance is " << squareRoot << endl;
				Integer a, b, c;
				a = squareRoot;
				while (a < 1000) {
					b = palindrome / a;
					if ((palindrome % a) == 0 && b < 1000) {
						cout << "In step " << nrOfSteps << " found largest 3-digit palindrome product: " << a << " * " << b << " = " << palindrome << " check product: " << a * b << endl;
						return true;
					}
					else {
						++a;
						++nrOfSteps;
					}
				}
			}
		}
	}
	return false; // didn't find a solution
}

template<size_t nbits>
bool Largest4DigitPalindromeProduct() {
	using namespace std;
	using namespace sw::unum;

	using Integer = integer<nbits>;
	using Real = long double;

	int nrOfSteps = 0;
	// construct palindromes starting from the largest descending to the smallest
	for (Integer i = 9; i >= 0; --i) {
		for (Integer j = 9; j >= 0; --j) {
			for (Integer k = 9; k >= 0; --k) {
				for (Integer m = 9; m >= 0; --m) {
					// construct the palindrome
					Integer palindrome = (i * 10000000) + (j * 1000000) + (k * 100000) + (m * 10000) + (m * 1000) + (k * 100) + (j * 10) + i;
					// cout << palindrome << endl;
					// generate guidance on the scale of the product terms
					Real squareRoot = sqrt((long double)palindrome);
					// cout << "sqrt guidance is " << squareRoot << endl;
					Integer a, b;
					a = squareRoot;
					while (a < 10000) {
						b = palindrome / a;
						if ((palindrome % a) == 0 && b < 10000) {
							cout << "In step " << nrOfSteps << " found largest 4-digit palindrome product: " << a << " * " << b << " = " << palindrome << " check product: " << a * b << endl;
							return true;
						}
						else {
							++a;
							++nrOfSteps;
						}
					}
				}
			}
		}
	}
	return false; // didn't find a solution
}

int main()
try {
	using namespace std;
	using namespace sw::unum;

	constexpr size_t nbits = 32;
	using Integer = integer<nbits>;
	using Real = long double;

	Largest2DigitPalindromeProduct<nbits>();
	Largest3DigitPalindromeProduct<nbits>();
	Largest4DigitPalindromeProduct<nbits>();

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
