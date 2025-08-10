// inverse.cpp: example program comparing float vs posit using Gauss-Jordan algorithm
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the HPRBLAS project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <chrono>
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// enable fast posits
#define POSIT_FAST_SPECIALIZATION
#include <universal/number/posit/posit.hpp>
#include <blas/blas.hpp>
#include <blas/generators.hpp>

template<typename Matrix, typename Vector>
void BenchmarkGaussJordan(const Matrix& A, Vector& x, const Vector& b) {
	using namespace sw::universal;
	using namespace sw::blas;
	assert(num_rows(A) == num_cols(A));
	size_t N = num_cols(A);
	{
		using namespace std::chrono;
		steady_clock::time_point t1 = steady_clock::now();
		auto Ainv = inv(A);
		steady_clock::time_point t2 = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
		double elapsed = time_span.count();
		std::cout << "Gauss-Jordan took " << elapsed << " seconds.\n";
		double nrOps = double(N) * double(N) * double(N);
		std::cout << "Performance " << (uint32_t)(nrOps / (1000000.0 * elapsed)) << " MOPS/s\n";

		x = Ainv * b;
		if (N < 10) {
			std::cout << "Inverse\n" << Ainv << '\n';
			std::cout << "Solution\n" << x << '\n';
			std::cout << "RHS\n" << b << '\n';
		}
	}

	std::cout << std::endl;
}

void Test1() {
	using namespace sw::universal;
	using namespace sw::numeric::containers;
	using namespace sw::blas;

	using Scalar = float;
	using Matrix = matrix<Scalar>;

	Matrix A = {
		{  2, -1,  0,  0,  0 },
		{ -1,  2, -1,  0,  0 },
		{  0, -1,  2, -1,  0 },
		{  0,  0, -1,  2, -1 },
		{  0,  0,  0, -1,  2 }
	};

	auto Ainv = inv(A);
	std::cout << Ainv << '\n';
	std::cout << Ainv * A << '\n';

	// A = L + D + U decomposition
	auto D = diag(diag(A));
	auto L = tril(A) - D;
	auto U = triu(A) - D;

	auto I = eye<Matrix>(num_cols(A));
	L += I;
	auto Linv = inv(L);
	std::cout << Linv << '\n';
	std::cout << Linv * L << '\n' << L * Linv << '\n';
}

template<typename Scalar>
void FiniteDifferenceTest(size_t N) {
	using namespace sw::universal;
	using namespace sw::blas;

	using Matrix = matrix<Scalar>;
	using Vector = vector<Scalar>;

	Matrix A;
	tridiag(A, N, Scalar(-1), Scalar(2), Scalar(-1));

	Vector x(N);
	x = Scalar(1);
	auto b = A * x;

	BenchmarkGaussJordan(A, x, b);

	if (N < 10) {
		std::cout << "Finite Difference Matrix\n" << A << '\n';

		// visual feedback
		auto Ainv = inv(A);
		std::cout << Ainv << '\n';
		std::cout << Ainv * A << '\n';
		auto L = tril(A);
		std::cout << inv(L) << '\n';
	}
	std::cout << "--------------------------------\n\n";
}

template<typename Scalar>
int TestSingularMatrix() {
	using Matrix = sw::numeric::containers::matrix<Scalar>;

	std::cout << "Test Singular matrix\n";

	// define a singular matrix
	Matrix A = {
		{ 1, 2, 3 },
		{ 4, 5, 6 },
		{ 7, 8, 9 }
	};
	std::cout << A << '\n';
	Matrix B = sw::blas::inv(A);
	// should report an error and return a null matrix
	int nrOfFailedTests{ 0 };
	if (B.cols() != 0 && B.rows() != 0) ++nrOfFailedTests;
	std::cout << "inv(A) will return a null matrix when singular\n";
	std::cout << "B.rows() : " << B.rows() << "\nB.cols() : " << B.cols() << '\n';
	std::cout << "--------------------------------\n\n";

	return nrOfFailedTests;
}

template<typename Scalar>
void TestNearSingular() {
	std::cout << "Test Near Singular matrix\n\n";

	std::cout << "Gauss-Jordan inverse test with near-singular matrix\n";
	std::cout << "Scalar type: " << typeid(Scalar).name() << '\n';

	using Matrix = sw::numeric::containers::matrix<Scalar>;

	// define a singular matrix
	Matrix A = {
		{ 1, 2, 3 },
		{ 4, 5, 6 },
		{ 7, 8, 9 }
	};
	// define an eps entry
	Matrix Aeps = {
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, std::numeric_limits<Scalar>::epsilon() }
	};
	std::cout << "eps: " << Aeps(2, 2) << '\n';
	Scalar m = 1024;
	Matrix B = sw::blas::inv(A + m * Aeps);
	std::cout << "Test matrix with poor condition number\n" << (A + m * Aeps) << '\n';
	if (num_cols(B) == 0) {
		std::cout << "singular matrix\n";
	}
	else {
		std::cout << "Inverse\n" << B << '\n';
		std::cout << "Validation to Identity matrix\n" << B * (A + m * Aeps) << '\n';
	}
	std::cout << "--------------------------------\n\n";
}

int main()
try {
	using namespace sw::universal;
	using namespace sw::numeric::containers;
	using namespace sw::blas;

	using Scalar = float;
	using Matrix = matrix<Scalar>;

	int nrOfFailedTestCases = 0;

	nrOfFailedTestCases += TestSingularMatrix<float>();

	TestNearSingular<float>();
	TestNearSingular<posit<8, 0> >();
	TestNearSingular<posit<16, 1> >();
	TestNearSingular<posit<32, 2> >();
	TestNearSingular<posit<64, 3> >();
	TestNearSingular<posit<128, 4> >();

	{
		// generate the inverse of a tridiag matrix, which can be solved without pivoting
		Matrix A = tridiag<Scalar>(5);
		std::cout << "tridiagonal matrix\n" << A << '\n';
		std::cout << "inverse full-pivoting Gauss-Jordan\n" << inv(A) << '\n';
		std::cout << "fast inverse no-pivoting Gauss-Jordan\n" << invfast(A) << '\n';
	}


	constexpr size_t N = 100;
	FiniteDifferenceTest<float>(N);
	FiniteDifferenceTest < sw::universal::posit<32, 2> >(N);

	return (nrOfFailedTestCases == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
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
