// floating-point.cpp: dynamic range comparisons among floating-point types
//     
// Copyright(c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT 
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/posit/posit.hpp>
// Stillwater BLAS library
#include <blas/blas.hpp>

int main()
try {
	using namespace sw::universal;

	std::streamsize prec = std::cout.precision();
	std::cout << std::setprecision(17);
	
	{
		std::cout << symmetry_range<cfloat<4, 1, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<4, 2, uint8_t, true, true, false>>() << '\n';
	}
	{
		std::cout << symmetry_range<cfloat<6, 2, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<6, 3, uint8_t, true, true, false>>() << '\n';
	}
	{
		std::cout << symmetry_range<cfloat<8,2, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<8,3, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<8,4, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<8,5, uint8_t, true, true, false>>() << '\n';
	}
	{
		std::cout << symmetry_range<cfloat<10, 2, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<10, 3, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<10, 4, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<10, 5, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<10, 6, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<10, 7, uint8_t, true, true, false>>() << '\n';
	}
	{
		std::cout << symmetry_range<cfloat<12, 2, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<12, 3, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<12, 4, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<12, 5, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<12, 6, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<12, 7, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<12, 8, uint8_t, true, true, false>>() << '\n';
	}
	{
		std::cout << symmetry_range<cfloat<14, 2, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<14, 3, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<14, 4, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<14, 5, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<14, 6, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<14, 7, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<14, 8, uint8_t, true, true, false>>() << '\n';
	}
	{
		std::cout << symmetry_range<cfloat<16, 2, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<16, 3, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<16, 4, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<16, 5, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<16, 6, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<16, 7, uint8_t, true, true, false>>() << '\n';
		std::cout << symmetry_range<cfloat<16, 8, uint8_t, true, true, false>>() << '\n';
	}


	{
		std::cout << symmetry_range<lns<4, 2, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<4, 1, uint8_t>>() << '\n';
	}
	{
		std::cout << symmetry_range<lns<6, 4, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<6, 3, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<6, 2, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<6, 1, uint8_t>>() << '\n';
	}
	{
		std::cout << symmetry_range<lns<8, 6, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<8, 5, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<8, 4, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<8, 3, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<8, 2, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<8, 1, uint8_t>>() << '\n';
	}
	{
		std::cout << symmetry_range<lns<10, 6, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<10, 5, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<10, 4, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<10, 3, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<10, 2, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<10, 1, uint8_t>>() << '\n';
	}
	{
		std::cout << symmetry_range<lns<12, 6, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<12, 5, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<12, 4, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<12, 3, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<12, 2, uint8_t>>() << '\n';
		std::cout << symmetry_range<lns<12, 1, uint8_t>>() << '\n';
	}

	{
		std::cout << symmetry_range<dbns<4, 1, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<4, 2, uint8_t>>() << '\n';
	}
	{
		std::cout << symmetry_range<dbns<6, 1, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<6, 2, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<6, 3, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<6, 4, uint8_t>>() << '\n';
	}
	{
		std::cout << symmetry_range<dbns<8, 1, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<8, 2, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<8, 3, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<8, 4, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<8, 5, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<8, 6, uint8_t>>() << '\n';
	}
	{
		std::cout << symmetry_range<dbns<10, 1, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<10, 2, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<10, 3, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<10, 4, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<10, 5, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<10, 6, uint8_t>>() << '\n';
	}
	{
		std::cout << symmetry_range<dbns<12, 3, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<12, 4, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<12, 5, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<12, 6, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<12, 7, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<12, 8, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<12, 9, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<12, 10, uint8_t>>() << '\n';
	}
	{
		std::cout << symmetry_range<dbns<14, 5, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<14, 6, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<14, 7, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<14, 8, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<14, 9, uint8_t>>() << '\n';
		std::cout << symmetry_range<dbns<14,10, uint8_t>>() << '\n';
	}
	std::cout << std::setprecision(prec);

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
