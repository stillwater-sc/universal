// gemm.cpp: dynamic range measurement of mixed-precision general matrix-matrix product
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/posit/posit.hpp>
#include <universal/number/posit/fdp.hpp>
// enable operation counts
#define EDECIMAL_OPERATIONS_COUNT 1
#include <universal/number/edecimal/edecimal.hpp>

#include <blas/blas.hpp>

template<typename Scalar>
std::string conditional_fdp(const sw::numeric::containers::vector< Scalar >& a, const sw::numeric::containers::vector< Scalar >& b) {
	return std::string("no FDP for non-posit value_type");
}
template<size_t nbits, size_t es>
std::string conditional_fdp(const sw::numeric::containers::vector< sw::universal::posit<nbits, es> >& a, const sw::numeric::containers::vector< sw::universal::posit<nbits, es> >& b) {
	std::stringstream ss;
	ss << sw::universal::fdp(a, b);
	return ss.str();
}

#if EDECIMAL_OPERATIONS_COUNT

// create the static storage for the occurrence measurements of the decimal number system
bool sw::universal::edecimal::enableAdd = true;
sw::universal::occurrence<sw::universal::edecimal> sw::universal::edecimal::ops;

#endif

int main()
try {
	using namespace sw::universal;
	using namespace sw::numeric::containers;
	using namespace sw::blas;

	using Scalar = edecimal;
	using Matrix = matrix<Scalar>;

	constexpr size_t N = 5;

	Matrix A = eye<Matrix>(N);
	Matrix B = frank<Scalar>(N);
	edecimal proxy;
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
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught universal internal exception: " << err.what() << std::endl;
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
