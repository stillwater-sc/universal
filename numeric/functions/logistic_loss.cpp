// logistic_loss.cpp: logistic loss function and its tempered and bi-tempered variants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define ALIASING_ALLOWED
#include <universal/number/posit/posit.hpp>
#include <math/functions/loss.hpp>

int main()
try {
	using namespace sw::universal;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Posit = posit<nbits,es>;

	// print detailed bit-level computational intermediate results

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

	Posit one_minus_1ulp(1.0f);
	--one_minus_1ulp;
	Posit one_plus_1ulp(1.0f);
	++one_plus_1ulp;

	{

		// has to be in double as floats do not have the precision to capture 1 - ULP of 32bit posit
		double tmps[] = { 0.0, 0.2, 0.4, 0.6, 0.8, double(one_minus_1ulp) };   // temperature can't be 1


		// logt(x) := (1 / (1 – t)) * (x ^ (1–t) – 1)
		// plot tempered logarithm
		for (Posit t : tmps) {
			Posit upperbound = Posit(4.0);
			constexpr unsigned nrSamples = 16;
			Posit step = upperbound / nrSamples;
			Posit x = 0;
			for (unsigned i = 0; i <= nrSamples; ++i) {
				std::cout << "x = " << x << " logt(" << t << "," << x << ") = " << sw::math::function::logt(t, x) << '\n';
				x += step;
			}
		}
	}

	// expt(x) : = [1 + (1 – t) x]+ ^ (1 / (1–t))
	// plot tempered exponent
	{

		// has to be in double as floats do not have the precision to capture 1 - ULP of 32bit posit
		double tmps[] = { double(one_plus_1ulp), 1.5, 2.0, 2.5, 3.0, 3.5 };   // temperature can't be 1


		// expt(x) : = [1 + (1 – t) x]+ ^ (1 / (1–t))
		// plot tempered exponent
		for (Posit t : tmps) {
			Posit lowerbound = Posit(-4.0);
			Posit upperbound = Posit(0.5);
			constexpr unsigned nrSamples = 16;
			Posit step = (upperbound - lowerbound)/ nrSamples;
			Posit x = lowerbound;
			for (unsigned i = 0; i <= nrSamples; ++i) {
				std::cout << "x = " << x << " expt(" << t << "," << x << ") = " << sw::math::function::expt(t, x) << '\n';
				x += step;
			}
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
