//  fractionviz.cpp : fraction bits visualization of real types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/posit/posit>
//#include <universal/native/ieee754.h>

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
	posit<32,2> p32 = 1.0e10;
	posit<64,2> p64 = 1.0e10;

	cout << color_print(f) << endl;
	cout << color_print(d) << endl;
	cout << color_print(ld) << endl;
	cout << color_print(p32) << endl;
	cout << color_print(p64) << endl;

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
