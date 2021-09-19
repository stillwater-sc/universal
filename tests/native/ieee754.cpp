// ieee754.cpp : native IEEE-754 operations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/native/ieee754.hpp>

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	// compare bits of different real number representations
	
	float f         = 1.0e1;
	double d        = 1.0e10;
#if LONG_DOUBLE_SUPPORT
	long double ld  = 1.0e100;
#else
	std::cout << "This environment does not support a native long double format\n";
#endif

	std::cout << "scale of " << f << " is 2^" << scale(f) << " ~ 10^" << int(scale(f)/ 3.3) << '\n';
	std::cout << "scale of " << d << " is 2^" << scale(d) << " ~ 10^" << int(scale(d) / 3.3) << '\n';
#if LONG_DOUBLE_SUPPORT
	std::cout << "scale of " << ld << " is 2^" << scale(ld) << " ~ 10^" << int(scale(ld) / 3.3) << '\n';
#endif

	std::cout << to_binary(f, true) << " " << f << '\n';
	std::cout << to_binary(d, true) << " " << d << '\n';
#if LONG_DOUBLE_SUPPORT
	std::cout << to_binary(ld, true) << " " << ld << '\n';
#endif

	std::cout << color_print(f) << " " << f << '\n';
	std::cout << color_print(d) << " " << d << '\n';
#if LONG_DOUBLE_SUPPORT
	std::cout << color_print(ld) << " " << ld << '\n';
#endif
	valueRepresentations(f);
	valueRepresentations(d);
#if LONG_DOUBLE_SUPPORT
	valueRepresentations(ld);
#endif

	std::cout << std::endl; // flush the stream

	return EXIT_SUCCESS;
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
