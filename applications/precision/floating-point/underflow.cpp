// underflow.cpp: experiments with underfow in posit number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// pull in the posit number system
#include <universal/number/posit/posit.hpp>

// Undetected underflow
//
// two dice, one slightly unfair. Pick one die. Problem: what is the conditional probability that you picked the fair die?
//
// Bayesian rule
// r = number of rolls
// p1 = 1/6 probability of the fair die
// p2 = 1.001/6 probability of the biased die
//
// probability of r nr of rolls of a die, fair or biased
// p_roll_fair = p1^r
// p_roll_bias = p2^r
//
// two ways of computing the conditional probability
// p_fair_v1 = p_roll_fair / (p_roll_fair + p_roll_bias)
// p_fair_v2 = 1 / (1 = (p2/p1) ^ r)
//
//
template<typename Scalar> 
Scalar integerPower(Scalar p, int r) {
	if (0 == r) return 1.0;
	Scalar power = p;
	for ( int i = 1; i < r; ++i) {
		power *= power;
	}
	return power;
}

template<typename Scalar>
Scalar ConditionalProb_v1(int r, Scalar p1, Scalar p2) {
	Scalar p_roll_fair = integerPower(p1, r);
	Scalar p_roll_bias = integerPower(p2, r);
	return p_roll_fair / (p_roll_fair + p_roll_bias);
}

template<typename Scalar>
Scalar ConditionalProb_v2(int r, Scalar p1, Scalar p2) {
	return 1.0 / (1.0 + integerPower(p2/p1, r));
}

/*
bad for abs(z) << 1
y = 1 - sqrt(1-z);
better
y = z/(1 + sqrt(1-z));
*/
template<typename Scalar>
Scalar BadOneMinusSqrtOfOneMinusZ(Scalar z) {
	return 1 - sqrt(1 - z);
}
template<typename Scalar>
Scalar GoodOneMinusSqrtOfOneMinusZ(Scalar z) {
	return z / (1 + sqrt(1 - z));
}

template<typename Scalar>
void OneMinusSqrtOfOneMinusZ() {
	using namespace sw::universal;
	constexpr size_t columnWidth = 20;
	std::cout << std::setw(columnWidth) << "z"
		<< std::setw(columnWidth) << "bad"
		<< std::setw(columnWidth) << "good"
		<< std::setw(columnWidth) << "error"
		<< '\n';
	for (float z = 0.0f; z < 1.0f; z = z + 0.05f) {
		Scalar bad = BadOneMinusSqrtOfOneMinusZ(z);
		Scalar good = GoodOneMinusSqrtOfOneMinusZ(z);
		Scalar error = abs(bad - good);
		std::cout << std::setw(columnWidth) << z << " " << bad << " " << good << " " << error << '\n';
	}
}

int main()
try {
	using namespace sw::universal;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Scalar = posit<nbits,es>;
	//using Scalar = double;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

	Scalar p1 = 1.0/6;
	Scalar p2 = 1.001/6;
	int    r = 20;

	for (int i = 0; i < r; ++i) {
		std::cout << std::setw(3) << i << " " << ConditionalProb_v1(i, p1, p2) << " " << ConditionalProb_v2(i, p1, p2) << '\n';
	}

	std::cout << "1 - SQRT(1 - z)\n";
	OneMinusSqrtOfOneMinusZ< posit<16, 1> >();
	OneMinusSqrtOfOneMinusZ< posit<32, 2> >();
	OneMinusSqrtOfOneMinusZ< posit<64, 3> >();

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
