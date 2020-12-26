// sterbenz_lemma.cpp: Demonstration of Sterbenz Lemma
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include "common.hpp"
// pull in the posit number system
#include <universal/posit/posit>

/*
Sterbenz Lemma: For any two numbers of the same format, but excluding NaR,
                 a/2 <= b <= 2*a => a subop b = a - b
Stated in English: a - b is exactly representable

For a given posit format, if the fraction field length of 2^l is p >= 1 bits, 
then for all d in [0,p], the fraction field length of 2^(l-p) is at least (p - d).

The proof in general case is based on the following lemma:

For a given posit format, if the fraction field length of 2^m is p >= m bits, 
then for all d in [0,p], the fraction field length of 2^(m-d) is at least (p - d).
*/

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	constexpr size_t nbits = 8;
	constexpr size_t es = 0;
	using Posit = posit<nbits,es>;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;

	// preserve the existing ostream precision
	auto precision = cout.precision();
	cout << setprecision(12);

	// set an a
	Posit a = 1.0f;
	// how many samples are there in the range [a/2, 2a]?
	// a/2 = 0011000000000000 
	// a   = 0100000000000000 
	// 2*a = 0101000000000000
	cout << "[ " << color_print(a / 2) << " " << color_print(a) << " " << color_print(2 * a) << " ]" << endl;

	Posit c = a - (a / 2);
	cout << color_print(c) << endl;

	int nrOfFailures = 0;
	float fa = float(a);
	for (Posit b = a / 2; b <= 2 * a; ++b) {
		float fb = float(b);
		float fc = fa - fb;
		c = a - b;
		if (fc != c) {
			cout << "FAIL: " << a << " - " << b << " = " << c << " reference = " << fc << endl;
			++nrOfFailures;
		}
	}
	if (nrOfFailures == 0) {
		cout << "PASS" << endl;
	}


	// restore the previous ostream precision
	cout << setprecision(precision);

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
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
