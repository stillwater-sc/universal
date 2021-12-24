// logistic-loss.cpp: logictic loss function and its tempered and bi-tempered variants
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#define ALIASING_ALLOWED
#include <universal/number/posit/posit.hpp>
#include <universal/functions/loss.hpp>

int main()
try {
	using namespace sw::universal;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Posit = posit<nbits,es>;

	// print detailed bit-level computational intermediate results
	//bool verbose = false;

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

	Posit tmps[] = { Posit(0.0), Posit(0.2), Posit(0.4), Posit(0.6), Posit(0.8), --Posit(1.0) };

	for (Posit t : tmps) {
		Posit ub = Posit(4.0);
		constexpr unsigned nrSamples = 16;
		Posit step = ub / nrSamples;
		Posit x = Posit(0); // minpos<nbits, es>();
		for (unsigned i = 0; i <= nrSamples; ++i) {
			std::cout << "x = " << x << " logt(" << t << "," << x << ") = " << logt(t, x) << '\n';
			x += step;
		}
	}

	// restore the previous ostream precision
	std::cout << std::setprecision(precision);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
