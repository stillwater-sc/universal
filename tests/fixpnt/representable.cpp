//  representable.cpp : check if a ratio is representable
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
// is representable
#include <universal/functions/isrepresentable.hpp>

// enumerate a couple ratios to test representability
void ReproducibilityTestSuite() {
	for (int i = 0; i < 30; i += 3) {
		for (int j = 0; j < 70; j += 7) {
			sw::unum::reportRepresentability(i, j);
		}
	}
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::unum;

	std::string tag = "failed";

	ReproducibilityTestSuite();

	cout << "done" << endl;

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
