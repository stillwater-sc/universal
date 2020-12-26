// matrix_ops.cpp: matrix API for sw::universal::blas
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifdef _MSC_VER
#pragma warning(disable : 4100) // argc/argv unreferenced formal parameter
#pragma warning(disable : 4514 4571)
#pragma warning(disable : 4625 4626) // 4625: copy constructor was implicitly defined as deleted, 4626: assignment operator was implicitely defined as deleted
#pragma warning(disable : 5025 5026 5027)
#pragma warning(disable : 4710 4774)
#pragma warning(disable : 4820)
#endif
// pull in the number systems you would like to use
#include <universal/posit/posit>
#include <universal/integer/integer>
#include <universal/decimal/decimal>
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>

int main(int argc, char* argv[])
try {
	using namespace std;
	using namespace sw::universal::blas;

	{
		using Scalar = float;
		using Matrix = sw::universal::blas::matrix<Scalar>;
		Matrix A = row_order_index<Scalar>(23, 57);
		Matrix B(A);
		A.transpose().transpose();
		if (A != B) {
			cout << "transpose FAIL\n";
		}
		else {
			cout << "transpose PASS\n";
		}
	}

	{
		using Scalar = sw::universal::posit<256,5>;
		using Matrix = sw::universal::blas::matrix<Scalar>;
		Matrix A = row_order_index<Scalar>(117, 253);
		Matrix B(A);
		A.transpose().transpose();
		if (A != B) {
			cout << "transpose FAIL\n";
		}
		else {
			cout << "transpose PASS\n";
		}
	}

	{
		using Scalar = sw::universal::integer<8192>;
		using Matrix = sw::universal::blas::matrix<Scalar>;
		Matrix A = row_order_index<Scalar>(253, 771);
		Matrix B(A);
		A.transpose().transpose();
		if (A != B) {
			cout << "transpose FAIL\n";
		}
		else {
			cout << "transpose PASS\n";
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
