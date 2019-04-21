// sum_of_integers.cpp: evaluation of a sequence of additions in the posit number systems
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#define ALIASING_ALLOWED
#include "common.hpp"

template<typename Scalar>
Scalar NaiveSumOfIntegers(long lowerbound = 0, long upperbound = 10000) {
	Scalar sum = 0;
	for (long i = lowerbound; i < upperbound; ++i) {
		sum += i;
	}
	return sum;
}

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

	cout << "SumOfIntegers using 64-bit int\n";
	for (int i = 1; i < 8; ++i) {
		cout << " 0 - " << pow(10,i) << " : " << NaiveSumOfIntegers<long long>(0, (long)pow(10,i)) << endl;
	}
	cout << "SumOfIntegers using IEEE single precision float\n";

	for (int i = 1; i < 8; ++i) {
		cout << " 0 - " << pow(10, i) << " : " << NaiveSumOfIntegers<float>(0, (long)pow(10, i)) << endl;
	}
	cout << "SumOfIntegers using posit<32,2>\n";

	for (int i = 1; i < 8; ++i) {
		cout << " 0 - " << pow(10, i) << " : " << NaiveSumOfIntegers<Posit>(0, (long)pow(10, i)) << endl;
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
