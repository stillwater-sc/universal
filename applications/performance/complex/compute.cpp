// compute.cpp: experiments with complex real/imaginary computations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal number project, which is released under an MIT Open Source license.
//
// c * conj(c) = |c|^2 is real: its imaginary part cancels exactly, and its real part is the
// sum of two squares. Printing the encodings shows how each number system represents the
// operands and the product.
//
// The experiment uses sw::universal::complex<T>. std::complex<T> is defined only for float,
// double and long double (ISO C++ 26.2/2); this file used to instantiate it on fixpnt and posit
// and carried overloads to patch the gaps that left (#2033).
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <universal/native/ieee754.hpp>
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/fixpnt/fixpnt.hpp>
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/cfloat/cfloat.hpp>
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/math/complex.hpp>

template<typename Scalar>
void TestComplexConjugate(float r = 0.25f, float i = 0.5f) {
	using Complex = sw::universal::complex<Scalar>;
	constexpr bool nibbleMarker = true;
	std::cout << "TestComplexConjugate for type " << sw::universal::type_tag(Scalar{}) << '\n';
	const Complex c{ Scalar(r), Scalar(i) };
	const Complex cconj = conj(c);
	std::cout << to_binary(c, nibbleMarker) << " : " << c << '\n';
	std::cout << to_binary(cconj, nibbleMarker) << " : " << cconj << '\n';
	const Complex product = c * cconj;
	std::cout << '(' << r << '+' << i << ")*(" << r << '-' << i << ") = " << to_binary(product, nibbleMarker) << " : " << product << '\n';
	std::cout << "----\n";
}

int main()
try {
	using namespace sw::universal;

	std::cout << "----\ntesting complex conjugate operations for different number types\n";
	TestComplexConjugate<float>();
	TestComplexConjugate<fixpnt<8, 4>>();
	// at 8 bits a cfloat with es = 3 needs subnormals to represent 0.25 * 0.25
	TestComplexConjugate<cfloat<8, 3, std::uint8_t, true, false, false>>();
	TestComplexConjugate<posit<8, 2>>();

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
