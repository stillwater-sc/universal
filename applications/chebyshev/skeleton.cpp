// nodes.cpp: Does a posit configuration exist to produce chebyshev nodes
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
// Configure the posit library with arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>
#include <universal/fixpnt/fixpnt>
#include <universal/blas/blas>

// skeleton environment to experiment with Chebyshev polynomials and approximations
int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal::blas;

	int nrOfFailedTestCases = 0;

	cout << "Chebyshev polynomial test skeleton" << endl;

//	using Scalar = sw::universal::fixpnt<32,16, Modulo, uint32_t>;
	using Scalar = sw::universal::posit<32, 2>;
	Scalar PI{ 3.14159265358979323846 };  // best practice for C++
	constexpr int N = 12;
	auto k = arange<Scalar>(0, N);
	cout << "k       = " << k << endl;
	auto cosines = -cos(k * PI / N);
	cout << "cosines = " << cosines << endl;

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
