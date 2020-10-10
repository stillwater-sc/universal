// inverse.cpp: example program comparing float vs posit using Gauss-Jordan algorithm
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the HPRBLAS project, which is released under an MIT Open Source license.
#ifdef _MSC_VER
#pragma warning(disable : 4514)   // unreferenced inline function has been removed
#pragma warning(disable : 4710)   // 'int sprintf_s(char *const ,const size_t,const char *const ,...)': function not inlined
#pragma warning(disable : 4820)   // 'sw::unum::value<23>': '3' bytes padding added after data member 'sw::unum::value<23>::_sign'
#pragma warning(disable : 5045)   // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif

#include <chrono>
//
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// enable fast posits
#define POSIT_FAST_SPECIALIZATION
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
		double nrOps = double(N) * double(N) * double(N);
		std::cout << "Performance " << (uint32_t)(nrOps / (1000000.0 * elapsed)) << " MOPS/s" << std::endl;

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
	cout << "--------------------------------\n\n";
}

template<typename Scalar>
void TestSingularMatrix() {
	using namespace std;
	using Matrix = sw::unum::blas::matrix<Scalar>;

	cout << "Test Singular matrix\n";

	// define a singular matrix
	Matrix A = {
		{ 1, 2, 3 },
		{ 4, 5, 6 },
		{ 7, 8, 9 }
	};
	cout << A << endl;
	Matrix B = inv(A);
	// should report an error
	cout << "--------------------------------\n\n";
}

template<typename Scalar>
void TestNearSingular() {
	using namespace std;
	cout << "Test Near Singular matrix\n" << endl;

	cout << "Gauss-Jordan inverse test with near-singular matrix\n";
	cout << "Scalar type: " << typeid(Scalar).name() << '\n';

	using Matrix = sw::unum::blas::matrix<Scalar>;
	using Vector = sw::unum::blas::vector<Scalar>;

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
	cout << "eps: " << Aeps(2, 2) << endl;
	Scalar m = 1024;
	Matrix B = sw::unum::blas::inv(A + m * Aeps);
	cout << "Test matrix with poor condition number\n" << (A + m * Aeps) << endl;
	if (num_cols(B) == 0) {
		cout << "singular matrix\n";
	}
	else {
		cout << "Inverse\n" << B << endl;
		cout << "Validation to Identity matrix\n" << B * (A + m * Aeps) << endl;
	}
	cout << "--------------------------------\n\n";
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::unum::blas;

	using Scalar = float;
	using Matrix = sw::unum::blas::matrix<Scalar>;
	using Vector = sw::unum::blas::vector<Scalar>;

	if (argc == 1) cout << argv[0] << '\n';
	int nrOfFailedTestCases = 0;

	TestSingularMatrix<float>();

	TestNearSingular<float>();
	TestNearSingular<posit<8, 0> >();
	TestNearSingular<posit<16, 1> >();
	TestNearSingular<posit<32, 2> >();
	TestNearSingular<posit<64, 3> >();
	TestNearSingular<posit<128, 4> >();

	{
		// generate the inverse of a tridiag matrix, which can be solved without pivoting
		Matrix A = tridiag<Scalar>(5);
		cout << "tridiagonal matrix\n" << A << endl;
		cout << "inverse full-pivoting Gauss-Jordan\n" << inv(A) << endl;
		cout << "fast inverse no-pivoting Gauss-Jordan\n" << invfast(A) << endl;
	}


	constexpr size_t N = 100;
	FiniteDifferenceTest<float>(N);
	FiniteDifferenceTest < sw::unum::posit<32, 2> >(N);

	return (nrOfFailedTestCases == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
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
