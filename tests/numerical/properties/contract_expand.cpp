// contract_expand.cpp: evaluation of contractions and expansions of posit number systems
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include "common.hpp"
// pull in the posit number system
#include <universal/posit/posit>

template<typename Scalar>
void ContractionExpansion(int depth) {
	using namespace std;
	using namespace sw::unum;

	int columnWidth = 20;
	Scalar seed = 2.0;
	cout << "Contraction/Expansion sequence sqrt(sqrt(sqrt(...sqrt(x))))))^depth => seed with seed = " << seed << endl;
	cout << setw(3) << "#"
		<< setw(columnWidth) << "contraction"
		<< setw(columnWidth) << "expansion"
		<< setw(columnWidth) << "error"
		<< endl;
	for (int i = 1; i < depth; ++i) {
		Scalar x = seed;
		for (int k = 1; k < i; ++k) {
			x = sqrt(x);
		}
		Scalar contraction = x;
		for (int k = 1; k < i; ++k) {
			x = exp2(x);
		}
		Scalar expansion = x;
		cout << setw(3) << i << " " 
			<< setw(columnWidth) << contraction << " "
			<< setw(columnWidth) << expansion << " "
			<< setw(columnWidth) << expansion - seed 
			<< endl;
	}
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Posit = posit<nbits,es>;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;

	// preserve the existing ostream precision
	auto precision = cout.precision();
	cout << setprecision(12);

	ContractionExpansion<Posit>(10);

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
