// l3_fused_mv.cpp example program to demonstrate BLAS L3 Reproducible Matrix-Matrix product
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
#include <posit>
#include "blas_operators.hpp"

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;
	const size_t nbits = 8;
	const size_t es = 0;

	int nrOfFailedTestCases = 0;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
