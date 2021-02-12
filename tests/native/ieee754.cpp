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
	using namespace std;
	using namespace sw::universal;

	// compare bits of different real number representations
	
	float f         = 1.0e1;
	double d        = 1.0e10;
	long double ld  = 1.0e100;

	cout << "scale of " << f << " is 2^" << scale(f) << " ~ 10^" << int(scale(f)/ 3.3) << '\n';
	cout << "scale of " << d << " is 2^" << scale(d) << " ~ 10^" << int(scale(d) / 3.3) << '\n';
	cout << "scale of " << ld << " is 2^" << scale(ld) << " ~ 10^" << int(scale(ld) / 3.3) << '\n';

	cout << to_binary(f, true) << " " << f << '\n';
	cout << to_binary(d, true) << " " << d << '\n';
	cout << to_binary(ld, true) << " " << ld << '\n';

	cout << color_print(f) << " " << f << '\n';
	cout << color_print(d) << " " << d << '\n';
	cout << color_print(ld) << " " << ld << '\n';

	cout << endl; // flush the stream

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
