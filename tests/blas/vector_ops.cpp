// vector_ops.cpp: example program to show sw::universal::blas::vector operators
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// enable the following define to show the intermediate steps in the fused-dot product
// #define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_MUL
#define QUIRE_TRACE_ADD
// configure posit environment using fast posits
#define POSIT_FAST_POSIT_8_0 1
#define POSIT_FAST_POSIT_16_1 1
#define POSIT_FAST_POSIT_32_2 1
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/blas/blas.hpp>

template<unsigned nbits, unsigned es>
void PrintProducts(const sw::universal::blas::vector<sw::universal::posit<nbits,es>>& a, 
		           const sw::universal::blas::vector<sw::universal::posit<nbits,es>>& b) 
{
	sw::universal::quire<nbits, es> q(0);
	for (unsigned i = 0; i < a.size(); ++i) {
		q += sw::universal::quire_mul(a[i], b[i]);
		std::cout << a[i] << " * " << b[i] << " = " << a[i] * b[i] << std::endl << "quire " << q << std::endl;
	}
	sw::universal::posit<nbits,es> sum;
	sw::universal::convert(q.to_value(), sum);     // one and only rounding step of the fused-dot product
	std::cout << "fdp result " << sum << std::endl;
}


int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// set up the properties of the arithmetic system
	constexpr unsigned nbits = 32;
	constexpr unsigned es = 2;
	using Scalar = posit<nbits, es>;
	using Vector = blas::vector<Scalar>;

	constexpr unsigned vectorSize = SIZE_32K + 2;
	Vector a(vectorSize), b(vectorSize);
	Scalar epsilon = std::numeric_limits<Scalar>::epsilon();
	for (unsigned i = 1; i < vectorSize-1; ++i) {
		a[i] = 1;
		b[i] = epsilon;
	}
	a[0] = a[vectorSize - 1] = posit<nbits, es>(SpecificValue::maxpos);
	b[0] = -1;  b[vectorSize - 1] = 1;
	if (vectorSize < 10) {
		std::cout << a << '\n';
		std::cout << b << '\n';
		PrintProducts(a, b);
	}
	
	// accumulation of 32K epsilons for a posit<32,2> yields
	// 	   a:   maxpos     1       1    ...    1     maxpos
	// 	   b:    -1     epsilon epsilon ... epsilon    1
	// 	   the two maxpos values will cancel out leaving the 32k epsilon's accumulated
	// 	the dot product will experience catastrophic cancellation, 
	//  fdp will calculate the sum of products correctly
	// dot: 0
	// fdp: 0.000244141
	std::cout << "\naccumulation of 32k epsilons (" << epsilon << ") for a " << type_tag(Scalar()) << " yields:\n";
	std::cout << "dot            : " << dot(a, b)  << " : " << hex_format(dot(a,b)) << '\n';
	std::cout << "fdp            : " << fdp(a, b)  << " : " << hex_format(fdp(a,b)) << '\n';
	Scalar validation = (vectorSize - 2) * epsilon;
	std::cout << "32k * epsilon  : " << validation << " : " << hex_format(validation) << '\n';

	// scale a vector
	std::cout << "\nscaling a vector\n";
	for (unsigned i = 0; i < vectorSize; ++i) {
		a[i] = 1;
		b[i] = epsilon;
	}
	a *= epsilon; // a * epsilon -> b
	bool success = true;
	for (unsigned i = 0; i < size(a); ++i) {
		if (a[i] != b[i]) {
			std::cout << a[i] << " != " << b[i] << '\n';
			success = false;
			break;
		}
	}
	if (success) {
		std::cout << "PASS: scaling vector a by epsilon yielded vector b\n";
	}
	else {
		std::cout << "FAIL: scaling vector a by epsilon failed to yield vector b\n";
	}

	// normalize a vector
	std::cout << "\nnormalizing a vector\n";
	for (unsigned i = 0; i < vectorSize; ++i) {
		a[i] = 1;
	}
	b /= epsilon; // b / epsilon -> a
	success = true;
	for (unsigned i = 0; i < size(a); ++i) {
		if (a[i] != b[i]) {
			std::cout << a[i] << " != " << b[i] << '\n';
			success = false;
			break;
		}
	}
	if (success) {
		std::cout << "PASS: normalizing vector b by epsilon yielded vector a\n";
	}
	else {
		std::cout << "FAIL: scaling vector b by epsilon failed to yield vector a\n";
	}

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
