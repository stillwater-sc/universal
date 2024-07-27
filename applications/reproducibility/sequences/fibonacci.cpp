// fibonacci.cpp: experiments with representing Fibonacci sequences
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/integer/integer.hpp>
#include <universal/sequences/fibonacci.hpp>

int main(int argc, char** argv)
try {
	using namespace sw::universal;
	using namespace sw::sequences;

	int nrOfFailedTestCases = 0;

	using int256 = sw::universal::integer<256>;
	auto v = Fibonacci<int256>(100);
	std::cout << "Fibonacci sequence\n";
	for (auto e: v) { std::cout << e << '\n'; }

	//streamsize precision = cout.precision();
	// ...
	//cout << setprecision(precision);

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
