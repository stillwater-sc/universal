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
	using namespace std;

	const size_t nbits = 8;
	const size_t es = 0;
	const size_t vecSize = 32;

	int nrOfFailedTestCases = 0;

	{
		vector<double> A = {
			1.0, 0.0, 0.0,
			0.0, 1.0, 0.0,
			0.0, 0.0, 1.0
		};
		vector<double> x = { DBL_EPSILON, DBL_EPSILON, DBL_EPSILON };
		vector<double> b(3);
		matvec(A, x, b);
		cout << setprecision(21);
		print(cout, 3, b, 1);
		cout << endl;
	}

	{
		// we can represent DBL_EPSILON with a 32-bit posit
		using posit_32_2 = sw::unum::posit<32, 2>;

		vector< posit_32_2 > A = {
			1.0, 0.0, 0.0,
			0.0, 1.0, 0.0,
			0.0, 0.0, 1.0
		};
		vector<posit_32_2> x = { DBL_EPSILON, DBL_EPSILON, DBL_EPSILON };
		vector<posit_32_2> b(3);
		matvec(A, x, b);
		print(cout, 3, b, 1);
		cout << endl;

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
