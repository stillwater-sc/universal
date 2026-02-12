// trigonometry.cpp: test suite for vectorized trigonometry math functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
// Configure the posit library with arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
// enable posit arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/cfloat/cfloat.hpp>
#include <blas/blas.hpp>

constexpr double PI = 3.14159265358979323846;  // best practice for C++

template<typename Scalar>
void TestTriangleVmath(unsigned N = 12) {
	using namespace sw::numeric::containers;
	using namespace sw::blas;
	using namespace sw::universal;

	using Vector = vector<Scalar>;
	Vector v = linspace<Scalar>(0, 2*PI, N);
	std::cout << "\narithmetic type : " << sw::universal::type_tag(Scalar()) << '\n';
	std::cout << "radians  = " << v << '\n';;
	auto cosines = cos(v);
	std::cout << "cosines  = " << cosines << '\n';;
	auto sines = sin(v);
	std::cout << "sines    = " << sines << '\n';;
	auto tangents = tan(v);
	std::cout << "tangents = " << tangents << '\n';;
}

int main()
try {
	using namespace sw::universal;
	using namespace sw::blas;

	int nrOfFailedTestCases = 0;

	TestTriangleVmath<posit<32,2>>();
	TestTriangleVmath<fp16>();
	TestTriangleVmath<float>();

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
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
