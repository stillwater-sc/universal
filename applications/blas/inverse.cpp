// inverse.cpp: example program comparing float vs posit using Gauss-Jordan algorithm
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the HPRBLAS project, which is released under an MIT Open Source license.

#include <chrono>
//
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// enable fast posit<16,1> and posit<32,2>
#define POSIT_FAST_POSIT_16_1 1
#define POSIT_FAST_POSIT_32_2 1
#include <universal/posit/posit>
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>
#include <universal/functions/isrepresentable.hpp>

template<typename Matrix, typename Vector>
void BenchmarkGaussJordan(const Matrix& A, Vector& x, const Vector& b) {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::unum::blas;
	assert(num_rows(A) == num_cols(A));
	size_t N = num_cols(A);
	{
		using namespace std::chrono;
		steady_clock::time_point t1 = steady_clock::now();
		auto Ainv = inv(A);
		steady_clock::time_point t2 = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
		double elapsed = time_span.count();
		std::cout << "Gauss-Jordan took " << elapsed << " seconds." << std::endl;
		std::cout << "Performance " << (uint32_t)(N*N*N / (1000000.0 * elapsed)) << " MOPS/s" << std::endl;

		x = Ainv * b;
		if (N < 10) {
			cout << "Inverse\n" << Ainv << endl;
			cout << "Solution\n" << x << endl;
			cout << "RHS\n" << b << endl;
		}
	}

	std::cout << std::endl;
}

void Test1() {
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
}

template<typename Scalar>
void FiniteDifferenceTest(size_t N) {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::unum::blas;

	using Matrix = sw::unum::blas::matrix<Scalar>;
	using Vector = sw::unum::blas::vector<Scalar>;

	Matrix A;
	tridiag(A, N, Scalar(-1), Scalar(2), Scalar(-1));

	Vector x(N);
	x = Scalar(1);
	auto b = A * x;

	BenchmarkGaussJordan(A, x, b);

	if (N < 10) {
		cout << "Finite Difference Matrix\n" << A << endl;

		// visual feedback
		auto Ainv = inv(A);
		cout << Ainv << endl;
		cout << Ainv * A << endl;
		auto L = tril(A);
		cout << inv(L) << endl;
	}
}
int main(int argc, char** argv)
try {
	using namespace std;

	FiniteDifferenceTest<float>(5);
	FiniteDifferenceTest<sw::unum::posit<32, 2>>(5);

	constexpr size_t N = 100;
	FiniteDifferenceTest<float>(N);
	FiniteDifferenceTest < sw::unum::posit<32, 2> >(N);

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
