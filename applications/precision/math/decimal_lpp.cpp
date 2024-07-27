// edecimal_lpp.cpp : algorithm to find the largest palindrome product using adaptive precision decimal number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>

#include <universal/number/edecimal/edecimal.hpp>

/*
 * A palindrome number reads the same both ways. The largest palindrome made from the product
 * of two 2-digit numbers is 9009 = 91 x 99.
 *
 * Find the largest palindrome made from the product of two n-digit numbers.
 */

sw::universal::edecimal convert(std::string& palindrome) {
	sw::universal::edecimal p;
	if (p.parse(palindrome)) {
		return p;
	}
	return sw::universal::edecimal(0);
}

bool isPalindrome(const sw::universal::edecimal& suspectedPalindrome) {
	std::string s1 = to_string(suspectedPalindrome);
	std::string s2(s1);
	std::reverse(s2.begin(), s2.end());
	return (s1 == s2);
}

bool LargestPalindromeProduct(const sw::universal::edecimal& nrDigits) {
	using namespace sw::universal;

	std::stringstream ss;
	for (long i = 0; i < long(nrDigits); ++i) {
		ss << '9';
	}
	edecimal max;
	max.parse(ss.str());

	edecimal nrOfSteps = 0;
	edecimal largestPalindrome = 0;
	for (edecimal i = max; i >= 0; --i) {
		for (edecimal j = max; j >= 0; --j) {
			++nrOfSteps;
			edecimal possiblePalindrome = i * j;
			if (isPalindrome(possiblePalindrome)) {
				if (largestPalindrome < possiblePalindrome) {
					largestPalindrome = possiblePalindrome;
					std::cout << possiblePalindrome << '\n';
				}
			}
			if (possiblePalindrome < largestPalindrome) break;
		}
	}
	std::cout << "In step " << nrOfSteps << " found largest " << nrDigits << "-digit palindrome product: " << largestPalindrome << std::endl;

	return false;
}

int main()
try {
	using namespace sw::universal;

	using Decimal = edecimal;

	Decimal nrDigits = 1;

	LargestPalindromeProduct(nrDigits++);
	LargestPalindromeProduct(nrDigits++);
	LargestPalindromeProduct(nrDigits++);
#ifdef STRESS_TESTING
	LargestPalindromeProduct(nrDigits++);
	LargestPalindromeProduct(nrDigits++);
#endif
	/*
		9009
		In step 145 found largest 2-digit palindrome product: 9009
		90909
		580085
		906609
		In step 9338 found largest 3-digit palindrome product: 906609
		99000099
		In step 14950 found largest 4-digit palindrome product: 99000099
		990090099
		5866006685
		8873113788
		9966006699
		In step 1112574 found largest 5-digit palindrome product: 9966006699
		999000000999
		In step 1499500 found largest 6-digit palindrome product: 999000000999
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
