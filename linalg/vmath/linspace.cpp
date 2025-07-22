// linspace.cpp: test suite for linspace/logspace/geomspace sequence generators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>

// Universal numbers of interest
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

// Stillwater BLAS library
#include <blas/blas.hpp>

//constexpr double pi = 3.14159265358979323846;  // best practice for C++

template<typename Scalar>
void TestRangeGeneration() {
	using namespace sw::universal::blas;
	using std::pow;
	using Vector = sw::universal::blas::vector<Scalar>;
	Vector v = linspace<Scalar>(0, 10, 5);
	std::cout << "linspace = " << v << '\n';
	v = linspace<Scalar>(0, 10, 5, false);
	std::cout << "linspace = " << v << '\n';

	v = logspace<Scalar>(0, 10, 5);
	std::cout << "logspace = " << v << '\n';
	v = logspace<Scalar>(0, 10, 5, false);
	std::cout << "logspace = " << v << '\n';

	Scalar x{ 10 }, y{ 1.5 };
	std::cout << "x^y = " << pow(x, y) << '\n';

	v = geomspace<Scalar>(0, 10, 5);
	std::cout << "geomspace = " << v << '\n';
	v = geomspace<Scalar>(0, 10, 5, false);
	std::cout << "geomspace = " << v << '\n';
}

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	TestRangeGeneration<float>();
	TestRangeGeneration<single>();
	TestRangeGeneration<posit<32,2>>();
	TestRangeGeneration<lns<16,8>>();

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
