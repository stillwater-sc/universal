// gcd.cpp: greatest common divisor algorithm on arbitrary precision integers
//
// Binomial coefficients are useful to generate the inverse of a Hilbert matrix
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal number project, which is released under an MIT Open Source license.
#include <iostream>
#include <universal/integer/integer>

template<size_t nbits, typename BlockType>
sw::unum::integer<nbits, BlockType> greatest_common_divisor(const sw::unum::integer<nbits, BlockType>& a, const sw::unum::integer<nbits, BlockType>& b) {
	std::cout << "gcd(" << a << ", " << b << ")\n";
	return b.iszero() ? a : greatest_common_divisor(b, a % b);
}

// conditional compilation
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main() 
try {
	using namespace std;
	using namespace sw::unum;

#if MANUAL_TESTING

	integer<1024, uint32_t> a, b, c;
	a = 1234567890500;
	b = 92875085904958;
	c = a * b * 10;
	cout << greatest_common_divisor(a,c) << " a = " << a << endl;

	cout << gcd(a, c) << " a = " << a << endl;

	a = 252;
	b = 105;
	c = a * b;
	cout << gcd(a, b) << " answer should be 21" << endl;
	cout << gcd(a, c) << " answer should be 252" << endl;
	cout << gcd(b, c) << " answer should be 105" << endl;
	cout << gcd(a, gcd(b, c)) << endl;
	cout << gcd(b, gcd(a, c)) << endl;
	cout << gcd(c, gcd(a, b)) << endl;

#else // MANUAL_TESTING

	// GCD of three numbers is
	// gcd(a, b, c) == gcd(a, gcd(b, c)) == gcd(gcd(a, b), c) == gcd(b, gcd(a, c))


#if STRESS_TESTING

#endif // STRESS_TESTING
#endif // MANUAL_TESTING


	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
