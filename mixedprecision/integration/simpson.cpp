// simpson.cpp: mixed-precision experiments with simpson rule integration
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit/posit.hpp>
#include <blas/blas.hpp>

// f(x): function to integrate
template<typename Scalar>
Scalar f(const Scalar& x) {
	return x * sin(x);
}

/// <summary>
/// Simpson 1/3 rule
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns>approximate value under the curve</returns>
template<typename Scalar>
Scalar SimpsonOneOverThreeRule(const Scalar& a, const Scalar& b, size_t n, Scalar (*f)(const Scalar&)) {
	using namespace sw::universal;
	using namespace sw::universal::blas;

	using Vector = sw::universal::blas::vector<Scalar>;

	Scalar h = (b - a) / Scalar(n);
	Vector x(n + 1);

	for (size_t j = 0; j <= n; ++j) {
		x[j] = a + h*j;
	}

	Scalar area;
	Scalar half = n / 2ull;
	for (size_t j = 1; j <= half; ++j) {
		area += (*f)(x[2 * j - 2]) + Scalar(4.0) * (*f)(x[2 * j - 1]) + (*f)(x[2 * j]);
	}
	return area * h / Scalar(3.0f);   // weakness: 3.0 can't be represented in binary representations
}

// TODO: use quire for sum
// TODO: Simpson's 3/8 rule

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	using Scalar = posit<16, 2>;
	Scalar a = 1.0;
	Scalar b = 3.0;
	// area is roughly: 2.80992881892
	for (size_t n = 10; n < 41; n += 2) {   // precondition: n must be even for Simpson 1/3 rule
		Scalar area = SimpsonOneOverThreeRule(a, b, n, &f);
		std::cout << "integral of f(x) between " << a << " and " << b << " = " << area << '\n';
	}

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
