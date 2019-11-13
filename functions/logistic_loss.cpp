// logistic-loss.cpp: logictic loss function and its tempered and bi-tempered variants
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#define ALIASING_ALLOWED
#include <universal/posit/posit>
#include <universal/functions/loss.hpp>

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

	Posit tmps[] = { Posit(0.0), Posit(0.2), Posit(0.4), Posit(0.6), Posit(0.8), --Posit(1.0) };

	for (Posit t : tmps) {
		Posit ub = Posit(4.0);
		constexpr unsigned nrSamples = 16;
		Posit step = ub / nrSamples;
		Posit x = Posit(0); // minpos<nbits, es>();
		for (unsigned i = 0; i <= nrSamples; ++i) {
			cout << "x = " << x << " logt(" << t << "," << x << ") = " << sw::function::logt(t, x) << endl;
			x += step;
		}
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
