// l1_asum.cpp: example program contrasting a BLAS L1 ?asum routine between FLOAT and POSIT
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"

#include <vector>
#include <posit>

#include "blas.hpp"

using namespace std;
using namespace sw::unum;

int main(int argc, char** argv)
try {
	//constexpr size_t nbits = 16;
	//constexpr size_t es = 1;
	//constexpr size_t vecSize = 32;

	int nrOfFailedTestCases = 0;

	cout << "ASUM is " << 1 << endl;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
	return EXIT_FAILURE;
}
