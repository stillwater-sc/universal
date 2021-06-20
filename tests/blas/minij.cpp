// minij.cpp: Minimum IJ matrix
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifdef _MSC_VER
#pragma warning(disable : 4100) // argc/argv unreferenced formal parameter
#pragma warning(disable : 4514 4571)
#pragma warning(disable : 4625 4626) // 4625: copy constructor was implicitly defined as deleted, 4626: assignment operator was implicitely defined as deleted
#pragma warning(disable : 5025 5026 5027 5045)
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
#include <universal/number/posit/posit.hpp>
#define BLAS_TRACE_ROUNDING_EVENTS 1
#include <universal/blas/blas.hpp>
#include <universal/blas/generators/minij.hpp>

template<typename Scalar>
void MinIJMatrixTest(size_t N = 5) {
	using namespace std;
	using namespace sw::universal::blas;

	cout << "MinIJ MatrixTest for type: " << typeid(Scalar).name() << endl;
	auto M = minij<Scalar>(N);

	// normalize the column vectors
	auto total = sum(M);
	cout << "Total    : " << total << endl;
	auto rowSums = sum(M, 1);
	cout << "Row sums : " << rowSums << endl;
	auto colSums = sum(M, 2);
	cout << "Col sums : " << colSums << endl;
}

int main(int argc, char* argv[])
try {
	using namespace std;
	using namespace sw::universal;

	if (argc == 1) cout << argv[0] << endl;

	MinIJMatrixTest< float >();
	MinIJMatrixTest< posit<32, 2> >();

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
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
