// dot.cpp: data flow performance measurements of mixed-precision dot product
//     
// Copyright(c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT 
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// enable the following define to show the intermediate steps in the fused-dot product
// #define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_MUL
#define QUIRE_TRACE_ADD
// configure posit environment using fast posits
#define POSIT_FAST_POSIT_8_0 1
#define POSIT_FAST_POSIT_16_1 1
#define POSIT_FAST_POSIT_32_2 1
#define POSIT_FAST_POSIT_64_3 0  // TODO
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/number/edecimal/edecimal.hpp>
#include <blas/blas.hpp>

int main()
try {
	using namespace sw::universal;
	using namespace sw::blas;

	std::streamsize prec = std::cout.precision();
	std::cout << std::setprecision(17);
	
	{
		using Scalar = edecimal;
		using Vector = sw::numeric::containers::vector<Scalar>;
//		Scalar a1 = 3.2e8, a2 = 1, a3 = -1, a4 = 8e7;             // TODO: <--- bug conversion from double
//		Scalar b1 = 4.0e7, b2 = 1, b3 = -1, b4 = -1.6e8;
		Scalar a1 = 320'000'000, a2 = 1, a3 = -1, a4 =   80'000'000;
		Scalar b1 =  40'000'000, b2 = 1, b3 = -1, b4 = -160'000'000;
		Vector a = { a1, a2, a3, a4 };
		Vector b = { b1, b2, b3, b4 };

		std::cout << "a: " << a << '\n';
		std::cout << "b: " << b << '\n';

		std::cout << "\n\n";
		edecimal v = dot(a, b);
		std::cout << v << (v == 2 ? " <----- PASS\n" : " <-----      FAIL\n");
	}

	std::cout << std::setprecision(prec);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
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
