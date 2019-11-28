// isrepresentable.cpp : test suite for representability test in different number systems
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/functions/isrepresentable.hpp>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/integer/integer.hpp>
#include <universal/integer/numeric_limits.hpp>


#define MANUAL_TESTING 1
#define STRESS_TESTING 0

// Test isRepresentable function across different number systems and scales
int main()
try {
	using namespace std;
	using namespace sw::unum;

	std::string tag = "representation tests failed";

#if MANUAL_TESTING
	std::cout << "Manual isRepresentable verfication" << std::endl;

	for (int a = 0; a < 10; ++a) {
		for (int b = 1; b < 10; ++b) {
			cout << a << "/" << b << " = " << float(a)/float(b) << " is " << (isRepresentable(a,b) ? "representable\n" : "is not representable\n");
		}
	}
	/*
	TODO: need to implement literal comparisons and operator%()
	using int16 = integer<16>;
	for (int16 a = 0; a < 10; ++a) {
		for (int16 b = 1; b < 10; ++b) {
			cout << a << "/" << b << " = " << float(a)/float(b) << " is " << (isRepresentable(a,b) ? "representable" : "is not representable");
		}
	}
	*/
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
