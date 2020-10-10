// hilbert.cpp: Hilbert matrix
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifdef _MSC_VER
#pragma warning(disable : 4100) // argc/argv unreferenced formal parameter
#pragma warning(disable : 4514 4571)
#pragma warning(disable : 4625 4626) // 4625: copy constructor was implicitly defined as deleted, 4626: assignment operator was implicitely defined as deleted
#pragma warning(disable : 5025 5026 5027)
#pragma warning(disable : 4710 4774)
#pragma warning(disable : 4820)
#endif
// enable the following define to show the intermediate steps in the fused-dot product
// #define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_MUL
#define QUIRE_TRACE_ADD
// configure posit environment
#define POSIT_FAST_POSIT_8_0 1
#define POSIT_FAST_POSIT_16_1 1
#define POSIT_FAST_POSIT_32_2 1
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>
#define BLAS_TRACE_ROUNDING_EVENTS 1
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>

int main(int argc, char* argv[])
try {
	using namespace std;
	using namespace sw::unum::blas;

	if (argc == 1) cout << argv[0] << endl;

	using Scalar = float;
	using Vector = sw::unum::blas::vector<Scalar>;
	using Matrix = sw::unum::blas::matrix<Scalar>;

	constexpr size_t N = 5;
	Matrix H(N, N), Hscale(N, N), Hinv(N, N);
	GenerateHilbertMatrix<float>(H, false);
	Scalar lcm = GenerateHilbertMatrix<float>(Hscale, true); // scale the Hilbert matrix entries to be binary representable
	cout << "Hilbert matrix\n" << H << endl;
	cout << "Scaled Hilbert matrix\n" << Hscale << endl;
	GenerateHilbertMatrixInverse(Hinv, lcm);
	cout << "Scaled Hilbert inverse matrix\n" << Hinv << endl;
	cout << "Validation: Hinv * H => I\n" << Hinv * H << endl;

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
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
