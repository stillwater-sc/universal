// l2_fused_mv.cpp example program to demonstrate BLAS L2 Reproducible Matrix-Vector product
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "common.hpp"
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <posit>
#include "blas_operators.hpp"

int main(int argc, char** argv)
try {
	using namespace std;

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
		cout << setprecision(5) << endl;
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
		cout << setprecision(21);
		print(cout, 3, b, 1);
		cout << setprecision(5) << endl;
	}

	{
		using posit_32_2 = sw::unum::posit<32, 2>;
		constexpr int n = 5;
		vector< posit_32_2 > A(n*n);
		randomVectorFillAroundOneEPS(n*n, A, 18);
		vector< posit_32_2 > x(n), b(n);
		randomVectorFillAroundZeroEPS(n, x, 0);
		matvec(A, x, b);		// use template inference to match to a fused dot product version when you use posits
		printMatrix(cout, "random matrix", A);
		cout << endl;
		print(cout, n, x, 1);
		cout << endl;
		print(cout, n, b, 1);
		cout << setprecision(5) << endl;
	}

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
