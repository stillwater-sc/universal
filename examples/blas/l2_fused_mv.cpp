// l2_fused_mv.cpp example program to demonstrate BLAS L2 Reproducible Matrix-Vector product
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "common.hpp"
#include <posit>
#include "blas_operators.hpp"

int main(int argc, char** argv)
try {
	const size_t nbits = 8;
	const size_t es = 0;
	const size_t vecSize = 32;

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
