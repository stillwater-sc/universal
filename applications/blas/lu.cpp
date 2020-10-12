// lu.cpp: example program comparing float vs posit Gaussian Elimination (LU Decomposition) equation solver
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
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>
#include <universal/functions/isrepresentable.hpp>

template<size_t nbits, size_t es, size_t capacity = 10>
void BenchmarkLUDecomposition(sw::unum::blas::matrix< sw::unum::posit<nbits, es> >& A, sw::unum::blas::vector< sw::unum::posit<nbits, es> >& x, sw::unum::blas::vector< sw::unum::posit<nbits, es> >& b) {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::unum::blas;
	assert(num_rows(A) == num_cols(A));

	size_t N = num_cols(A);
	matrix< posit<nbits, es> > LU(N,N);

	{
		using namespace std::chrono;
		steady_clock::time_point t1 = steady_clock::now();
		Crout(A, LU);
		steady_clock::time_point t2 = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
		double elapsed = time_span.count();
		std::cout << "Crout took " << elapsed << " seconds." << std::endl;
		std::cout << "Performance " << (uint32_t)(N*N*N / (1000 * elapsed)) << " KOPS/s" << std::endl;

		SolveCrout(LU, b, x);
		cout << "Crout LU\n" << LU << endl;
		cout << "Solution\n" << x << endl;
		cout << "RHS\n" << b << endl;
	}

	{
		using namespace std::chrono;
		steady_clock::time_point t1 = steady_clock::now();
		CroutFDP(A, LU);
		steady_clock::time_point t2 = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
		double elapsed = time_span.count();
		std::cout << "CroutFDP took " << elapsed << " seconds." << std::endl;
		std::cout << "Performance " << (uint32_t)(N * N * N / (1000 * elapsed)) << " KOPS/s" << std::endl;

		SolveCroutFDP(LU, b, x);
		cout << "CroutFDP LU\n" << LU << endl;
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
		{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0 },
		{ 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 },
		{ 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 },
		{ 0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 },
		{ 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0 },
		{ 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0 },
		{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 3.0 },
		{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 2.0 },
		{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 },
	};
	Matrix L = transpose(U);

	auto A = L * U;   // construct the A matrix to solve
	cout << "L\n" << L << endl;
	cout << "U\n" << U << endl;
	cout << "A\n" << A << endl;
	size_t N = num_cols(A);
	// define a difficult solution
	Scalar epsplus = Scalar(1) + numeric_limits<Scalar>::epsilon();
	Vector x(N);
	x = epsplus;
	auto b = fmv(A, x);   // construct the right hand side
	cout << "b" << b << endl;
	cout << endl << ">>>>>>>>>>>>>>>>" << endl;

	BenchmarkLUDecomposition(A, x, b);
}

template<typename Scalar>
void FrankMatrixTest() {
	using namespace std;
	using Vector = sw::unum::blas::vector<Scalar>;
	using Matrix = sw::unum::blas::matrix<Scalar>;

	Matrix A = {
	{ 5, 4, 3, 2, 1 },
	{ 4, 4, 3, 2, 1 },
	{ 0, 3, 3, 2, 1 },
	{ 0, 0, 2, 2, 1 },
	{ 0, 0, 0, 1, 1 }
	};

	auto LU = lu(A);
	cout << "\n---------------- result ------------------\n";
	cout << "Combined matrix\n" << LU << endl;
	auto D = diag(diag(LU));
	auto L = tril(LU) - D + sw::unum::blas::eye<Scalar>(num_cols(A));
	auto U = triu(LU);
	cout << "Lower Triangular matrix\n" << L << endl;
	cout << "Upper Triangular matrix\n" << U << endl;
}

template<typename Scalar>
void MagicSquareTest(size_t N) {
	using namespace std;
	using namespace sw::unum::blas;
	using Vector = sw::unum::blas::vector<Scalar>;
	using Matrix = sw::unum::blas::matrix<Scalar>;

	Matrix A = magic<Scalar>(N);
	Scalar magicSum = sum(diag(A));
	Vector b(N); 
	b = magicSum;

	A = tridiag<float>(5);
	Vector x(N);
	x = 1;
	b = A * x;
	cout << "A\n" << A << endl;
	cout << "b\n" << b << endl;
//	auto x = A\b;
	x = solve(A, b);
	cout << "x\n" << x << endl;


//	Matrix L, U, P;
//	auto error = lu(A, L, U, P);
//	auto y = L\(P*b);
//	auto x = U\y;

}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;
	using namespace sw::unum::blas;

	// We want to solve the system Ax=b
	// GaussianEliminationTest<32, 2>();

	using Scalar = float;
	using Vector = sw::unum::blas::vector<Scalar>;
	using Matrix = sw::unum::blas::matrix<Scalar>;

	if (argc == 1) cout << argv[0] << '\n';
	int nrOfFailedTestCases = 0;

	//FrankMatrixTest<float>();
	MagicSquareTest<Scalar>(5);

	//

#if 0
	constexpr float eps = std::numeric_limits<float>::epsilon();
	constexpr float epsminus = 1.0f - eps;
	constexpr float epsplus = 1.0f + eps;

	cout << setprecision(std::numeric_limits<float>::max_digits10);
	cout << "posit<25,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<25, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<25, 2>(epsplus) << endl;
	cout << "posit<26,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<26, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<26, 2>(epsplus) << endl;
	cout << "posit<27,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<27, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<27, 2>(epsplus) << endl;
	cout << "posit<28,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<28, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<28, 2>(epsplus) << endl;
	cout << "posit<29,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<29, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<29, 2>(epsplus) << endl;
	cout << "posit<30,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<30, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<30, 2>(epsplus) << endl;
	cout << "posit<31,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<31, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<31, 2>(epsplus) << endl;
	cout << "posit<32,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<32, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<32, 2>(epsplus) << endl;

	cout << endl << endl;
	// is there a representational difference between 1+FLT_EPSILON in float versus posit<32,2>?
	double depsplus = epsplus;
	double pepsplus = double(posit<32, 2>(epsplus));
	cout << "1.0 + FLT_EPSILON = " << depsplus << " converts to posit value " << epsplus << endl;
	cout << "1.0 + FLT_EPSILON = " << depsplus << " converts to posit value " << pepsplus << endl;

	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << hex_format(posit<32, 2>(epsplus)) << endl;
	cout << "1.0 + FLT_EPSILON = " << depsplus << " converts to posit value " << hex_format(posit<32, 2>(pepsplus)) << endl;

#endif

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
