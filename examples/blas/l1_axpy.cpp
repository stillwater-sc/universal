// l1_axpy.cpp: example program contrasting a BLAS L1 ?axpy routine between FLOAT and POSIT
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"

#include <vector>
#include <posit>

#include "blas.hpp"


int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::blas;

	const size_t nbits = 16;
	const size_t es = 1;
	const size_t vecSize = 32;

	int nrOfFailedTestCases = 0;

	constexpr int d = 5;
	vector< posit<nbits, es> > v1 = { 1.0, 2.0, 3.0, 4.0, 5.0 };
	vector< posit<nbits, es> > v2(d);
	posit<nbits, es> alpha = minpos<nbits, es>();

	cout << "AXPY is " << endl;
	print(cout, d, v1, 1); cout << endl;
	print(cout, d, v2, 1); cout << endl;

	axpy(d, alpha, v1, 1, v2, 1);

	print(cout, d, v2, 1); cout << endl;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
