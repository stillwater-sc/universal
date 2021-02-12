// isrepresentable.cpp : test suite for representability test in different number systems
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/functions/isrepresentable.hpp>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>
#include <universal/number/integer/numeric_limits.hpp>


#define MANUAL_TESTING 1
#define STRESS_TESTING 0

// Test isRepresentable function across different number systems and scales
int main()
try {
	using namespace std;
	using namespace sw::universal;

	std::string tag = "representation tests failed";

#if MANUAL_TESTING
	std::cout << "Manual isRepresentable verfication" << std::endl;

	for (int a = 0; a < 5; ++a) {
		for (int b = 1; b < 5; ++b) {
			cout << a << "/" << b << " = " << float(a)/float(b) << " is " << (isRepresentable(a,b) ? "representable\n" : "is not representable\n");
		}
	}

	using int16 = integer<16>;
	for (int16 a = 0; a < 5; ++a) {
		for (int16 b = 1; b < 5; ++b) {
			cout << a << "/" << b << " = " << float(a)/float(b) << " is " << (isRepresentable(a,b) ? "representable\n" : "is not representable\n");
		}
	}

	integer<128> a, b;
	a = 123456789012;
	b = 210987654321;
	cout << a << "/" << b << " = " << (long double)(a) / (long double)(b) << " is " << (isRepresentable(a, b) ? "representable\n" : "is not representable\n");
	b = 210987654323;
	cout << a << "/" << b << " = " << (long double)(a) / (long double)(b) << " is " << (isRepresentable(a, b) ? "representable\n" : "is not representable\n");

	cout << "done" << endl;
	return EXIT_SUCCESS;
#else
	std::cout << "Representation verfication" << std::endl;


#if STRESS_TESTING

#endif // STRESS_TESTING

	cout << "done" << endl;
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
