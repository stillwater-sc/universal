// gemm.cpp: energy measurement of mixed-precision general matrix-matrix product
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the following define to show the intermediate steps in the fused-dot product
// #define NUMBER_VERBOSE_OUTPUT
#define NUMBER_TRACE_MUL
#define QUIRE_TRACE_ADD
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
// enable operation counts
#define DECIMAL_OPERATIONS_COUNT 1
#include <universal/number/decimal/decimal.hpp>
#define BLAS_TRACE_ROUNDING_EVENTS 1
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>

template<typename Scalar>
std::string conditional_fdp(const sw::universal::blas::vector< Scalar >& a, const sw::universal::blas::vector< Scalar >& b) {
	return std::string("no FDP for non-posit value_type");
}
template<size_t nbits, size_t es>
std::string conditional_fdp(const sw::universal::blas::vector< sw::universal::posit<nbits, es> >& a, const sw::universal::blas::vector< sw::universal::posit<nbits, es> >& b) {
	std::stringstream ss;
	ss << sw::universal::fdp(a, b);
	return ss.str();
}

#if DECIMAL_OPERATIONS_COUNT

// create the static storage for the occurrence measurements of the decimal number system
bool sw::universal::decimal::enableAdd = true;
sw::universal::occurrence<sw::universal::decimal> sw::universal::decimal::ops;

#endif

int main(int argc, char** argv)
try {
	using namespace sw::universal::blas;

	using Scalar = sw::universal::decimal;
	using Matrix = matrix<Scalar>;

	constexpr size_t N = 5;

	Matrix A = eye<Scalar>(N);
	Matrix B = frank<Scalar>(N);
	sw::universal::decimal proxy;
	proxy.resetStats();
	Matrix C = A * B;
	std::cout << C << '\n';
	proxy.printStats(std::cout);

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
