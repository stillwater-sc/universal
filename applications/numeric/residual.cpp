// residual.cpp: example program to show exact residual calucation using the quire
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
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
#include <universal/blas/generators/hilbert.hpp>

/*
template<typename Scalar>
sw::unum::blas::vector<Scalar> residual(const sw::unum::blas::matrix<Scalar>& A, const sw::unum::blas::vector<Scalar>& x, const sw::unum::blas::vector<Scalar>& b) {
	using namespace sw::unum;
	using namespace sw::unum::blas;
	using Vector = sw::unum::blas::vector<Scalar>;
	size_t M = num_rows(A);
	size_t N = num_cols(A);
	Vector r(M);
	for (size_t i = 0; i < M; ++i) {
		quire<Scalar> q(-b);
		for (size_t j = 0; j < N; ++j) {
			q += quire_mul(A(i, j), x(j));
		}
		r(i) = q.to_value();
	}
	return r;
}
*/

template<size_t nbits, size_t es, size_t capacity = 10>
sw::unum::blas::vector<sw::unum::posit<nbits, es>> residual(const sw::unum::blas::matrix<sw::unum::posit<nbits, es>>& A, const sw::unum::blas::vector<sw::unum::posit<nbits, es>>& x, const sw::unum::blas::vector<sw::unum::posit<nbits, es>>& b) {
	using namespace sw::unum;
	using namespace sw::unum::blas;
	using Scalar = sw::unum::posit<nbits, es>;
	using Vector = sw::unum::blas::vector<Scalar>;
	size_t M = num_rows(A);
	size_t N = num_cols(A);
	Vector r(M);
	for (size_t i = 0; i < M; ++i) {
		quire<nbits, es, capacity> q(-b(i));
		for (size_t j = 0; j < N; ++j) {
			q += quire_mul(A(i, j), x(j));
		}
		convert(q.to_value(), r(i));
	}
	return r;
}

template<typename Scalar>
void FrankMatrixTest(int N) {
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

template<size_t nbits, size_t es>
void ResidualTest(const sw::unum::blas::matrix< sw::unum::posit<nbits, es> >& A) {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::unum::blas;

	using Scalar = posit<nbits, es>;
	using Vector = sw::unum::blas::vector<Scalar>;
	using Matrix = sw::unum::blas::matrix<Scalar>;

	size_t M = num_rows(A);
	size_t N = num_cols(A);
	if (M != N) {
		cerr << "Matrix should be square, but is (" << M << " by " << N << ")\n";
		return;
	}
	cout << "Matrix order " << N << endl;
	cout << setw(14) << A << endl;
	cout << hex_format(A) << endl;
	Vector b(N), ones(N), x(N);
	ones = Scalar(1);
	b = A * ones; // <-- posit specialized FDP matrix-vector multiply
	sw::unum::blas::vector<size_t> indx(N);
	Matrix LU(A); // the LU decomposition is in place, so create a copy first
	auto error = ludcmp(LU, indx);
	cout << "LU decomposition\n";
	cout << hex_format(LU) << endl;
	x = lubksb(LU, indx, b);
	cout << "right hand side        : [ " << hex_format(b) << "]\n";
	cout << "solution vector x      : [ " << hex_format(x) << "]\n";
	Vector e = A * x - b;
	Vector r = residual(A, x, b);
	cout << "Residual (non-quire)   : [ " << hex_format(e) << "]\n";
	cout << "Residual (quire)       : [ " << hex_format(r) << "]\n";
	cout << "Residual (quire) value : [ " << setw(14) << r << "]\n";
	cout << '\n';

	Vector minposRef(N);
	Scalar mp;
	minpos<32, 2>(mp);
	minposRef = mp;
	cout << "Minpos reference       : [ " << hex_format(minposRef) << "]\n\n";

	// solve for the residual
	Vector c = lubksb(LU, indx, r);
	cout << "right hand side        : [ " << hex_format(r) << "]\n";
	cout << "solution vector c      : [ " << hex_format(c) << "]\n";
	e = A * c - r;
	r = residual(A, c, r);
	cout << "Residual (non-quire)   : [ " << hex_format(e) << "]\n";
	cout << "Residual (quire)       : [ " << hex_format(r) << "]\n";
	cout << "Residual (quire) value : [ " << setw(14) << r << "]\n";
	cout << '\n';

	cout << "Result x' = x - c\n";
	cout << "Solution vector x'     : [ " << hex_format(x - c) << "]\n";
	cout << "Exact solution vector  : [ " << hex_format(ones) << "]\n";
	cout << '\n';

	cout << "1-norm x' - ones       :   " << norm1(x - c - ones) << '\n';
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::unum::blas;

	cout << "Residual calculations\n";

	streamsize precision = cout.precision();

	/*
	sw::unum::blas::vector<int> sizes = { 5, 15, 45, 95 };
	for (auto N : sizes) {
		FrankMatrixTest<float>(N);
		FrankMatrixTest<posit<32, 2>>(N);
	}
	*/
	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Scalar = posit<nbits, es>;
	using Vector = sw::unum::blas::vector<Scalar>;
	using Matrix = sw::unum::blas::matrix<Scalar>;
	constexpr size_t N = 5;
	Matrix A = sw::unum::blas::frank<Scalar>(N);

	cout << "Frank matrix\n";
	ResidualTest(A);
	cout << '\n';

	cout << "Hilbert matrix\n";
	A = sw::unum::blas::hilbert<Scalar>(N);
	ResidualTest(A);

	{
		// reference float version
		using Scalar = float;
		using Vector = sw::unum::blas::vector<Scalar>;
		using Matrix = sw::unum::blas::matrix<Scalar>;

		Vector ones(N);
		ones = Scalar(1);
		Vector b(N);
		Matrix A = sw::unum::blas::hilbert<Scalar>(N);
		b = A * ones;
		Vector x = solve(A, b);
		cout << "1-norm of float ref    :   " << norm1(x - ones) << endl;
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
