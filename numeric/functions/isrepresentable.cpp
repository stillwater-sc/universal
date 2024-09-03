// isrepresentable.cpp : test suite for representability test in different number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/utility/long_double.hpp>
#include <universal/math/functions/isrepresentable.hpp>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>
#include <universal/number/integer/numeric_limits.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

// Test isRepresentable function across different number systems and scales
int main()
try {
	using namespace sw::universal;

	std::string tag = "representation tests failed";

#if MANUAL_TESTING
	std::cout << "Manual isRepresentable verfication\n";

	for (int a = 0; a < 5; ++a) {
		for (int b = 1; b < 5; ++b) {
			std::cout << a << "/" << b << " = " << float(a)/float(b) << " is " << (isRepresentable(a,b) ? "representable\n" : "is not representable\n");
		}
	}

	using int16 = integer<16>;
	for (int16 a = 0; a < 5; ++a) {
		for (int16 b = 1; b < 5; ++b) {
			std::cout << a << "/" << b << " = " << float(a)/float(b) << " is " << (isRepresentable(a,b) ? "representable\n" : "is not representable\n");
		}
	}

#if LONG_DOUBLE_SUPPORT
	integer<128> a, b;
	a = 123456789012;
	b = 210987654321;
	std::cout << a << "/" << b << " = " << (long double)(a) / (long double)(b) << " is " << (isRepresentable(a, b) ? "representable\n" : "is not representable\n");
	b = 210987654323;
	std::cout << a << "/" << b << " = " << (long double)(a) / (long double)(b) << " is " << (isRepresentable(a, b) ? "representable\n" : "is not representable\n");
#endif

	std::cout << "done" << std::endl;
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
