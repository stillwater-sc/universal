// compfp.cpp: components of a fixed-point: cli to show the sign/scale/fraction components of a fixed-point value 
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <typeinfo>
#include <universal/fixpnt/fixpnt>

// receive a float and print its components
int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc != 2) {
		cerr << "compfp : components of a fixed-point value\n";
		cerr << "Show the sign/scale/fraction components of a fixed-point value.\n";
	    cerr << "Usage: compfp float_value\n";
		cerr << "Example: compfp 1.0625\n";
		cerr << "class sw::universal::fixpnt<32,16,1,unsigned char>: 1.0625000000000000 b0000000000000001.0001000000000000" << endl;
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}
	using FixedPoint = fixpnt<32,16>;
	FixedPoint v = FixedPoint(atof(argv[1]));

	// fixed-point attributes
	constexpr int max_digits10 = std::numeric_limits<FixedPoint>::max_digits10;

	cout << typeid(FixedPoint).name() << ": "  << setprecision(max_digits10) << v << " " << to_binary(v) << endl;

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
