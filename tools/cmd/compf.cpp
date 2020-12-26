// compf.cpp: components of a float: cli to show the sign/scale/fraction components of a float 
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/value/value>

// receive a float and print its components
int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	// float attributes
	constexpr int max_digits10 = std::numeric_limits<double>::max_digits10;
	constexpr int fbits = std::numeric_limits<float>::digits - 1;

	if (argc != 2) {
		cerr << "compf : components of an IEEE single-precision float\n";
		cerr << "Show the sign/scale/fraction components of an IEEE float.\n";
	    cerr << "Usage: compf float_value\n";
		cerr << "Example: compf 0.03124999\n";
		cerr << "float: 0.031249990686774254 (+,-6,11111111111111111111011)" << endl;
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}
	float f = float(atof(argv[1]));
	value<fbits> v(f);

	cout << "float: " << setprecision(max_digits10) << f << " " << components(v) << endl;
	return EXIT_SUCCESS;
}
catch (const char* const msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
