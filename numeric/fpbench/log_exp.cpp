// log_exp.cpp: numerical test programs for fpbench tests for functions constructed with log and exp 
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal project, which is released under an MIT Open Source license.
#include <iostream>
#include <random>
#include <algorithm>
#include <vector>

// select your number system
//#include <universal/areal/areal>
#include <universal/number/posit1/posit1.hpp>
#include <universal/number/valid/valid.hpp>

// ln(e^x) -> should always yield x
template<typename Scalar>
Scalar ln_of_exp_x(const Scalar& x) {
	Scalar exponent(exp(x));
	Scalar result = log(exponent);
	return result;
}

// ln(1 + e^x)
template<typename Scalar>
Scalar ln_of_one_plus_exp_x(const Scalar& x) {
	Scalar one_plus_exponent(Scalar(1) + exp(x));
	Scalar result = log(one_plus_exponent);
	return result;
}

template<typename Scalar>
void SampleFunctionEvaluation(const std::vector<double>& samples) {
	using namespace std;
	using namespace sw::universal;

	size_t nrSamples = size(samples);
	vector<Scalar> results(nrSamples);
	for (size_t i = 0; i < nrSamples; ++i) {
		results[i] = ln_of_exp_x(Scalar(samples[i]));
	}

	vector<Scalar> diffs(nrSamples);
	for (size_t i = 0; i < nrSamples; ++i) {
		diffs[i] = Scalar(samples[i]) - results[i];
	}

	Scalar eps = numeric_limits<Scalar>::epsilon();
	cout << setw(50) << typeid(Scalar).name() << ": epsilon() = " << eps << endl;
	int i = 0;
	for_each(begin(diffs), end(diffs), [&](Scalar n) {
		if (n != 0) {
			Scalar nrEps = n / eps;
			cout << "FAIL: " << hex_format(n) << " " << n << " nr of epsilons of error: " << nrEps << endl;
			cout << color_print(n) << endl;
			cout << color_print(Scalar(samples[i])) << endl;
			cout << color_print(results[i]) << endl;
			++i;
		}
		else {
			// cout << "PASS: " << hex_format(n) << endl;
		}
	});
}

int main(int argc, char* argv[]) 
try {
	using namespace std;
	using namespace sw::universal;

	// preserve the existing ostream precision
	auto precision = cout.precision();
	cout << setprecision(12);

	constexpr size_t NR_SAMPLES = 16;
	// Use random_device to generate a seed for Mersenne twister engine.
	random_device rd{};
	// Use Mersenne twister engine to generate pseudo-random numbers.
	mt19937 engine{ rd() };
	// "Filter" MT engine's output to generate pseudo-random double values,
	// **uniformly distributed** on the closed interval [lowerbound, upperbound].
	// (Note that the range is [inclusive, inclusive].)
	double lowerbound = 1;
	double upperbound = 2;
	uniform_real_distribution<double> dist{ lowerbound, upperbound };
	// Pattern to generate pseudo-random number.
	// double rnd_value = dist(engine);

	vector<double> samples(NR_SAMPLES);
	for_each(begin(samples), end(samples), [&](double& n) {	n = dist(engine); });

	SampleFunctionEvaluation < float >(samples);
	SampleFunctionEvaluation < double >(samples);
	SampleFunctionEvaluation < posit< 8, 0> >(samples);
	SampleFunctionEvaluation < posit<16, 1> >(samples);
	SampleFunctionEvaluation < posit<32, 2> >(samples);
	SampleFunctionEvaluation < posit<64, 3> >(samples);
	SampleFunctionEvaluation < posit<128, 4> >(samples);
	SampleFunctionEvaluation < posit<256, 5> >(samples);

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
