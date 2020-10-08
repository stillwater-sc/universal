// inverse.cpp: example program comparing float vs posit using Gauss-Jordan algorithm
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the HPRBLAS project, which is released under an MIT Open Source license.

#include <chrono>
//
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/blas/blas.hpp>
#include <universal/functions/isrepresentable.hpp>

template<size_t nbits, size_t es, size_t capacity = 10>
void ComparePositDecompositions(sw::unum::blas::matrix< sw::unum::posit<nbits, es> >& A, sw::unum::blas::vector< sw::unum::posit<nbits, es> >& x, sw::unum::blas::vector< sw::unum::posit<nbits, es> >& b) {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::unum::blas;
	assert(num_rows(A) == num_cols(A));

	{
		using namespace std::chrono;
		steady_clock::time_point t1 = steady_clock::now();
		auto Ainv = inv(A);
		steady_clock::time_point t2 = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
		double elapsed = time_span.count();
		std::cout << "Gauss-Jordan took " << elapsed << " seconds." << std::endl;
		std::cout << "Performance " << (uint32_t)(N*N*N / (1000 * elapsed)) << " KOPS/s" << std::endl;

		x = Ainv * b;
		cout << "Inverse\n" << Ainv << endl;
		cout << "Solution\n" << x << endl;
		cout << "RHS\n" << b << endl;
	}

	std::cout << std::endl;
}

template<size_t nbits, size_t es>
void GaussianEliminationTest() {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::unum::blas;
	using Scalar = sw::unum::posit<nbits, es>;
	using Vector = sw::unum::blas::vector<Scalar>;
	using Matrix = sw::unum::blas::matrix<Scalar>;
	cout << "Using " << dynamic_range<nbits, es>() << endl;

	// repeat set up for posits
	cout << "Posit inputs\n";
	Matrix U = {     // define the upper triangular matrix
		{ 1.0, 2.0, 3.0, 4.0, 5.0 },
		{ 0.0, 1.0, 2.0, 3.0, 4.0 },
		{ 0.0, 0.0, 1.0, 2.0, 3.0 },
		{ 0.0, 0.0, 0.0, 1.0, 2.0 },
		{ 0.0, 0.0, 0.0, 0.0, 1.0 },
	};
	Matrix L = {     // define the lower triangular matrix
		{ 1.0, 0.0, 0.0, 0.0, 0.0 },
		{ 2.0, 1.0, 0.0, 0.0, 0.0 },
		{ 3.0, 2.0, 1.0, 0.0, 0.0 },
		{ 4.0, 3.0, 2.0, 1.0, 0.0 },
		{ 5.0, 4.0, 3.0, 2.0, 1.0 },
	};
	auto A = L * U;   // construct the A matrix to solve
	cout << "L\n" << L << endl;
	cout << "U\n" << U << endl;
	cout << "A\n" << A << endl;
	// define a difficult solution
	Scalar epsplus = Scalar(1) + numeric_limits<Scalar>::epsilon();
	Vector x = {
		epsplus,
		epsplus,
		epsplus,
		epsplus,
		epsplus
	};
	auto b = fmv(A, x);   // construct the right hand side
	cout << "b" << b << endl;
	cout << endl << ">>>>>>>>>>>>>>>>" << endl;
	cout << "LinearSolve fused-dot product" << endl;
	ComparePositDecompositions(A, x, b);
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::unum::blas;

	using Scalar = float;
	using Matrix = sw::unum::blas::matrix<Scalar>;
	using Vector = sw::unum::blas::vector<Scalar>;

	Matrix A = {
		{  2, -1,  0,  0,  0 },
		{ -1,  2, -1,  0,  0 },
		{  0, -1,  2, -1,  0 },
		{  0,  0, -1,  2, -1 },
		{  0,  0,  0, -1,  2 }
	};

	auto Ainv = inv(A);
	cout << Ainv << endl;
	cout << Ainv * A << endl;

	// A = L + D + U decomposition
	auto D = diag(diag(A));
	auto L = tril(A) - D;
	auto U = triu(A) - D;

	auto I = eye<Scalar>(num_cols(A));
	L += I;
	auto Linv = inv(L);
	cout << Linv << endl;
	cout << Linv * L << endl << L * Linv << endl;
	return EXIT_SUCCESS;
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
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
