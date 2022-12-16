// residual.cpp: example program to show exact residual calucation using the quire
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <limits>
#include <numeric>   // nextafter

// select the number systems we would like to compare
#include <universal/number/integer/integer.hpp>
//#include <universal/number/fixpnt/fixpnt.hpp>// TODO: this causes this warning for an unknown reason:
// include\universal/posit/posit.hpp(851,1): warning C4305: 'specialization': truncation from 'const size_t' to 'bool'

#include <universal/number/areal/areal.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

#include <universal/blas/blas.hpp>
#include <universal/blas/generators/frank.hpp>
#include <universal/blas/generators/hilbert.hpp>

namespace sw {
	namespace universal {
/*
template<typename Scalar>
blas::vector<Scalar> residual(const blas::matrix<Scalar>& A, const blas::vector<Scalar>& x, const blas::vector<Scalar>& b) {
	using namespace sw::universal;
	using namespace sw::universal::blas;
	using Vector = sw::universal::blas::vector<Scalar>;
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

template<unsigned nbits, unsigned es, unsigned capacity = 10>
blas::vector<posit<nbits, es>> residual(const blas::matrix<posit<nbits, es>>& A, const blas::vector<posit<nbits, es>>& x, const blas::vector<posit<nbits, es>>& b) {
	using namespace sw::universal::blas;
	using Scalar = posit<nbits, es>;
	using Vector = blas::vector<Scalar>;
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
	using Vector = blas::vector<Scalar>;
	using Matrix = blas::matrix<Scalar>;
	Matrix A = sw::universal::blas::frank<Scalar>(N);
	std::cout << "Frank matrix order " << N << '\n';
	Vector b(N), x(N);
	x = Scalar(1);
	b = A * x;
//	cout << "right hand side [" << b << "]\n";
	x = solve(A, b);
//	cout << "solution vector [" << x << "]\n";
	Vector e = A * x - b;
	std::cout << "L1-norm of error vector: " << norm(e, 1) << '\n';
}

void Experiment1() {
	blas::vector<int> sizes = { 5, 15, 45, 95 };
	for (auto N : sizes) {
		FrankMatrixTest<float>(N);
		FrankMatrixTest<posit<32, 2>>(N);
	}
}

template<typename Matrix>
void ResidualTest(const Matrix& A) {
	/*
	using namespace sw::universal;
	using namespace sw::universal::blas;

	using Scalar = posit<nbits, es>;
	using Vector = sw::universal::blas::vector<Scalar>;
	using Matrix = matrix<Scalar>;

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
	sw::universal::blas::vector<size_t> indx(N);
	Matrix LU(A); // the LU decomposition is in place, so create a copy first
	auto error = ludcmp(LU, indx);
	if (error != 0) {
		cerr << "LU decomposition failed\n";
	}
	cout << "LU decomposition\n";
	cout << hex_format(LU) << endl;
	x = lubksb(LU, indx, b);
	cout << "right hand side        : [ " << hex_format(b) << "]\n";
	cout << "right hand side        : [ " << (b) << "]\n";
	cout << "solution vector x      : [ " << hex_format(x) << "]\n";
	cout << "solution vector x      : [ " << (x) << "]\n";
	Vector e = A * x - b;
	Vector r = residual(A, x, b);
	cout << "Residual (non-quire)   : [ " << hex_format(e) << "]\n";
	cout << "Residual (non-quire)   : [ " << (e) << "]\n";
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
	cout << "right hand side        : [ " << (r) << "]\n";
	cout << "solution vector c      : [ " << hex_format(c) << "]\n";
	e = A * c - r;
	r = residual(A, c, r);
	cout << "Residual (non-quire)   : [ " << hex_format(e) << "]\n";
	cout << "Residual (non-quire)   : [ " << (e) << "]\n";
	cout << "Residual (quire)       : [ " << hex_format(r) << "]\n";
	cout << "Residual (quire) value : [ " << setw(14) << r << "]\n";
	cout << '\n';

	cout << "Result x' = x - c\n";
	cout << "Solution vector x'     : [ " << hex_format(x - c) << "]\n";
	cout << "Solution vector x'     : [ " << (x - c) << "]\n";
	cout << "Exact solution vector  : [ " << hex_format(ones) << "]\n";
	cout << '\n';

	cout << "1-norm x' - ones       :   " << norm1(x - c - ones) << '\n';
	*/
}

void Experiment2() {
	constexpr unsigned nbits = 32;
	constexpr unsigned es = 2;
	using Scalar = posit<nbits, es>;
	using Matrix = blas::matrix<Scalar>;
	constexpr unsigned N = 5;
	Matrix A = blas::frank<Scalar>(N);

	std::cout << "Frank matrix\n";
	ResidualTest(A);
	std::cout << '\n';

	std::cout << "Hilbert matrix\n";
	A = sw::universal::blas::hilbert<Scalar>(N);
	ResidualTest(A);

	{
		// reference float version
		using Scalar = float;
		using Vector = blas::vector<Scalar>;
		using Matrix = blas::matrix<Scalar>;

		Vector ones(N);
		ones = Scalar(1);
		Vector b(N);
		Matrix A = sw::universal::blas::hilbert<Scalar>(N);
		b = A * ones;
		Vector x = solve(A, b);
		std::cout << "1-norm of float ref    :   " << norm(x - ones, 1) << '\n';

	}

	{
		// reference double version
		using Scalar = double;
		using Vector = blas::vector<Scalar>;
		using Matrix = blas::matrix<Scalar>;

		Vector ones(N);
		ones = Scalar(1);
		Vector b(N);
		Matrix A = blas::hilbert<Scalar>(N);
		b = A * ones;
		Vector x = solve(A, b);
		std::cout << "1-norm of double ref   :   " << norm(x - ones, 1) << '\n';

	}

}


template<unsigned nbits, unsigned es>
void QuireCompensation(const blas::matrix<posit<nbits, es>>& A, const posit<nbits, es>& tolerance = 1.0e-15, unsigned MAX_ITERATIONS = 100) {
	using Scalar = posit<nbits, es>;
	using Vector = blas::vector<Scalar>;
	using Matrix = blas::matrix<Scalar>;

	const size_t M = num_rows(A);
	const size_t N = num_cols(A);
	if (M != N) {
		std::cerr << "matrix is not square: (" << M << " by " << N << ")\n";
		return;
	}

	// visual feedback control
	constexpr unsigned MAX_COLUMNS = 8;

	Matrix LU(A);
	sw::universal::blas::vector<size_t> indx(N);
	if (ludcmp(LU, indx)) return; // LU decomposition failed, simply bail

	Vector b(M), x(M), r(M), c(M);
	x = Scalar(1);
	b = A * x;  // FDP-enabled matvec multiply

	// Residual compensation iteration
	unsigned iterations = 0;
	x = lubksb(LU, indx, b);
	r = residual(A, x, b);
	Scalar error = norm(r, 1);
	constexpr unsigned columnWidth = 14;
	if (M < MAX_COLUMNS) std::cout << "solution vector: " << std::setw(columnWidth) << x << "\n";
	std::cout << "error: " << error << "\n";
	Scalar eps = std::numeric_limits<Scalar>::epsilon();
//	std::cout << "epsilon for " << typeid(Scalar).name() << " = " << eps << '\n';
	while (error > tolerance && iterations < MAX_ITERATIONS) {
		c = lubksb(LU, indx, r);
		if (M < MAX_COLUMNS) std::cout << "compensation vector: " << std::setw(columnWidth) << c << "\n";
		x = x - c; // compensated solution vector
		if (M < MAX_COLUMNS) std::cout << "solution     vector: " << std::setw(columnWidth) << x << "\n";
		if (M < MAX_COLUMNS) std::cout << "solution     vector: " << hex_format(x) << "\n";
		r = residual(A, c, r);
		error = norm(r, 1);
		std::cout << "error: " << error << "\n";
		++iterations;
		if (error < eps) break;
	}
	if (error < eps) {
		std::cout << "Reduced error to machine precision: error = " << error << " epsilon = " << eps << '\n';
	}
	if (iterations >= MAX_ITERATIONS) {
		std::cout << "Reached max iteration limit\n";
	}
	if (error < tolerance) {
		std::cout << "Reduced error to below requested tolerance of " << tolerance << '\n';
	}
}

template<typename Scalar>
void IeeeReference(unsigned MATRIX_ROWS) {
	std::cout << "\n\ncalculate " << typeid(Scalar).name() << " reference\n";
	using Vector = blas::vector<Scalar>;
	using Matrix = blas::matrix<Scalar>;
	Matrix A = blas::hilbert<Scalar>(MATRIX_ROWS);
	const unsigned MATRIX_COLS = MATRIX_ROWS; // we are a square matrix
	Vector ones(MATRIX_COLS);
	ones = 1.0;
	Vector b = A * ones;
	Vector x = solve(A, b);
	Vector r = A * x - b;
	Scalar error = norm(r, 1);
	std::cout << "error : " << error << "\n";
}

}} // namespace sw::universal

int main(int argc, char** argv)
try {
	using namespace sw::universal;
	using namespace sw::universal::blas;

	std::cout << "Kulisch iterator\n";

	std::streamsize precision = std::cout.precision();

	constexpr unsigned nbits = 32;
	constexpr unsigned es = 2;
	using Scalar = posit<nbits, es>;
	using Matrix = blas::matrix<Scalar>;

	std::cout << "epsilon for " << typeid(Scalar).name() << " = " << std::numeric_limits<Scalar>::epsilon() << '\n';
	std::cout << "Hilbert matrix\n";
	constexpr unsigned MATRIX_ROWS = 10;
	Matrix A = blas::hilbert<Scalar>(MATRIX_ROWS); // default is a scaled Hilbert matrix with exact representation
	QuireCompensation(A);

	IeeeReference<float>(MATRIX_ROWS);
	IeeeReference<double>(MATRIX_ROWS);
	IeeeReference<long double>(MATRIX_ROWS);

	std::cout << std::setprecision(precision);
	std::cout << std::endl;
	
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
