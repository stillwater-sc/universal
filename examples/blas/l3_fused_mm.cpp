// l3_fused_mm.cpp: example program showing a fused matrix-matrix product
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the following define to show the intermediate steps in the fused-dot product
// #define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_MUL
#define QUIRE_TRACE_ADD
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>
#define BLAS_TRACE_ROUNDING_EVENTS 1
#include <universal/blas/blas.hpp>

template<typename Scalar>
void catastrophicCancellationTest() {
	using namespace std;
	cout << "\nScalar type : " << typeid(Scalar).name() << '\n';
	using Matrix = sw::unum::blas::matrix<Scalar>;
	using Vector = sw::unum::blas::vector<Scalar>;

	Scalar a1 = 3.2e8;
	Scalar a2 = 1;
	Scalar a3 = -1;
	Scalar a4 = 8e7;
	Matrix A = { 
		{ a1, a2, a3, a4 }, 
		{ a4, a3, a2, a1 },
	};
	Scalar b1 = 4.0e7;
	Scalar b2 = 1;
	Scalar b3 = -1;
	Scalar b4 = -1.6e8;
	Matrix B = {
		{ b1, b4 },
		{ b2, b3 },
		{ b3, b2 },
		{ b4, b1 }
	};

	cout << std::setprecision(10);
	cout << "matrix A: \n" << A << endl;
	cout << "matrix B: \n" << B << endl;
	auto C = A * B;
	cout << "matrix C: \n" << C << endl;
	if (C[0][0] == 2 && C[0][1] == 2 && C[1][0] == 2 && C[1][1] == 2) {
			cout << "PASS\n";
	}
	else {
		cout << "FAIL\n";
	}
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum::blas;

	catastrophicCancellationTest<float>();  // FAILS due to catastrophic cancellation
	catastrophicCancellationTest<double>(); // FAILS due to catastrophic cancellation
	catastrophicCancellationTest< sw::unum::posit<32,2> >(); // PASSES due to FDP

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
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
