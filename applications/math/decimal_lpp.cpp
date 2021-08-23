//  decimal_lpp.cpp : algorithm to find the largest palindrome product using decimal number system
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
// decimal number system
#include <universal/number/decimal/decimal.hpp>

/*
 * A palindrome number reads the same both ways. The largest palindrome made from the product
 * of two 2-digit numbers is 9009 = 91 x 99.
 *
 * Find the largest palindrome made from the product of two n-digit numbers.
 */

sw::universal::decimal convert(std::string& palindrome) {
	sw::universal::decimal p;
	if (p.parse(palindrome)) {
		return p;
	}
	return sw::universal::decimal(0);
}

bool isPalindrome(const sw::universal::decimal& suspectedPalindrome) {
	std::string s1 = to_string(suspectedPalindrome);
	std::string s2(s1);
	std::reverse(s2.begin(), s2.end());
	return (s1 == s2);
}

bool LargestPalindromeProduct(const sw::universal::decimal& nrDigits) {
	using namespace sw::universal;

	std::stringstream ss;
	for (long i = 0; i < long(nrDigits); ++i) {
		ss << '9';
	}
	decimal max;
	max.parse(ss.str());

	decimal nrOfSteps = 0;
	decimal largestPalindrome = 0;
	for (decimal i = max; i >= 0; --i) {
		for (decimal j = max; j >= 0; --j) {
			++nrOfSteps;
			decimal possiblePalindrome = i * j;
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

	using Decimal = decimal;

	Decimal nrDigits = 1;

	LargestPalindromeProduct(nrDigits++);
	LargestPalindromeProduct(nrDigits++);
	LargestPalindromeProduct(nrDigits++);
	LargestPalindromeProduct(nrDigits++);
	LargestPalindromeProduct(nrDigits++);

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
	std::cerr << "Caught exception: " << msg << std::endl;
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
