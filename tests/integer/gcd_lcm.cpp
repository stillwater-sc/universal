// gcd_lcm.cpp: greatest common divisor and least common multiple tests on arbitrary precision integers
//
// Binomial coefficients are useful to generate the inverse of a Hilbert matrix
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal number project, which is released under an MIT Open Source license.
#include <iostream>
#include <universal/integer/integer>

template<size_t nbits, typename BlockType>
sw::universal::integer<nbits, BlockType> greatest_common_divisor(const sw::universal::integer<nbits, BlockType>& a, const sw::universal::integer<nbits, BlockType>& b) {
	std::cout << "gcd(" << a << ", " << b << ")\n";
	return b.iszero() ? a : greatest_common_divisor(b, a % b);
}

// conditional compilation
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main() 
try {
	using namespace std;
	using namespace sw::universal;

#if MANUAL_TESTING

	using Integer = integer<1024, uint32_t>;
	Integer a, b, c;
	a = 1234567890500;
	b = 92875085904958;
	c = a * b * 10;
	cout << greatest_common_divisor(a,c) << " a = " << a << endl;
	cout << sw::universal::gcd(a, c) << " a = " << a << endl;

	a = 252;
	b = 105;
	c = a * b;
	cout << "gcd(" << a << "," << b << ") = " << gcd(a, b) << " answer should be 21" << endl;
	cout << "gcd(" << a << "," << c << ") = " << gcd(a, c) << " answer should be 252" << endl;
	cout << "gcd(" << b << "," << c << ") = " << gcd(b, c) << " answer should be 105" << endl;
	cout << "gcd(" << a << "," << gcd(b, c) << ") = " << gcd(a, gcd(b, c)) << endl;
	cout << "gcd(" << a << "," << gcd(a, c) << ") = " << gcd(b, gcd(a, c)) << endl;
	cout << "gcd(" << a << "," << gcd(a, b) << ") = " << gcd(c, gcd(a, b)) << endl;

	vector< Integer > v;
	v.push_back(a);
	v.push_back(b);
	v.push_back(c);
	cout << gcd(v) << endl;

	a = 3;
	b = 7;
	c = a * b;
	cout << "lcm(" << a << "," << b << ") = " << lcm(a, b) << " answer should be 21" << endl;
	cout << "lcm(" << a << "," << lcm(b, c) << ") = " << lcm(a, lcm(b, c)) << endl;
	cout << "lcm(" << a << "," << lcm(a, c) << ") = " << lcm(b, lcm(a, c)) << endl;
	cout << "lcm(" << a << "," << lcm(a, b) << ") = " << lcm(c, lcm(a, b)) << endl;

	v.clear();
	v.push_back(2);
	v.push_back(3);
	v.push_back(4);
	v.push_back(5);
	v.push_back(6);
	v.push_back(7);
	v.push_back(8);
	v.push_back(9);
	v.push_back(10);
	v.push_back(11);
	v.push_back(12);
	v.push_back(13);
	v.push_back(14);
	v.push_back(15);
	cout << "lcm( 2 through 15 ) = " << lcm(v) << endl;
	v.push_back(16);
	v.push_back(17);
	cout << "lcm( 2 through 17 ) = " << lcm(v) << endl;
	v.push_back(18);
	v.push_back(19);
	cout << "lcm( 2 through 19 ) = " << lcm(v) << endl;
	v.push_back(20);
	v.push_back(21);
	cout << "lcm( 2 through 21 ) = " << lcm(v) << endl;
	v.push_back(22);
	cout << "lcm( 2 through 22 ) = " << lcm(v) << endl;
	v.push_back(91);
	cout << "lcm( 2 through 91 ) = " << lcm(v) << endl;

	Integer leastCM = lcm(v);
	cout << leastCM / 17 << " " << leastCM % 17 << endl;
	cout << leastCM / 21 << " " << leastCM % 21 << endl;
	cout << leastCM / 91 << " " << leastCM % 91 << endl;

	cout << endl;

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
