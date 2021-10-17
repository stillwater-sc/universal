//  representable.cpp : check if a ratio is representable
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <limits>
// is representable
#include <universal/functions/isrepresentable.hpp>

// enumerate a couple ratios to test representability
void ReproducibilityTestSuite() {
	for (int i = 0; i < 30; i += 3) {
		for (int j = 0; j < 70; j += 7) {
			sw::universal::reportRepresentability(i, j);
		}
	}
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string tag = "failed";

	ReproducibilityTestSuite();

#if LONG_DOUBLE_SUPPORT
	constexpr long double denorm_min = std::numeric_limits<long double>::denorm_min();
	std::cout << "smallest long double: " << to_binary(denorm_min) << " : " << denorm_min << '\n';
#endif
	constexpr long double denorm_min = std::numeric_limits<long double>::denorm_min();
	std::cout << "smallest long double: " << denorm_min << '\n';
	std::cout << "done" << std::endl;

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
