//  largest_palindrome_product.cpp : algorithm to find the largest palindrome product
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <cmath>
#include <universal/number/integer/integer.hpp>
#include <universal/number/edecimal/edecimal.hpp>

/*
 * A palinddrome number reads the same both ways. The largest palindrome made from the product
 * of two 2-digit numbers is 9009 = 91 x 99.
 *
 * Find the largest palindrome made from the product of two n-digit numbers.
 */

template<size_t nbits>
bool Largest2DigitPalindromeProduct() {
	using namespace sw::universal;

	using Integer = integer<nbits>;
	using Real = double;

	int nrOfSteps = 0;
	// construct palindromes starting from the largest descending to the smallest
	for (Integer i = 9; i >= 0; --i) {
		for (Integer j = 9; j >= 0; --j) {
			// construct the palindrome
			Integer palindrome = (i * 1000) + (j * 100) + (j * 10) + i;
//			cout << palindrome << endl;
			// generate guidance on the scale of the product terms
			Real squareRoot = std::sqrt(double(palindrome));
//			cout << "sqrt guidance is " << squareRoot << endl;
			Integer a, b;
			a = squareRoot;
			while (a < 100) {
				b = palindrome / a;
				if ((palindrome % a) == 0 && b < 100) {
					std::cout << "In step " << nrOfSteps << " found largest 2-digit palindrome product: " << a << " * " << b << " = " << palindrome << " check product: " << a * b << '\n';
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
	using namespace sw::universal;

	using Integer = integer<nbits>;
	using Real = double;

	int nrOfSteps = 0;
	// construct palindromes starting from the largest descending to the smallest
	for (Integer i = 9; i >= 0; --i) {
		for (Integer j = 9; j >= 0; --j) {
			for (Integer k = 9; k >= 0; --k) {
				// construct the palindrome
				Integer palindrome = (i * 100000) + (j * 10000) + (k * 1000) + (k * 100) + (j * 10) + i;
// 				cout << palindrome << endl;
				// generate guidance on the scale of the product terms
				Real squareRoot = std::sqrt(double(palindrome));
//				cout << "sqrt guidance is " << squareRoot << endl;
				Integer a, b;
				a = squareRoot;
				while (a < 1000) {
					b = palindrome / a;
					if ((palindrome % a) == 0 && b < 1000) {
						std::cout << "In step " << nrOfSteps << " found largest 3-digit palindrome product: " << a << " * " << b << " = " << palindrome << " check product: " << a * b << '\n';
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
	using namespace sw::universal;

	using Integer = integer<nbits>;
	using Real = double;

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
					Real squareRoot = std::sqrt(double(palindrome));
					// cout << "sqrt guidance is " << squareRoot << endl;
					Integer a, b;
					a = squareRoot;
					while (a < 10000) {
						b = palindrome / a;
						if ((palindrome % a) == 0 && b < 10000) {
							std::cout << "In step " << nrOfSteps << " found largest 4-digit palindrome product: " << a << " * " << b << " = " << palindrome << " check product: " << a * b << '\n';
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

template<size_t nbits>
bool Largest5DigitPalindromeProduct() {
	using namespace sw::universal;

	using Integer = integer<nbits>;
	using Real = double;

	int nrOfSteps = 0;
	// construct palindromes starting from the largest descending to the smallest
	for (Integer i = 9; i >= 0; --i) {
		for (Integer j = 9; j >= 0; --j) {
			for (Integer k = 9; k >= 0; --k) {
				for (Integer m = 9; m >= 0; --m) {
					for (Integer n = 9; n >= 0; --n) {
						// construct the palindrome
						Integer palindrome = (i * 1000000000) + (j * 100000000) + (k * 10000000) + (m * 1000000) + (n * 100000) + (n * 10000) + (m * 1000) + (k * 100) + (j * 10) + i;
						// cout << palindrome << endl;
						// generate guidance on the scale of the product terms
						Real squareRoot = std::sqrt(double(palindrome));
						// cout << "sqrt guidance is " << squareRoot << endl;
						Integer a, b;
						a = squareRoot;
						while (a < 100000) {
							b = palindrome / a;
							if ((palindrome % a) == 0 && b < 100000) {
								std::cout << "In step " << nrOfSteps << " found largest 5-digit palindrome product: " << a << " * " << b << " = " << palindrome << " check product: " << a * b << '\n';
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
	}
	return false; // didn't find a solution
}

int main()
try {
	using namespace sw::universal;

	Largest2DigitPalindromeProduct<16>();
	Largest3DigitPalindromeProduct<24>();
	Largest4DigitPalindromeProduct<32>();
	Largest5DigitPalindromeProduct<64>();

	/*
	In step 30 found largest 2-digit palindrome product: 99 * 91 = 9009 check product: 9009
	In step 2287 found largest 3-digit palindrome product: 993 * 913 = 906609 check product: 906609
	In step 2556 found largest 4-digit palindrome product: 9999 * 9901 = 99000099 check product: 99000099
	In step 29072 found largest 5-digit palindrome product: 99979 * 99681 = 9966006699 check product: 9966006699
	 */

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
