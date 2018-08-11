// l3_fused_mv.cpp example program to demonstrate BLAS L3 Reproducible Matrix-Matrix product
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
// enable the following define to show the intermediate steps in the fused-dot product
// #define POSIT_VERBOSE_OUTPUT
#define QUIRE_TRACE_ADD
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <posit>
#include "blas_operators.hpp"

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

	{
		using value_type = sw::unum::posit<8, 0>;
		constexpr size_t n = 3;
		vector<value_type> A(n*n), B(n*n), C(n*n);
		randomVectorFillAroundOneEPS(n*n, A, 3);
		randomVectorFillAroundOneEPS(n*n, B, 3);
		init(C, 0.0);
		matmul(A, B, C);
		printMatrix(cout, "A matrix", A);
		printMatrix(cout, "B matrix", B);
		printMatrix(cout, "C matrix", C);
		cout << setprecision(5) << endl;
	}

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