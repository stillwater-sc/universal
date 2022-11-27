// matrix_ops.cpp: matrix API for sw::universal::blas
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// pull in the number systems you would like to use
#include <universal/number/posit/posit.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/decimal/decimal.hpp>
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>

template<typename Scalar>
void FdpTest() {
	using namespace sw::universal::blas;
	std::cout << "Fused DOT product BLAS when posits are used\n";
	using Matrix = sw::universal::blas::matrix<Scalar>;
	using Vector = sw::universal::blas::vector<Scalar>;
	constexpr unsigned M = 15;
	constexpr unsigned N = M;
	Matrix A = row_order_index<Scalar>(M, N);
	Vector x(M, 1);
	
	auto b = A * x;
	std::cout << b << '\n';
}


int main(int argc, char* argv[])
try {
	using namespace sw::universal::blas;
	using namespace sw::universal;
	{
		std::cout << "Fused DOT product BLAS when posits are used\n";
		FdpTest<float>();
		FdpTest<posit<16, 2> >();
	}


	{
		using Scalar = float;
		using Matrix = sw::universal::blas::matrix<Scalar>;
		Matrix A = row_order_index<Scalar>(23, 57);
		Matrix B(A);
		A.transpose().transpose();
		if (A != B) {
			std::cout << "transpose FAIL\n";
		}
		else {
			std::cout << "transpose PASS\n";
		}
	}

	{
		using Scalar = sw::universal::posit<256,5>;
		using Matrix = sw::universal::blas::matrix<Scalar>;
		Matrix A = row_order_index<Scalar>(117, 253);
		Matrix B(A);
		A.transpose().transpose();
		if (A != B) {
			std::cout << "transpose FAIL\n";
		}
		else {
			std::cout << "transpose PASS\n";
		}
	}

	{
		using Scalar = sw::universal::integer<8192>;
		using Matrix = sw::universal::blas::matrix<Scalar>;
		Matrix A = row_order_index<Scalar>(253, 771);
		Matrix B(A);
		A.transpose().transpose();
		if (A != B) {
			std::cout << "transpose FAIL\n";
		}
		else {
			std::cout << "transpose PASS\n";
		}
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
