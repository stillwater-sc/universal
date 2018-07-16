// l1_axpy.cpp: example program contrasting a BLAS L1 ?axpy routine between FLOAT and POSIT
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"

#include <vector>
#include <posit>
#include "blas_utils.hpp"

// LEVEL 1 BLAS operator
// a time x plus y
template<typename scale_T, typename vector_T>
void axpy(size_t n, scale_T a, const vector_T& x, size_t incx, vector_T& y, size_t incy) {
	size_t cnt, ix, iy;
	for (cnt = 0, ix = 0, iy = 0; cnt < n && ix < x.size() && iy < y.size(); ++cnt, ix += incx, iy += incy) {
		y[iy] += a * x[ix];
	}
}


int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	//constexpr size_t vecSize = 32;

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
