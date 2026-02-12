//  error_vs_cost.cpp : finding the Pareto front for the trade-off between cost and precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/posit1/posit1.hpp>
// Stillwater BLAS library
#include <blas/blas.hpp>
#include <blas/solvers.hpp>

/*
* Multi-objective optimization to select mixed-precision number systems
* 
* cost  = f(nbits, encoding)
* error = g(nbits, algorithm)
* 
* error = abs(Oracle - value)
 */

/*
template<bool Cond, class T = void> struct enable_if {};
template<class T> struct enable_if<true, T> { typedef T type; };

template <bool _Test, class _Ty = void>
using enable_if_t = typename enable_if<_Test, _Ty>::type;
*/

/*
template<typename Scalar>
typename std::enable_if_t<sw::universal::is_posit<Scalar>, Scalar > Dot(const sw::universal::blas::vector<Scalar>& x, const sw::universal::blas::vector<Scalar>& y) {
	return Scalar(1.0);
}
*/

template<typename Scalar>
typename sw::universal::enable_if_posit1<Scalar, Scalar> Dot(const sw::numeric::containers::vector<Scalar>& x, const sw::numeric::containers::vector<Scalar>& y) {
	std::cerr << "fused dot product\n";
	return sw::universal::fdp(x, y);
}

template<typename Scalar>
typename std::enable_if_t<std::is_floating_point<Scalar>::value, Scalar> Dot(const sw::numeric::containers::vector<Scalar>& x, const sw::numeric::containers::vector<Scalar>& y) {
	std::cerr << "regular dot product\n";
	return sw::blas::dot(x, y);
}

void Enumerate() {
	using namespace sw::universal;
	using namespace sw::numeric::containers;
	using namespace sw::blas;

	// algorithm is dot product

	// randomized values for the least common denominator
	constexpr size_t N = 10;
	vector<posit<8, 0>> x(N), y(N);
	uniform_random(x);
	uniform_random(y);

	{
		posit<8, 0> a = Dot(x, y); // fused dot
		std::cout << a << std::endl;
	}

	{
		vector<float> _x(10), _y(10);
		for (size_t i = 0; i < N; ++i) {
			_x[i] = float(x[i]);
			_y[i] = float(y[i]);
		}

		float a = Dot(_x, _y); // regular dot
		std::cout << a << std::endl;
	}

}

int main()
try {
	using namespace sw::blas;

	std::cout << "Pareto frontier for mixed-precision number selection\n";

	// first algorithm: dot product
	// integer : if dynamic range insufficient needs a quire to avoid overflow
	// fixpnt  : if dynamic range insufficient needs a quire to avoid overflow
	// ieee-754: FMA and large dynamic range 
	// posit   : quire 

	// if the sum overflows it implies that the dynamic range of the representation
	// is insufficient.
	Enumerate();
	
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
