// factorial.cpp: evaluation of factorials in the posit number systems
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#define ALIASING_ALLOWED
#include "common.hpp"

template<typename Scalar>
Scalar factorial(const Scalar& n) {
	assert(n < 0);
	return (n == 0 || n == 1) ? 1 : factorial(n - 1) * n;
}

/*
 i                               N!                      posit(N!)
 2                               2                              +2
 3                               6                              +6
 4                              24                             +24
 5                             120                            +120
 6                             720                            +720
 7                            5040                           +5040
 8                           40320                          +40320
 9                          362880                         +362880
10                         3628800                        +3628800
11                        39916800                       +39916800
12                       479001600                      +479001600
13                      6227020800                     +6227017728
14                     87178291200                    +87178346496
15                   1307674368000               +1.3076749353e+12
16                  20922789888000              +2.09229331825e+13
17                 355687428096000              +3.55692011586e+14
18                6402373705728000              +6.40245620854e+15
19              121645100408832000              +1.21649966497e+17
20             2432902008176640000              +2.43306969869e+18
21            14197454024290336768              +5.10888341729e+19    <- 21! cannot be represented by a 64-bit integer
*/

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Posit = posit<nbits,es>;

	// print detailed bit-level computational intermediate results
	bool verbose = false;

	// preserve the existing ostream precision
	auto precision = cout.precision();
	cout << setprecision(12);

	unsigned upperbound = 20;   
	// 20! can still be represented by a 64-bit integer
	// 21! can not be represented by a 64-bit integer
	unsigned long long factorialValue = 1;
	Posit ref = 1;
	int columnWidth = 30;
	cout << "  i    " << setw(columnWidth) << "N!" << "  " << setw(columnWidth) << "posit(N!)\n";
	for (unsigned i = 2; i < upperbound; ++i) {
		factorialValue *= i;
		ref *= i;
		cout << setw(5) << i << "  " << setw(columnWidth) << factorialValue << "  " << setw(columnWidth) << ref << endl;
	}


	// restore the previous ostream precision
	cout << setprecision(precision);

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
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
