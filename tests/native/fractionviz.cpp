//  fractionviz.cpp : fraction bits visualization of native IEEE-754 types
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
	using namespace std;
	using namespace sw::universal;

	// compare bits of different real number representations
	
	float f         = 1.0e10;
	double d        = 1.0e10;
	long double ld  = 1.0e10;

	cout << "single precision float     : " << color_print(f) << endl;
	cout << "double precision float     : " << color_print(d) << endl;
	cout << "long double precision float: " << color_print(ld) << endl;

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
