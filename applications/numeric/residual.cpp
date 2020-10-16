// residual.cpp: example program to show exact residual calucation using the quire
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <ostream>
#include <limits>
#include <numeric>   // nextafter

// select the number systems we would like to compare
#include <universal/integer/integer>
#include <universal/fixpnt/fixpnt>
#include <universal/areal/areal>
#include <universal/posit/posit>
#include <universal/lns/lns>

#include <universal/blas/blas>
#include <universal/blas/generators/frank.hpp>

template<typename Scalar>
void FrankMatrixTest(size_t N) {
	using namespace std;
	using Vector = sw::unum::blas::vector<Scalar>;
	using Matrix = sw::unum::blas::matrix<Scalar>;
	Matrix A = sw::unum::blas::frank<Scalar>(N);
	cout << "Frank matrix order " << N << endl;
	Vector b(N), x(N);
	x = Scalar(1);
	b = A * x;
//	cout << "right hand side [" << b << "]\n";
	x = solve(A, b);
//	cout << "solution vector [" << x << "]\n";
	Vector e = A * x - b;
	cout << "1-norm of error vector: " << norm1(e) << endl;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	cout << "Residual calculations\n";

	streamsize precision = cout.precision();

	vector<size_t> sizes = { 5, 15, 45, 95 };
	for (auto N : sizes) {
		FrankMatrixTest<float>(N);
		FrankMatrixTest<posit<32, 2>>(N);
	}

	cout << setprecision(precision);
	cout << endl;
	
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
